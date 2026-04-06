#include "tcp_server.h"
#include "protocol.h"

#include <QDebug>
#include <cstring>

// Конструктор TcpServer
TcpServer::TcpServer(QObject *parent) : QObject(parent),
                                server(new QTcpServer(this)),
                                socket(nullptr)
{
    connect(server, &QTcpServer::newConnection,
            this, &TcpServer::onNewConnection);
}

// Деструктор TcpServer
TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::start(int port)
{
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Не удалось запустить сервер по данному адресу";
        return false;
    }

    qDebug() << "Сервер запущен, порт: " << port;
    return true;
}

void TcpServer::stop()
{
    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
        socket = nullptr;
    }

    server->close();
}

// Обработка подключения Клиента
void TcpServer::onNewConnection()
{
    socket = server->nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead,
            this, &TcpServer::onReadyRead);

    connect(socket, &QTcpSocket::disconnected,
            this, &TcpServer::onDisconnected);

    buffer.clear();

    qDebug() << "Клиент успешно подключен";
    emit clientConnected();
}

// Отключение Клиента
void TcpServer::onDisconnected()
{
    qDebug() << "Клиент отключился";

    socket->deleteLater();
    socket = nullptr;

    emit clientDisconnected();
}

// Чтение данных
void TcpServer::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true)
    {
        // Проверка на размер пакета
        if (buffer.size() < static_cast<int>(Protocol::HEADER_SIZE))
            return;

        // Копируем данные в header
        Protocol::Header header;
        std::memcpy(&header, buffer.constData(), sizeof(header));

        // Проверка MAGIC
        if (header.magic != Protocol::MAGIC)
        {
            qDebug() << "Некорректное начало пакета";
            buffer.clear();
            return;
        }

        int fullSize = sizeof(Protocol::Header) + sizeof(Protocol::SpiRequest);

        // Ждём полный пакет
        if (buffer.size() < fullSize)
            return;

        // Вырезаем полный пакет
        QByteArray packet = buffer.left(fullSize);

        // Отправляем дальше в parser
        emit dataReceived(packet);

        // Очищаем буфер
        buffer.clear();
    }
}

// Отправляем данные Клиенту
void TcpServer::sendData(const QByteArray &data)
{
    if (socket)
    {
        socket->write(data);
    }
}
