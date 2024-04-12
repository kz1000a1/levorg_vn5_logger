８/*
  CAN bus data logger for SUBARU Levorg
*/

#include "subaru_levorg_vn5.h"
#include "driver/twai.h"

// Pins used to connect to CAN bus transceiver:
// #define RX_PIN GPIO_NUM_21
// #define TX_PIN GPIO_NUM_20
// #define RX_PIN GPIO_NUM_19
// #define TX_PIN GPIO_NUM_22
#define RX_PIN GPIO_NUM_32
#define TX_PIN GPIO_NUM_26

#define POLLING_RATE_MS 1000
static bool driver_installed = false;

static float EngineRPM = 0;
static float Speed = 0;
static float AcceleratorPosition = 0;
static uint8_t ShiftPosition = 4;
static int16_t SteeringAngle = 0;
static float BrakePercentage = 0;

int16_t bytesToInt(uint8_t raw[], int shift, int size) {

  int16_t result = 0;

  for (int i = 0; i < size; i++) {
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

uint16_t bytesToUint(uint8_t raw[], int shift, int size) {

  uint16_t result = 0;

  for (int i = 0; i < size; i++) {
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

uint16_t bitsToUint(uint8_t raw[], int shift, int size) {

  uint16_t result = 0;

  for (int i = shift; i < shift + size; i++) {
    result = result << 1;
    result += bitToUint(raw, i);
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

/*
uint16_t bitsToUintLe(uint8_t raw[], int shift, int size) {

  uint16_t result = 0;

  for (int i = shift; i < shift + size; i++) {
    result = result << 1;
    result += bitToUint(raw, i);
  }

  return result;

  // return &raw[shift];
}
*/

bool if_can_message_receive_is_pendig() {

  uint32_t alerts_triggered;
  twai_status_info_t twaistatus;

  // Check if alert happened
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));

  // If CAN message receive is pending, process the message
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {
    return true;
  } else {
    return false;
  }
}

void subaruLevorgEngineSpeed(twai_message_t* rx_frame) {

  // EngineRPM = bitsToUIntLe(rx_frame->data, 16, 14);
  EngineRPM = rx_frame->data[2] + ((rx_frame->data[3] & 0x3f) << 8);
  // Serial.printf("%5.2f rpm\n",EngineRPM);
  AcceleratorPosition = bytesToUint(rx_frame->data, 4, 1) / 2.55;
  // Serial.printf("Accel = %3.2f \%\n",AcceleratorPosition);
}

void subaruLevorgTransmission(twai_message_t* rx_frame) {

  ShiftPosition = rx_frame->data[3] & 0x07;
  
}

void subaruLevorgSteering(twai_message_t* rx_frame) {

  SteeringAngle = bytesToIntLe(rx_frame->data, 2, 2) * 0.1;
  // Serial.printf("Steering %-d\n",SteeringAngle);
}

void subaruLevorgBrake(twai_message_t* rx_frame) {
  // Speed = bitsToUIntLe(rx_frame->data, 16, 13) * 0.015694;
  Speed = (rx_frame->data[2] + ((rx_frame->data[3] & 0x1f) << 8)) * 0.05625;
  // BrakePressure = (3.4518689053 * bytesToInt(rxBuf, 0, 2) - 327.27) / 1000.00;
  // BrakePercentage = min(0.2 * (bytesToInt(rxBuf, 0, 2) - 102), 100);
  BrakePercentage = bytesToUint(rx_frame->data, 5, 1) / 0.7;
  if (100 < BrakePercentage) {
    BrakePercentage = 100;
  }
  // Serial.printf("Brake = %3.2f \%\n",BrakePercentage);
}

void subaruLevorgOutputCsv() {
  // Serial.println("mazdaMx5OutputCsv()");
  Serial.printf(", %.1f, %.1f, %d, %.1f, %.1f, %-.1f\n", Speed, EngineRPM, ShiftPosition, AcceleratorPosition, BrakePercentage, SteeringAngle * MAX_STEERING_ANGLE / STEERING_MAX);
}

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  //Look in the api-reference for other speed sets.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("# Error: Failed to install driver");
    return;
  }

  // Start TWAI driver
  if (twai_start() != ESP_OK) {
    Serial.println("# Error: Failed to start driver");
    return;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) != ESP_OK) {
    Serial.println("# Error: Failed to reconfigure alerts");
    return;
  }

  // TWAI driver is now successfully installed and started
  driver_installed = true;
}

void loop() {
  twai_message_t rx_frame;

  if (!driver_installed) {
    // Driver not installed
    delay(1000);
    return;
  }

  // If CAN message receive is pending, process the message
  if (if_can_message_receive_is_pendig()) {
    // One or more messages received. Handle all.
    while (twai_receive(&rx_frame, 0) == ESP_OK) {

      switch (rx_frame.identifier) {
        case CAN_ID_ENGINE_SPEED:
          subaruLevorgEngineSpeed(&rx_frame);
          break;
        case CAN_ID_TRANSMISSION:
          subaruLevorgTransmission(&rx_frame);
          break;
        case CAN_ID_STEERLING:
          subaruLevorgSteering(&rx_frame);
          break;
        case CAN_ID_BRAKE:
          subaruLevorgBrake(&rx_frame);
          break;
        case CAN_ID_ENGINE_TEMPERATURE:
          subaruLevorgOutputCsv();
          break;
          // default:
          // Serial.printf("Unexpected can frame received. rx_frame.identifier=%3x\n", rx_frame.identifier);
      }
    }
  }
}
