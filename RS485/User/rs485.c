#include "rs485.h"

#include <string.h>

#define RXBUFFERSIZE 1
#define RS485_RX_BUFFER_SIZE 200U
#define RS485_FRAME_GAP_MS 5U
extern UART_HandleTypeDef huart3;

static uint8_t g_usart_rx_buf[RS485_RX_BUFFER_SIZE];
static volatile uint16_t g_usart_rx_len;
static volatile uint32_t g_usart_rx_tick;
static uint8_t g_rx_buffer[RXBUFFERSIZE];

void MX_RS485_Init(void)
{
	RS485_Receive();
	g_usart_rx_len = 0U;
	g_usart_rx_tick = 0U;
	if (HAL_UART_Receive_IT(&huart3, g_rx_buffer, RXBUFFERSIZE) != HAL_OK)
	{
		Error_Handler();
	}
}
void RS485_Send(void)
{
    HAL_GPIO_WritePin(DE_RE_GPIO_Port, DE_RE_Pin, GPIO_PIN_SET);
}

void RS485_Receive(void)
{
    HAL_GPIO_WritePin(DE_RE_GPIO_Port, DE_RE_Pin, GPIO_PIN_RESET);
}

/**
 * @description: 控制rs485发送数据
 * @param {uint8_t} *data 要发送的数据
 * @param {size_t} len  要发送数据的长度
 * @return {*}   0 ：发送成功；-1：发送失败；
 */
int Rs485_SendData(const uint8_t *data, size_t len)
{
     HAL_StatusTypeDef status;

     if ((data == NULL) || (len == 0U) || (len > UINT16_MAX))
     {
         return -1;
     }

     RS485_Send();
     /* Allow the RS485 driver to become active before the UART start bit. */
     HAL_Delay(1U);
     status = HAL_UART_Transmit(&huart3, (uint8_t *)data, (uint16_t)len, HAL_MAX_DELAY);
     while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET)
     {
     }
     RS485_Receive();
     return status == HAL_OK ? 0 : -1;
}

size_t Rs485_ReadData(uint8_t *data, size_t capacity)
{
	uint16_t length;

	if ((data == NULL) || (capacity == 0U) || (g_usart_rx_len == 0U))
	{
		return 0U;
	}

	__disable_irq();
	if ((g_usart_rx_len < sizeof(g_usart_rx_buf)) &&
		((HAL_GetTick() - g_usart_rx_tick) < RS485_FRAME_GAP_MS))
	{
		__enable_irq();
		return 0U;
	}

	length = g_usart_rx_len;
	if (length > capacity)
	{
		length = (uint16_t)capacity;
	}
	memcpy(data, g_usart_rx_buf, length);
	g_usart_rx_len = 0U;
	__enable_irq();

	return length;
}
volatile uint8_t debug_rx_byte;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{  
	if(huart->Instance == USART3)     
    {
		debug_rx_byte = g_rx_buffer[0];
		if (g_usart_rx_len < sizeof(g_usart_rx_buf)) {
			g_usart_rx_buf[g_usart_rx_len++] = g_rx_buffer[0];
			g_usart_rx_tick = HAL_GetTick();
		}
		(void)HAL_UART_Receive_IT(&huart3, g_rx_buffer, RXBUFFERSIZE);
	}
}

