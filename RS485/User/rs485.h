#ifndef __RS485_H__
#define __RS485_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stddef.h>



void MX_RS485_Init(void);
int Rs485_SendData(const uint8_t *data, size_t len);
size_t Rs485_ReadData(uint8_t *data, size_t capacity);
void RS485_Send(void);
void RS485_Receive(void);


#ifdef __cplusplus
}
#endif
#endif



