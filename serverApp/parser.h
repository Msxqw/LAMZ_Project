#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QByteArray>
#include <QDebug>

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser(QObject *parent = nullptr);

public slots:
    // Прием данных от Клиента
    void process(const QByteArray &data);

signals:
    // Отправка ответа Клиенту
    void responseReady(const QByteArray &data);

private:
    // Обработка SPI-команд
    void handleSpiRequest(const QByteArray &data);
};

#endif // PARSER_H
