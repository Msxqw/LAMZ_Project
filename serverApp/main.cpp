#include "tcp_server.h"
#include "parser.h"
#include "spi_controller.h"

#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if(!init())
    {
        std::cerr << "Не удалось инициализировать SPI" << std::endl;
        return 1;
    }

    Parser parser;
    TcpServer server(&parser, 8080);

    int result = app.exec();

    deinit();

    return result;
}
