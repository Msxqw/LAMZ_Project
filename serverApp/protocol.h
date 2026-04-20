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

    /*Команды (на запись/чтение)*/
    enum Command : uint8_t
    {
        CMD_SPI_WRITE = 0x00,
        CMD_SPI_READ  = 0x01
    };

    /*Флаги*/
    enum Flags : uint8_t
    {
        FLAG_REQUEST  = 0x00,
        FLAG_RESPONSE = 0x01,
        FLAG_ERROR = 0x02
    };

    /*Структура заголовка*/
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

    /*Структура пакетов*/
    #pragma pack(push, 1)
    struct SpiRequest : public Header
    {
        uint8_t slave_id;               // идентификатор slave (AD, HMC, LMK)
        uint8_t ic_addr;                // адрес обращения регистра slave (адрес на самой микросхеме)
        uint32_t data_request;        // данные (если запрос на чтение, то отправляем пустоту или просто опускаем)
    };

    struct SpiResponse : public Header
    {
        uint8_t  status;
        uint32_t data_responce;
    };

    #pragma pack(pop)
}

#endif // PROTOCOL_H
