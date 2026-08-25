#ifndef __BMS_H__
#define __BMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

uint16_t BMS_Crc16(const uint8_t *data, uint16_t length);
uint16_t BMS_NextSequence(void);

int BMS_SendReadBasicInfo(uint16_t sequence);
int BMS_SendReadRealtimeStatus(uint16_t sequence);
int BMS_SendReadCellVoltages(uint16_t sequence);
int BMS_SendReadSerialNumber(uint16_t sequence);
int BMS_SendReadChargeState(uint16_t sequence);
int BMS_SendEnableCharging(uint16_t sequence);
int BMS_SendReadHardwareParams(uint16_t sequence);
int BMS_SendAuthentication30(uint16_t sequence);
int BMS_SendAuthentication30Session(uint16_t sequence);
int BMS_SendAuthentication31(uint16_t sequence);
int BMS_SendAuthentication32(uint16_t sequence);
int BMS_SendReadExtendedStatus(uint16_t sequence);
int BMS_SendChargeStopped(uint16_t sequence);

#ifdef __cplusplus
}
#endif

#endif
