#include "tcp_server.h"
#include "protocol.h"

// Конструктор TcpServer
TcpServer::TcpServer(Parser *parser, int port, QObject *parent) : QObject(parent),
                                server(new QTcpServer(this))
{
    connect(this, &TcpServer::dataReceived,  parser, &Parser::process);
    connect(parser, &Parser::responseReady, this, &TcpServer::sendData);

    connect(server, &QTcpServer::newConnection,
            this, &TcpServer::onNewConnection);

    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Не удалось запустить сервер по данному адресу";
    }

    qDebug() << "Сервер запущен, порт: " << port;
}

// Деструктор TcpServer
TcpServer::~TcpServer()
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
}

// Отключение Клиента
void TcpServer::onDisconnected()
{
    qDebug() << "Клиент отключился";
}

// Чтение данных
void TcpServer::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true)
    {
        // Проверка на размер пакета
        if (buffer.size() < Protocol::HEADER_SIZE)
            return;

        // Преобразуем байты в структуру заголовка
        const Protocol::Header* header = reinterpret_cast<const Protocol::Header*>(buffer.constData());

        // Проверка MAGIC
        if (header->magic != Protocol::MAGIC)
        {
            qDebug() << "Некорректное начало пакета";
            buffer.clear();
            return;
        }

        int fullSize = sizeof(Protocol::SpiRequest);

        // Ждём полный пакет
        if (buffer.size() < fullSize)
            return;

        // Отправляем дальше в parser
        emit dataReceived(buffer);

        // Очищаем буфер
        buffer.remove(0, fullSize);
    }
}

// Отправляем данные Клиенту
void TcpServer::sendData(const QByteArray &data)
{
        socket->write(data);
}
