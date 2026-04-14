#include "parser.h"
#include "protocol.h"
#include "spi_controller.h"

Parser::Parser(QObject *parent) : QObject(parent) {}

void Parser::process(const QByteArray &data)
{
    // Проверка минимального размера пакета
    if (data.size() < Protocol::HEADER_SIZE) {
        qDebug() << "Некорректный размер заголовка";
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

void Parser::handleSpiRequest(const QByteArray &data)
{
    // Проверка размера пакета запроса Клиента
    if (data.size() < sizeof(Protocol::SpiRequest)) {
        qDebug() << "Некорректный размер запроса SpiRequest";
        return;
    }

    // Преобразуем байты в структуру запроса
    const Protocol::SpiRequest *req = reinterpret_cast<const Protocol::SpiRequest*>(data.constData());

    uint32_t result = 0;
    uint8_t status = 1; // Все хорошо

    // Логирование для проверки корректности Request (!!!в дальнейшем сделать перегрузку оператора!!!)
    qDebug() << "SPI request:"
             << "cmd=" << req->command
             << "slave=" << req->slave_id
             << "ic_addr=" << req->ic_addr
             << "data=" << req->data_request;

    // Выполнение команды
    switch (req->command) {
    case Protocol::CMD_SPI_WRITE:
        writeReg(req->ic_addr, req->data_request);
        break;
    case Protocol::CMD_SPI_READ:
        result = readReg(req->ic_addr);
        break;
    default:
        status = 0;
        qDebug() << "Ошибка выполнения команды";
        break;
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
