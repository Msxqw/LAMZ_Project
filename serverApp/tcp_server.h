#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QDebug>
#include "parser.h"

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer (Parser *parser, int port, QObject *parent = nullptr);
    ~TcpServer();

signals:
    void dataReceived(const QByteArray &data);

public slots:
    void sendData(const QByteArray &data);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer *server;
    QTcpSocket *socket = nullptr;
    QByteArray buffer;
};

#endif // TCP_SERVER_H
