//-------------------------------------------------------------------------------------//
// SUBARU 2nd GEN Levorg(VN5) SPEC
// https://scdam.subaru.jp/20230925/20230925104506levorg_specifications.pdf
//-------------------------------------------------------------------------------------//

#define TYRE_OUTER_DIAMETER_17            2031.9821    // 215/50R17       
#define TYRE_OUTER_DIAMETER_18            2072.5087    // 225/45R18
#define WHEEL_BASE                           2.670
#define TURNING_CIRCLE                       5.500
#define MAX_STEERING_ANGLE                   (asin(WHEEL_BASE / TURNING_CIRCLE) * 180 / M_PI)
#define REV_LIMIT                             6000
#define STEERING_LOCK_TO_LOCK                  2.8
#define STEERING_MAX                           (STEERING_LOCK_TO_LOCK * 360 / 2)
#define STEERING_MIN                           (- STEERING_MAX)
#define SPEED_WARNING                           80
#define SPEED_WARNING_ON                      true
#define SPEED_WARNING_OFF                    false
#define GEAR_RATIO_1                         4.065
#define GEAR_RATIO_2                         2.600
#define GEAR_RATIO_3                         1.827
#define GEAR_RATIO_4                         1.377
#define GEAR_RATIO_5                         1.061
#define GEAR_RATIO_6                         0.836
#define GEAR_RATIO_7                         0.667
#define GEAR_RATIO_8                         0.559
#define FINAL_RATIO                          3.900

#define CAN_ID_TRANSMISSION                  0x048

//-------------------------------------------------------------------------------------//
// CAN IDs
// https://github.com/timurrrr/ft86/blob/main/can_bus/gen2.md
//-------------------------------------------------------------------------------------//
// 
#define CAN_ID_ENGINE_SPEED                  0x040
// CAN ID 0x40 (64)
// Update frequency: 100 times per second.
//
// Example values:
// 0x 5D 0B C9 82 00 00 00 C7 (parked)
// 0x 10 0D FA 06 00 00 00 C3 (moving slowly)
//
// Channel name	Equation	Notes
// Engine RPM	bitsToUIntLe(raw, 16, 14)	
// Accelerator position	E / 2.55	
// Accelerator position	F / 2.55	Seems to always have the same value as E
// Accelerator position	G / 2.55	Seems to always have the same value as E
// ???	H & 0xC0	0xC0 when off the accelerator pedal, 0x00 otherwise.
//
// CAN ID 0x41 (65)
// Update frequency: 100 times per second.
//
// Example values:
// 0x C9 43 96 A6 7B 27 65 02 (parked)
// 0x 94 4E 44 A7 78 27 7B 00 (moving slowly)
//
// Channel name	Equation	Notes
// A/C fan clutch	H & 0x2	2 is engaged, 0 is disengaged
//
// CAN ID 0x118 (280)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 7E 0F 00 07 00 4F 00 00 (parked)
// 0x 8C 02 00 21 00 50 00 00 (moving slowly)
//
#define CAN_ID_STEERLING                  0x138
// CAN ID 0x138 (312)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 28 0C DD 06 00 00 00 00 (parked)
// 0x 90 0D 13 FA 3F 00 00 00 (moving slowly)
//
// Channel name	Equation	Notes
// Steering angle	bytesToIntLe(raw, 2, 2) * -0.1	Positive value = turning left. You can add a - if you prefer it the other way around.
// Yaw rate	bytesToIntLe(raw, 4, 2) * -0.2725	Calibrated against the gyroscope in RaceBox Mini. Gen1 used 0.286478897 instead.
//
#define CAN_ID_BRAKE                  0x139
// CAN ID 0x139 (313)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 72 5A 00 E0 08 00 DA 1C (parked)
// 0x 62 5B FC E0 08 00 CD 1C (moving slowly)
//
// Channel name	Equation	Notes
// Speed	bitsToUIntLe(raw, 16, 13) * 0.015694	You may want to check the multiplier against an external GPS device, especially if running larger/smaller diameter tires
// Brake lights switch	(E & 0x4)	4 is on, 0 is off
// Brake pressure	F * 128	Coefficient taken from 1st gen cars, but might need to be verified.
//
// CAN ID 0x13A (314)
// Available on C_CAN, which can be found at the power steering unit. This channel is not available on B_CAN which connects to the ASC unit.
//
// Channel name	Equation	Notes
// Wheel speed FL	bitsToUIntLe(raw, 12, 13) * 0.015694	Use same multiplier as for speed in 0x139
// Wheel speed FR	bitsToUIntLe(raw, 25, 13) * 0.015694	Use same multiplier as for speed in 0x139
// Wheel speed RL	bitsToUIntLe(raw, 38, 13) * 0.015694	Use same multiplier as for speed in 0x139
// Wheel speed RR	bitsToUIntLe(raw, 51, 13) * 0.015694	Use same multiplier as for speed in 0x139
//
// CAN ID 0x13B (315)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 47 0F 00 00 FF FF FF FF (parked)
// 0x 37 02 00 00 FF FF FE FD (moving slowly)
//
// Channel name	Equation	Notes
// Lateral acceleration	bytesToIntLe(raw, 6, 1) * 0.2	
// Longitudinal acceleration	bytesToIntLe(raw, 7, 1) * -0.1	
// Combined acceleration	sqrt(pow2(bytesToIntLe(raw, 6, 1) * 0.2) + pow2(bytesToIntLe(raw, 7, 1) * 0.1))	
//
// CAN ID 0x13C (316)
// Update frequency: 50 times per second.
//
// Example values:
// 0x D4 0F 0E 38 02 00 40 00 (parked)
// 0x 12 0E 00 0B 0E 42 6C 00 (moving slowly)
//
// CAN ID 0x143 (323)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 51 0D 00 00 00 00 00 00 (parked)
// 0x 5C 07 00 10 01 00 00 00 (moving slowly)
//
// Channel name	Equation	Notes
// Speed	bitsToUIntLe(raw, 24, 14) * 0.015694	You may want to check the multiplier against an external GPS device, especially if running larger/smaller diameter tires
//
// CAN ID 0x146 (326)
// Update frequency: 50 times per second.
//
// Example values:
// 0x 7C 1B CE 42 09 01 00 00 (parked)
// 0x 19 11 68 48 11 00 00 00 (moving slowly)
//
// CAN ID 0x241 (577)
// Update frequency: 20 times per second.
//
// Channel name	Equation	Notes
// Clutch position (%)	(F & 0x80) / 1.28	100 is "clutch pedal depressed", 0 is "clutch pedal released"
// Gear	bitsToUIntLe(raw, 35, 3)	0 for N, 1—6 for gears 1–6
//
// CAN ID 0x2D2 (722)
// Update frequency: 33.3 times per second.
//
// Example values:
// 0x 3A 06 40 00 20 00 00 00 (parked)
// 0x 3E 02 48 00 20 00 00 00 (moving slowly)
//
#define CAN_ID_ENGINE_TEMPERATURE                  0x345
// CAN ID 0x345 (837)
// Update frequency: 10 times per second.
//
// Channel name	Equation	Notes
// Engine oil temperature	D - 40	
// Coolant temperature	E - 40
//
#define CAN_ID_AIR_TEMPERATURE                  0x390
// CAN ID 0x390 (912)
// Update frequency: 10 times per second.
//
// Channel name	Equation	Notes
// Air Temperature	E / 2 - 40	
