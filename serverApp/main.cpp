#include "tcp_server.h"
#include "parser.h"
#include "spi_controller.h"

#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TcpServer server;

    Parser parser;

    QObject::connect(&server, &TcpServer::dataReceived, &parser, &Parser::process);

    QObject::connect(&parser, &Parser::responseReady, &server, &TcpServer::sendData);

    init();

    deinit();

    return app.exec()
}
