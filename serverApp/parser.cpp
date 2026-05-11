#include "parser.h"
#include "protocol.h"
#include "spi_controller.h"
#include "ad9122.h"
#include "i2c_controler.h"
Parser::Parser(QObject *parent) : QObject(parent) {}

uint32_t result;
uint8_t status;

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
    default:
        qDebug() << "Неизвестная команда";
        break;
    }
}

void Parser::handleRequest(const QByteArray &data)
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
    switch (req->command) {
    case Protocol::CMD_SPI_WRITE:
        // spi_writeReg(req->slave_id, req->ic_addr, req->data_request);
        break;
    case Protocol::CMD_SPI_READ:
        // result = spi_readReg(req->slave_id, req->ic_addr);
        break;
    // case Protocol::CMD_I2C_WRITE:
    //     break;
    // case Protocol::CMD_I2C_READ:
    //     break;
    default:
        status = Protocol::STATUS_UNKNOWN_COMMAND;
        break;
    }

send_response:
    qDebug() << "Статус операции: " <<  Protocol::statusToString(status);

    // Формируем ответ
    Protocol::SpiResponse resp;

    resp.magic = Protocol::MAGIC;
    resp.command = req->command;
    resp.messageId = req->messageId;
    resp.flags = Protocol::FLAG_RESPONSE;
    resp.payloadSize = sizeof(Protocol::SpiResponse) - sizeof(Protocol::Header);

    resp.status = status;
    resp.data_responce = result;

    // Упаковываем в QByteArray
    QByteArray response(reinterpret_cast<const char*>(&resp), sizeof(resp));

    // Отправляем обратно
    emit responseReady(response);
}
