/*
  CAN bus data logger for MAZDA MX-5
*/

#include "mazda_mx5_nc.h"
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
static char ShiftPosition = 'N';
static int16_t SteeringAngle = 0;
static float BrakePercentage = 0;
static uint8_t clutch = 0;
static uint8_t neutral = 0;

int16_t bytesToInt(uint8_t raw[], int shift, int size) {

  int16_t result = 0;

  for (int i = 0; i < size; i++) {
    // Serial.printf("result=%04X << %d = ",result, sizeof(byte) * 8);
    result = result << (sizeof raw[0] * 8);
    // Serial.printf("%d\n",result);
    for (int j = 0; j < sizeof(byte) * 8; j++) {
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
    for (int j = 0; j < sizeof(byte) * 8; j++) {
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

void mazdaMx5EngineSpeed(twai_message_t* rx_frame) {
  static float GearRatio;

  EngineRPM = bytesToUint(rx_frame->data, 0, 2) / 4.00;
  // Serial.printf("%5.2f rpm\n",EngineRPM);
  Speed = (bytesToUint(rx_frame->data, 4, 2) / 100.00) - 100;
  // Serial.printf("%3.2f km/h\n",Speed);
  // Raw speed data is given in km/h with a 100 km/h offset.
  AcceleratorPosition = bytesToUint(rx_frame->data, 6, 1) / 2.00;
  // Serial.printf("Accel = %3.2f \%\n",AcceleratorPosition);
  // This is a percentage, increments of 0.5%
  if ((int)Speed != 0 && (int)EngineRPM != 0) {
    // Serial.printf("%5.2f rpm, %3.2f km/h\n",EngineRPM,Speed);
    GearRatio = (EngineRPM * 60) / FINAL_RATIO * (TYRE_OUTER_DIAMETER_16 / 100000) / Speed;
  } else {
    GearRatio = 0;
  }

  if (clutch) {
    ShiftPosition = 'C';
  } else {
    if (neutral) {
      ShiftPosition = 'N';
    } else {
      if (GearRatio == 0) {
        ShiftPosition = 'N';
      } else if (GearRatio < (GEAR_RATIO_6 + GEAR_RATIO_5) / 2) {
        ShiftPosition = '6';
      } else if (GearRatio < (GEAR_RATIO_5 + GEAR_RATIO_4) / 2) {
        ShiftPosition = '5';
      } else if (GearRatio < (GEAR_RATIO_4 + GEAR_RATIO_3) / 2) {
        ShiftPosition = '4';
      } else if (GearRatio < (GEAR_RATIO_3 + GEAR_RATIO_2) / 2) {
        ShiftPosition = '3';
      } else if (GearRatio < (GEAR_RATIO_2 + GEAR_RATIO_1) / 2) {
        ShiftPosition = '2';
      } else {
        ShiftPosition = '1';
      }
    }
  }
}

void mazdaMx5Transmission(twai_message_t* rx_frame) {

  clutch = bitToUint(rx_frame->data, 15);
  // Serial.printf("clutch = %d\n", clutch);
  neutral = bitToUint(rx_frame->data, 14);
  // Serial.printf("neutral = %d\n", neutral);
}

void mazdaMx5Steering(twai_message_t* rx_frame) {

  SteeringAngle = bytesToInt(rx_frame->data, 2, 2);
  // Serial.printf("Steering %-d\n",SteeringAngle);
}

void mazdaMx5Brake(twai_message_t* rx_frame) {
  // BrakePressure = (3.4518689053 * bytesToInt(rxBuf, 0, 2) - 327.27) / 1000.00;
  // BrakePercentage = min(0.2 * (bytesToInt(rxBuf, 0, 2) - 102), 100);
  BrakePercentage = 0.2 * (bytesToInt(rx_frame->data, 0, 2) - 102);
  if (100 < BrakePercentage) {
    BrakePercentage = 100;
  }
  // Serial.printf("Brake = %3.2f \%\n",BrakePercentage);
}

void mazdaMx5OutputCsv() {
  // Serial.println("mazdaMx5OutputCsv()");
  Serial.printf(", %3.1f, %4.1f, %c, %3.1f, %3.1f, %-2.1f\n", Speed, EngineRPM, ShiftPosition, AcceleratorPosition, BrakePercentage, SteeringAngle * MAX_STEERING_ANGLE / STEERING_MAX);
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
          mazdaMx5EngineSpeed(&rx_frame);
          break;
        case CAN_ID_TRANSMISSION:
          mazdaMx5Transmission(&rx_frame);
          break;
        case CAN_ID_STEERLING:
          mazdaMx5Steering(&rx_frame);
          break;
        case CAN_ID_BRAKE:
          mazdaMx5Brake(&rx_frame);
          break;
        case CAN_ID_ENGINE:
          mazdaMx5OutputCsv();
          break;
          // default:
          // Serial.printf("Unexpected can frame received. rx_frame.identifier=%3x\n", rx_frame.identifier);
      }
    }
  }
}
