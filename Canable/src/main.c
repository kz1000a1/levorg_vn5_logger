/*
  CAN bus data logger for SUBARU Levorg
*/
#include <math.h>

#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "can.h"
#include "led.h"
#include "system.h"
#include "error.h"
#include "printf.h"
#include "subaru_levorg_vn5.h"


static float EngineRPM = 0;
static float Speed = 0;
static float AcceleratorPosition = 0;
static uint8_t ShiftPosition = 0;
static int16_t SteeringAngle = 0;
static float BrakePercentage = 0;;

int16_t bytesToInt(uint8_t raw[], int shift, int size) {

  int16_t result = 0;

  for (int i = 0; i < size; i++) {
    // printf_("result=%04X << %d = ",result, sizeof(byte) * 8);
    result = result << (sizeof raw[0] * 8);
    // printf_("%d\n",result);
    for (int j = 0; j < sizeof raw[0] * 8; j++) {
      // printf_("%04X & %04X = %d += %04X,result=%04X\n", raw[i + shift], 1 << j,raw[i + shift] & (1 << j),result);
      // if(raw[i + shift] & (1 << j))(result += 1 << j);
      result += raw[i + shift] & (1 << j);
      //printf_("i=%d,j=%d,raw[%d]=0x%02X,result=0x%04X\n",i,j,i+shift,raw[i+shift],result);
    }
  }

  return result;

  // return &raw[shift];
}

uint16_t bytesToUint(uint8_t raw[], int shift, int size) {

  uint16_t result = 0;

  for (int i = 0; i < size; i++) {
    // printf_("result=%04X << %d = ",result, sizeof(byte) * 8);
    result = result << (sizeof raw[0] * 8);
    // printf_("%d\n",result);
    for (int j = 0; j < sizeof raw[0] * 8; j++) {
      // printf_("%04X & %04X = %d += %04X,result=%04X\n", raw[i + shift], 1 << j,raw[i + shift] & (1 << j),result);
      // if(raw[i + shift] & (1 << j))(result += 1 << j);
      result += raw[i + shift] & (1 << j);
      //printf_("i=%d,j=%d,raw[%d]=0x%02X,result=0x%04X\n",i,j,i+shift,raw[i+shift],result);
    }
  }

  return result;

  // return &raw[shift];
}

uint16_t bitToUint(uint8_t raw[], int shift) {
  uint16_t result;
  // Serial.printf("raw[%d]=%02X,>> %d = 0x%02X, & 0xFE\n",(shift - 1) / 8,raw[(shift - 1) / 8],7 - (shift - 1) % 8,(raw[(shift - 1) / 8] >> 7 - (shift - 1) % 8));

  result = (raw[(shift - 1) / 8] >> 7 - (shift - 1) % 8) & 0x01;
  /*
  printf("result0=%02X, result1=",result);
  result = result & 0x01;
  printf("%02X\n",result);
  } else {
    result = (raw[(shift - 1) / 8] & 0xFE);
  }*/
  return result;
}

uint16_t bitsToUint(uint8_t raw[], int shift, int size) {

  uint16_t result = 0;

  for (int i = shift; i < shift + size; i++) {
    result = result << 1;
    result += bitToUint(raw, i);
  }

  return result;

  // return &raw[shift];
}

int16_t bytesToIntLe(uint8_t raw[], int shift, int size) {

  int16_t result = 0;

  for (int i = size - 1; 0 <= i; i--) {
    // Serial.printf("result=%04X << %d = ",result, sizeof(byte) * 8);
    result = result << (sizeof raw[0] * 8);
    // Serial.printf("%d\n",result);
    for (int j = 0; j < sizeof raw[0] * 8; j++) {
      // Serial.printf("%04X & %04X = %d += %04X,result=%04X\n", raw[i + shift], 1 << j,raw[i + shift] & (1 << j),result);
      // if(raw[i + shift] & (1 << j))(result += 1 << j);
      result += raw[i + shift] & (1 << j);
      //Serial.printf("i=%d,j=%d,raw[%d]=0x%02X,result=0x%04X\n",i,j,i+shift,raw[i+shift],result);
    }
  }

  return result;

  // return &raw[shift];
}

void subaruLevorgEngineSpeed(uint8_t* rx_msg_data) {

  // EngineRPM = bitsToUIntLe(rx_msg_data, 16, 14);
  EngineRPM = rx_msg_data[2] + ((rx_msg_data[3] & 0x3f) << 8);
  // Serial.printf("%5.2f rpm\n",EngineRPM);
  AcceleratorPosition = bytesToUint(rx_msg_data, 4, 1) / 2.55;
  // Serial.printf("Accel = %3.2f \%\n",AcceleratorPosition);
}

void subaruLevorgTransmission(uint8_t* rx_msg_data) {

  ShiftPosition = rx_msg_data[3] & 0x07;
  
}

void subaruLevorgSteering(uint8_t* rx_msg_data) {

  SteeringAngle = bytesToIntLe(rx_msg_data, 2, 2) * 0.1;
  // Serial.printf("Steering %-d\n",SteeringAngle);
}

void subaruLevorgBrake(uint8_t* rx_msg_data) {
  // Speed = bitsToUIntLe(rx_msg_data, 16, 13) * 0.015694;
  Speed = (rx_msg_data[2] + ((rx_msg_data[3] & 0x1f) << 8)) * 0.05625;
  // BrakePressure = (3.4518689053 * bytesToInt(rxBuf, 0, 2) - 327.27) / 1000.00;
  // BrakePercentage = min(0.2 * (bytesToInt(rxBuf, 0, 2) - 102), 100);
  BrakePercentage = bytesToUint(rx_msg_data, 5, 1) / 0.7;
  if (100 < BrakePercentage) {
    BrakePercentage = 100;
  }
  // Serial.printf("Brake = %3.2f \%\n",BrakePercentage);
}

void subaruLevorgOutputCsv() {
  // Serial.println("subaruLevorgOutputCsv()");
  printf_(", %d, %d, %d, %d, %d, %-d\n", (int)Speed, (int)EngineRPM, (int)ShiftPosition, (int)AcceleratorPosition, (int)BrakePercentage, (int)(SteeringAngle * MAX_STEERING_ANGLE / STEERING_MAX));
}


int main(void)
{
    // Storage for status and received message buffer
    CAN_RxHeaderTypeDef rx_msg_header;
    uint8_t rx_msg_data[8] = {0};

    // Initialize peripherals
    system_init();
    can_init();
    led_init();
    usb_init();

    can_enable();

    while(1){

        // If CAN message receive is pending, process the message
        if(is_can_msg_pending(CAN_RX_FIFO0)){
            can_rx(&rx_msg_header, rx_msg_data);

            switch (rx_msg_header.StdId) {
                case CAN_ID_ENGINE_SPEED:
                    subaruLevorgEngineSpeed(rx_msg_data);
                    break;
                case CAN_ID_TRANSMISSION:
                    subaruLevorgTransmission(rx_msg_data);
                    break;
                case CAN_ID_STEERLING:
                    subaruLevorgSteering(rx_msg_data);
                    break;
                case CAN_ID_BRAKE:
                    subaruLevorgBrake(rx_msg_data);
                    break;
                case CAN_ID_ENGINE_TEMPERATURE:
                    subaruLevorgOutputCsv();
                    break;
                    // default:
                    // printf_("Unexpected can frame received. rx_frame.identifier=%3x\n", rx_frame.identifier);
            }
        }
    }
}

