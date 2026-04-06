#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer (QObject *parent = nullptr);
    ~TcpServer();
    bool start(int port);
    void stop();

signals:
    void dataReceived(const QByteArray &data);
    void clientConnected();
    void clientDisconnected();

public slots:
    void sendData(const QByteArray &data);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer *server;
    QTcpSocket *socket;
    QByteArray buffer;
};

#endif // TCP_SERVER_H
