#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

namespace Protocol {

constexpr uint32_t MAGIC = 0x50495053;

enum Status : uint8_t
{
    STATUS_OK = 0x00,
    STATUS_UNKNOWN_COMMAND = 0x01,
    STATUS_INVALID_PACKET = 0x02,
    STATUS_GLOBAL_ERROR = 0x03
};

inline const char* statusToString(uint8_t status)
{
    switch (status) {
    case STATUS_OK:
        return "OKAY";

    case STATUS_UNKNOWN_COMMAND:
        return "НЕИЗВЕСТНАЯ КОМАНДА";

    case STATUS_INVALID_PACKET:
        return "НЕКОРРЕКТНЫЙ ПАКЕТ";

    default:
        return "НЕИЗВЕСТНЫЙ СТАТУС";
    }
}

enum Command : uint8_t
{
    CMD_SPI_WRITE = 0x00,
    CMD_SPI_READ  = 0x01,
    CMD_CHECK_STROBE = 0x10,
    CMD_STROBE_PERIOD = 0x11,
    CMD_STROBE_PULSE = 0x12,
    CMD_SYNC_SOURCE    = 0x20,
    CMD_SYNC_FREQUENCY = 0x21,
    CMD_EEPROM_WRITE_128 = 0x30,
    CMD_EEPROM_WRITE_256 = 0x31,
    CMD_EEPROM_WRITE_512 = 0x32,

    CMD_EEPROM_READ_128  = 0x33,
    CMD_EEPROM_READ_256  = 0x34,
    CMD_EEPROM_READ_512  = 0x35
};

enum Flags : uint8_t
{
    FLAG_REQUEST  = 0x00,
    FLAG_RESPONSE = 0x01,
    FLAG_ERROR = 0x02
};

enum SyncSource : uint8_t
{
    SYNC_SOURCE_INTERNAL = 0x00,
    SYNC_SOURCE_EXTERNAL = 0x01
};

#pragma pack(push, 1)
struct Header
{
    uint32_t magic;
    uint8_t  command;
    uint8_t  messageId;
    uint8_t  flags;
    uint32_t payloadSize;
};
#pragma pack(pop)

constexpr std::size_t HEADER_SIZE = sizeof(Header);

#pragma pack(push, 1)
struct SpiRequest : public Header
{
    uint8_t slave_id;           // идентификатор slave (AD, HMC, LMK)
    uint8_t ic_addr;            // адрес обращения регистра slave (адрес на самой микросхеме)
    uint32_t data_request;      // данные (если запрос на чтение, то отправляем пустоту)
};

struct SpiResponse : public Header
{
    uint8_t  status;
    uint32_t data_responce;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct StrobeRequest : public Header
{
    uint32_t value;   // период (мкс) или длительность (мс)
};

struct StrobeResponse : public Header
{
    uint8_t  status;
    uint32_t value;   // подтверждённое / применённое значение
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SyncRequest : public Header
{
    uint8_t  source;     // 0 - internal, 1 - external
    uint32_t frequency;  // в МГц, используется только для internal
};

struct SyncResponse : public Header
{
    uint8_t  status;
    uint8_t  source;
    uint32_t frequency;  // подтверждённая/применённая частота
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromWriteResponse : public Header
{
    uint8_t status;
    uint32_t address;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromWrite128Request : public Header
{
    uint32_t address;
    uint8_t data[128];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromWrite256Request : public Header
{
    uint32_t address;
    uint8_t data[256];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromWrite512Request : public Header
{
    uint32_t address;
    uint8_t data[512];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead128Request : public Header
{
    uint32_t address;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead256Request : public Header
{
    uint32_t address;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead512Request : public Header
{
    uint32_t address;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead128Response : public Header
{
    uint8_t status;
    uint32_t address;
    uint8_t data[128];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead256Response : public Header
{
    uint8_t status;
    uint32_t address;
    uint8_t data[256];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EepromRead512Response : public Header
{
    uint8_t status;
    uint32_t address;
    uint8_t data[512];
};
#pragma pack(pop)

}

#endif // PROTOCOL_H
