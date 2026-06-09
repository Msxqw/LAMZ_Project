#include "parser.h"
#include "protocol.h"
#include "spi_controller.h"
#include "ad9122.h"
#include "i2c_controler.h"
Parser::Parser(QObject *parent) : QObject(parent) {}

uint32_t result;
uint8_t status;
uint32_t address;

void Parser::process(const QByteArray &data)
{
    qDebug() << "Parser::process начал обработку" << data.size() << "байт";

    // Проверка минимального размера пакета
    if (data.size() < Protocol::HEADER_SIZE) {
        qDebug() << "Некорректный размер заголовка";
        status = Protocol::STATUS_INVALID_PACKET;
        return;
    }

    // Преобразуем байты в структуру заголовка
    const Protocol::Header *header = reinterpret_cast<const Protocol::Header*>(data.constData());

    switch (header->command)
    {
    case Protocol::CMD_SPI_WRITE:
    case Protocol::CMD_SPI_READ:
        handleSpiRequest(data);
        break;
    case Protocol::CMD_EEPROM_WRITE:
    case Protocol::CMD_EEPROM_READ:
        handleI2CRequest(data);
        break;
    default:
        qDebug() << "Неизвестная команда";
        break;
    }
}

void Parser::handleSpiRequest(const QByteArray &data)
{
    // Проверка размера пакета запроса Клиента
    if (data.size() < sizeof(Protocol::SpiRequest)) {
        qDebug() << "Некорректный размер запроса SpiRequest";
        status = Protocol::STATUS_INVALID_PACKET;
        return;
    }

    // Преобразуем байты в структуру запроса
    const Protocol::SpiRequest *req = reinterpret_cast<const Protocol::SpiRequest*>(data.constData());

    // Логирование для проверки корректности Request (!!!в дальнейшем сделать перегрузку оператора!!!)
    qDebug() << "SPI request:"
             << "magic" << req->magic
             << "cmd=" << req->command
             << "slave=" << req->slave_id
             << "ic_addr=" << req->ic_addr
             << "data=" << req->data_request;

    // Проверка валидности адреса в зависимости от микросхемы
    bool addr_valid = false;

    switch (req->slave_id) {
    case 1:  // AD9122
        addr_valid = AD9122_IC::isValidRegister(req->ic_addr);
        break;
    case 2:
        addr_valid = true;
        break;
    case 3:
        addr_valid = true;
        break;
    default:
        qDebug() << "Неизвестный slave_id:" << req->slave_id;
        break;
    }

    // Если адрес невалидный - отправляем ошибку
    if (!addr_valid) {
        qDebug() << "Невалидный адрес регистра для slave_id:" << req->slave_id;
        status = Protocol::STATUS_INVALID_ICADDR;
        goto send_response;
    }
    // Выполнение команды
    else
    {
        switch (req->command) {
        case Protocol::CMD_SPI_WRITE:
            // spi_writeReg(req->slave_id, req->ic_addr, req->data_request);
            // status = Protocol::STATUS_OPERATION_WRITE_T;
            break;
        case Protocol::CMD_SPI_READ:
            if (req->slave_id == 3)
            {
                qDebug() << "Чтение из LMK01000 невозможно";
                status = Protocol::STATUS_INVALID_COMMAND_LMK;
            }
            else
            {
                // result = spi_readReg(req->slave_id, req->ic_addr);
                // status = Protocol::STATUS_OPERATION_READ_T;
            }
            break;
        default:
            status = Protocol::STATUS_UNKNOWN_COMMAND;
            break;
        }
    }

send_response:
    qDebug() << "Статус SPI операции: " <<  Protocol::statusToString(status);

    // Формируем ответ
    Protocol::SpiResponse resp;

    resp.magic = Protocol::MAGIC;
    resp.command = req->command;
    resp.messageId = req->messageId;
    resp.flags = Protocol::FLAG_RESPONSE;
    resp.payloadSize = sizeof(Protocol::SpiResponse) - sizeof(Protocol::Header);

    resp.status = status;
    resp.data_responce = result;

    QByteArray response(reinterpret_cast<const char*>(&resp), sizeof(resp));

    emit responseReady(response);
}

