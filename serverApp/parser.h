#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QByteArray>

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser(QObject *parent = nullptr);

public slots:
    // Прием данных от Клиента
    void process(constexpr QByteArray &data);

signals:
    // Отправка ответа Клиенту
    void responseReady(constexpr QByteArray &data);

private:
    // Обработка SPI-команд
    void handleSpiRequest(constexpr QByteArray &data);
};

#endif // PARSER_H
