#include "bms.h"
#include "rs485.h"

#include <string.h>

#define BMS_FRAME_HEADER       0x55U
#define BMS_SEND_FLAG          0x40U
#define BMS_CRC_INITIAL        0x3692U
#define BMS_CRC_POLY_REFLECTED 0x8408U
#define BMS_MAX_FRAME_SIZE     0x30U

static uint16_t g_bms_sequence;

/**
 * @brief 生成下一帧的帧序号。
 * @return 首次调用返回 0x0000，之后每次递增 1；0xFFFF 后回到 0x0000。
 * @note 组帧时按照低字节在前、高字节在后的顺序写入报文。
 */
uint16_t BMS_NextSequence(void)
{
    uint16_t sequence = g_bms_sequence;
    ++g_bms_sequence;
    return sequence;
}

static int BMS_SendFrame(uint16_t sequence,const uint8_t address[4],uint8_t command_low,const uint8_t *payload,uint8_t payload_length)
{
    uint8_t frame[BMS_MAX_FRAME_SIZE];
    uint8_t frame_length;
    uint16_t crc;

    frame_length = (uint8_t)(13U + payload_length);
    if ((address == NULL) || (frame_length > sizeof(frame)) ||
        ((payload_length > 0U) && (payload == NULL)))
    {
        return -1;
    }

    frame[0] = BMS_FRAME_HEADER;
    frame[1] = frame_length;
    memcpy(&frame[2], address, 4U);
    frame[6] = (uint8_t)sequence;
    frame[7] = (uint8_t)(sequence >> 8U);
    frame[8] = BMS_SEND_FLAG;
    frame[9] = 0x0DU;
    frame[10] = command_low;
    if (payload_length > 0U)
    {
        memcpy(&frame[11], payload, payload_length);
    }

    crc = BMS_Crc16(frame, (uint16_t)(frame_length - 2U));
    frame[frame_length - 2U] = (uint8_t)crc;
    frame[frame_length - 1U] = (uint8_t)(crc >> 8U);

    return Rs485_SendData(frame, frame_length);
}

/**
 * @brief 计算 BMS 协议 CRC-16。
 * @param data 待计算数据，范围从帧头 0x55 开始，不包含帧尾 CRC。
 * @param length data 中参与计算的字节数。
 * @return CRC 计算结果；发送时低字节在前、高字节在后。
 */
uint16_t BMS_Crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = BMS_CRC_INITIAL;
    uint8_t bit;

    if (data == NULL)
    {
        return 0U;
    }

    while (length-- > 0U)
    {
        crc ^= *data++;
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ BMS_CRC_POLY_REFLECTED);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

/**
 * @brief 发送功能码 0x0D01：读取电池型号、固件版本和基础配置。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendReadBasicInfo(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0xFCU, 0xE5U, 0x0BU};
    static const uint8_t payload[9] = {0U};
    return BMS_SendFrame(sequence, address, 0x01U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D02：读取总压、电流、容量、温度等实时状态。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendReadRealtimeStatus(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x92U, 0xE5U, 0x0BU};
    static const uint8_t payload[4] = {0U};
    return BMS_SendFrame(sequence, address, 0x02U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D03：读取 14 串单体电压。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendReadCellVoltages(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x92U, 0xE5U, 0x0BU};
    static const uint8_t payload[4] = {0U};
    return BMS_SendFrame(sequence, address, 0x03U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D04：读取 BMS/电池序列号并检测设备在线。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 使用协议表中第一种请求：帧长 0x16，数据为 9 个 0x00。
 */
