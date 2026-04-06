#include "parser.h"
#include "protocol.h"
#include "spi_controller.h"

Parser::Parser(QObject *parent) : QObject(parent) {}

void Parser::process(const QByteArray &data)
{
    // Проверка минимального размера пакета
    if (data.size() < static_cast<int>(Protocol::HEADER_SIZE)) {
        qDebug() << "Некорректный размер заголовка";
        return;
    }

    // Преобразуем байты в структуру заголовка
    const Protocol::Header *header = reinterpret_cast<const Protocol::Header*>(data.constData());

    switch (header.command)
    {
    case Protocol::CMD_SPI_WRITE:
        handleSpiRequest(data);
        break;
    case Protocol::CMD_SPI_READ:
        handleSpiRequest(data);
        break;
    default:
        qDebug() << "Неизвестная команда";
        break;
    }
}

void Parser::handleSpiRequest(const QByteArray &data)
{
    // Проверка размера пакета запроса Клиента
    if (data.size() < static_cast<int>(sizeof(Protocol::SpiRequest))) {
        qDebug() << "Некорректный размер запроса SpiRequest";
        return;
    }

    // Преобразуем байты в структуру запроса
    const Protocol::SpiRequest *req = reinterpret_cast<const Protocol::SpiRequest*>(data.constData());

    // Извлекаем поля
    uint8_t slave_id = req->slave_id;
    uint8_t addr     = req->ic_addr;
    uint32_t value   = req->data_request;

    uint32_t result = 0;
    uint8_t status = 1; // Все хорошо

    // Логирование для проверки корректности Request
    qDebug() << "SPI request:"
             << "cmd=" << req->command
             << "slave=" << slave_id
             << "ic_addr=" << addr
             << "data=" << value;

    // Выполнение команды
    if (req->command == Protocol::CMD_SPI_WRITE)
    {
        writeReg(addr, value);
        result = 0;
    }
    else if (req->command == Protocol::CMD_SPI_READ)
    {
        result = readReg(addr);
    }
    else
    {
        status = 0; // ошибка
    }

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
