#include "spi_controller.h"

#include <sys/mman.h>       // Для работы с отображением файлов в память (memory mapping)
#include <fcntl.h>          // Для работы с файловыми дескрипторами и управления файлами
#include <unistd.h>         //
#include <iostream>

int fd = -1;                //Изначально задаем отрицательное значение для проверки открытия
void* map_obj = nullptr;
static uint8_t* regs = nullptr;

bool init()
{
    fd = open("/dev/mem", O_RDWR);
    if (fd < 0)
    {
        std::cerr << "Opening error\n";
        return false;
    }

    map_obj = mmap(nullptr,
                   sysconf(_SC_PAGE_SIZE),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   fd,
                   BASE_ADDR_SPI);

    if (map_obj == MAP_FAILED)
    {
        std::cerr << "mmap failed\n";
        return false;
    }

    regs = reinterpret_cast<uint8_t*>(map_obj);

    std::cout << "SPI initialized\n";
    return true;

}

void deinit()
{
    if (map_obj)
    {
        munmap(map_obj, sysconf(_SC_PAGE_SIZE));
    }
    if (fd > 0)
    {
        close(fd);
    }
}

void writeReg(uint8_t addr, uint32_t data)
{
    // Адрес/команда записываем в TX_CMD
    *(regs + SPI_TX_CMD_RW) = (uint32_t)(addr);

    // Данные записываем в TX_DATA
    *(regs + SPI_TX_DATA_RW) = data;
}

uint32_t readReg(uint8_t addr)
{
    // Команда чтения (бит 7 = 1)
    *(regs + SPI_RX_CMD_RW) = (uint32_t)(SPI_READ_BIT | addr);

    // Небольшая пауза для завершения SPI транзакции
    sleep(1);

    // Чтение результата из RX_DATA
    return *(regs + SPI_RX_DATA_RO);
}

uint32_t readIPCR()
{
    uint32_t value = *(regs + SPI_IP_STNG_RO);

    return value;
}
