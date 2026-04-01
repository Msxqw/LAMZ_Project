// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

namespace Protocol {

constexpr uint32_t MAGIC = 0x53504950;

enum Command : uint8_t
{
    CMD_SPI_WRITE = 0x00,
    CMD_SPI_READ  = 0x01
};

enum Flags : uint8_t
{
    FLAG_REQUEST  = 0x00,
    FLAG_RESPONSE = 0x01,
    FLAG_ERROR = 0x02
};

#pragma pack(push, 1)
struct Header
{
    uint32_t magic;
    uint8_t command;
    uint8_t messageId;
    uint8_t flags;
    uint32_t payloadSize;
};
#pragma pack(pop)

constexpr std::size_t HEADER_SIZE = sizeof(Header);

#pragma pack(push, 1)
struct SpiRequest : public Header
{
    uint8_t slave_id;
    uint8_t ic_addr;
    uint32_t data_request;
};

struct SpiResponse : public Header
{
    uint8_t  status;
    uint32_t data_responce;
};
#pragma pack(pop)

}

#endif // PROTOCOL_H