void Parser::handleI2CRequest(const QByteArray &data)
{
    const Protocol::Header *header = reinterpret_cast<const Protocol::Header*>(data.constData());

    switch (header->command) {
    case Protocol::CMD_EEPROM_WRITE:
    {
        if (data.size() < sizeof(Protocol::EepromWrite128Request)) {
            qDebug() << "Некорректный размер запроса";
            status = Protocol::STATUS_INVALID_PACKET;
            break;
        }

        const Protocol::EepromWrite128Request *req_wr_i2c = reinterpret_cast<const Protocol::EepromWrite128Request*>(data.constData());

        if (i2c_init(static_cast<uint8_t>(req_wr_i2c->slave_address)))
        {
            bool transaction_wr = i2c_write_buffer(req_wr_i2c->data, 128);
            status = transaction_wr ? Protocol::STATUS_OPERATION_WRITE_T : Protocol::STATUS_OPERATION_WRITE_F;
            i2c_deinit();
        }

        Protocol::EepromWriteResponse resp_wr_i2c;
        resp_wr_i2c.magic = Protocol::MAGIC;
        resp_wr_i2c.command = req_wr_i2c->command;
        resp_wr_i2c.messageId = req_wr_i2c->messageId;
        resp_wr_i2c.flags = Protocol::FLAG_RESPONSE;
        resp_wr_i2c.payloadSize = sizeof(Protocol::EepromWriteResponse) - sizeof(Protocol::Header);
        resp_wr_i2c.status = status;
        resp_wr_i2c.slave_address = req_wr_i2c->slave_address;

        QByteArray response_wr_i2c(reinterpret_cast<const char*>(&resp_wr_i2c), sizeof(resp_wr_i2c));

        emit responseReady(response_wr_i2c);
        break;
    }

    case Protocol::CMD_EEPROM_READ:
    {
        if (data.size() < sizeof(Protocol::EepromRead128Request)) {
            qDebug() << "Некорректный размер запроса";
            status = Protocol::STATUS_INVALID_PACKET;
            break;
        }

        const Protocol::EepromRead128Request *req_rd_i2c = reinterpret_cast<const Protocol::EepromRead128Request*>(data.constData());

        Protocol::EepromRead128Response resp_rd_i2c;
        resp_rd_i2c.magic = Protocol::MAGIC;
        resp_rd_i2c.command = req_rd_i2c->command;
        resp_rd_i2c.messageId = req_rd_i2c->messageId;
        resp_rd_i2c.flags = Protocol::FLAG_RESPONSE;
        resp_rd_i2c.payloadSize = sizeof(Protocol::EepromWriteResponse) - sizeof(Protocol::Header);
        resp_rd_i2c.slave_address = req_rd_i2c->slave_address;


        if (i2c_init(static_cast<uint8_t>(req_rd_i2c->slave_address)))
        {
            bool transaction_rd = i2c_read_buffer(resp_rd_i2c.data, 128);
            status = transaction_rd ? Protocol::STATUS_OPERATION_WRITE_T : Protocol::STATUS_OPERATION_WRITE_F;
            i2c_deinit();
        }

        resp_rd_i2c.status = status;
        QByteArray response_rd_i2c(reinterpret_cast<const char*>(&resp_rd_i2c), sizeof(resp_rd_i2c));

        emit responseReady(response_rd_i2c);
        break;
    }

    default:
        qDebug() << "Неизвестная I2C команда";
        status = Protocol::STATUS_UNKNOWN_COMMAND;
        break;
    }

    qDebug() << "Статус I2C операции:" << Protocol::statusToString(status);
}