int BMS_SendReadSerialNumber(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0xFCU, 0xE5U, 0x0BU};
    static const uint8_t payload[9] = {0U};
    return BMS_SendFrame(sequence, address, 0x04U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0DC0：读取充电状态/控制状态。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 使用协议表中第一种请求：控制数据全部填写 0x00。
 */
int BMS_SendReadChargeState(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0xFCU, 0xE5U, 0x0BU};
    static const uint8_t payload[9] = {0U};
    return BMS_SendFrame(sequence, address, 0xC0U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0DC0：请求 BMS 允许充电。
 * @param sequence 帧序号，使用 BMS_NextSequence() 的返回值。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 固定数据为 00 01 00 00 00 00 00 00 00。
 */
int BMS_SendEnableCharging(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0xFCU, 0xE5U, 0x0BU};
    static const uint8_t payload[9] = {
        0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U
    };
    return BMS_SendFrame(sequence, address, 0xC0U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0DC4：读取固定硬件或充电参数。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendReadHardwareParams(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x66U, 0xE5U, 0x0BU};
    static const uint8_t payload[1] = {0U};
    return BMS_SendFrame(sequence, address, 0xC4U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D23、认证类型 0x30 的固定设备标识报文。
 * @param sequence 帧序号，使用 BMS_NextSequence() 的返回值。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 固定数据包含 ASCII 设备标识 "09KQZNC200200AY" 和 18 个 0x00。
 */
int BMS_SendAuthentication30(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x97U, 0xE5U, 0x0BU};
    static const uint8_t payload[35] = {
        0x00U, 0x30U,
        0x30U, 0x39U, 0x4BU, 0x51U, 0x5AU, 0x4EU, 0x43U, 0x32U,
        0x30U, 0x30U, 0x32U, 0x30U, 0x30U, 0x41U, 0x59U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U
    };
    return BMS_SendFrame(sequence, address, 0x23U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D23、认证类型 0x30 的固定会话认证报文。
 * @param sequence 帧序号，使用 BMS_NextSequence() 的返回值。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 与初次类型 0x30 报文不同，尾部包含原装充电器的固定会话数据。
 */
int BMS_SendAuthentication30Session(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x97U, 0xE5U, 0x0BU};
    static const uint8_t payload[35] = {
        0x00U, 0x30U,
        0x30U, 0x39U, 0x4BU, 0x51U, 0x5AU, 0x4EU, 0x43U, 0x32U,
        0x30U, 0x30U, 0x32U, 0x30U, 0x30U, 0x41U, 0x59U, 0x00U,
        0xCBU, 0xD3U, 0x49U, 0xF3U, 0x8CU, 0x7DU, 0xB2U, 0x42U,
        0x56U, 0xEFU, 0xABU, 0xF5U, 0x71U, 0x37U, 0xA9U, 0xC3U,
        0x48U
    };
    return BMS_SendFrame(sequence, address, 0x23U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D23、认证类型 0x31 的固定认证数据报文。
 * @param sequence 帧序号，使用 BMS_NextSequence() 的返回值。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendAuthentication31(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x97U, 0xE5U, 0x0BU};
    static const uint8_t payload[35] = {
        0x00U, 0x31U,
        0x30U, 0xB5U, 0xE4U, 0x72U, 0xBCU, 0xC1U, 0xD5U, 0xE1U,
        0xBDU, 0x06U, 0x57U, 0xA4U, 0xE6U, 0x77U, 0x15U, 0x00U,
        0xCBU, 0xD3U, 0x49U, 0xF3U, 0x8CU, 0x7DU, 0xB2U, 0x42U,
        0x56U, 0xEFU, 0xABU, 0xF5U, 0x71U, 0x37U, 0xA9U, 0xC3U,
        0x48U
    };
    return BMS_SendFrame(sequence, address, 0x23U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D23、认证类型 0x32 的固定会话报文。
 * @param sequence 帧序号，使用 BMS_NextSequence() 的返回值。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 固定数据为 0x30，后跟 32 个 0x00。
 */
int BMS_SendAuthentication32(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x97U, 0xE5U, 0x0BU};
    static const uint8_t payload[35] = {0x00U, 0x32U, 0x30U};
    return BMS_SendFrame(sequence, address, 0x23U, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0DDA：读取扩展状态数据。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 */
int BMS_SendReadExtendedStatus(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0x66U, 0xE5U, 0x0BU};
    static const uint8_t payload[1] = {0U};
    return BMS_SendFrame(sequence, address, 0xDAU, payload, sizeof(payload));
}

/**
 * @brief 发送功能码 0x0D12：通知 BMS 充电器已经停止充电。
 * @param sequence 帧序号，从 0x0000 开始逐次递增；请求和应答使用相同序号。
 * @return 0 表示发送成功，-1 表示发送失败。
 * @note 数据固定为：00 46 37 28 19 EF BE AD DE 00 00 00 00。
 */
int BMS_SendChargeStopped(uint16_t sequence)
{
    static const uint8_t address[4] = {0x04U, 0xB1U, 0xE5U, 0x0BU};
    static const uint8_t payload[13] = {
        0x00U, 0x46U, 0x37U, 0x28U, 0x19U, 0xEFU, 0xBEU,
        0xADU, 0xDEU, 0x00U, 0x00U, 0x00U, 0x00U
    };
    return BMS_SendFrame(sequence, address, 0x12U, payload, sizeof(payload));
}
