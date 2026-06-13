#include "spi_controller.h"
#include "protocol.h"

#include <sys/mman.h>       // Для работы с отображением файлов в память (memory mapping)
#include <fcntl.h>          // Для работы с файловыми дескрипторами и управления файлами
#include <unistd.h>
#include <iostream>

int spi_fd = -1;

static void* regs_ad = nullptr;
static void* regs_hmc = nullptr;
static void* regs_lmk = nullptr;

static uint8_t* regsIP_ad = nullptr;
static uint8_t* regsIP_hmc = nullptr;
static uint8_t* regsIP_lmk = nullptr;

static uint8_t* regs = nullptr;

bool spi_init()
{
    spi_fd = open("/dev/mem", O_RDWR);
    if (spi_fd < 0)
    {
        std::cerr << "SPI: Ошибка открытия\n";
        return false;
    }

    regs_ad = mmap(nullptr,
                   sysconf(_SC_PAGE_SIZE),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   spi_fd,
                   BASE_ADDR_SPI_AD);

    regs_hmc = mmap(nullptr,
                   sysconf(_SC_PAGE_SIZE),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   spi_fd,
                   BASE_ADDR_SPI_HMC);

    regs_lmk = mmap(nullptr,
                   sysconf(_SC_PAGE_SIZE),
                   PROT_WRITE,
                   MAP_SHARED,
                   spi_fd,
                   BASE_ADDR_SPI_LMK);

    if (regs_ad == MAP_FAILED || regs_hmc == MAP_FAILED || regs_lmk == MAP_FAILED)
    {
        std::cerr << "SPI: mmap failed\n";
        return false;
    }

    regsIP_ad = reinterpret_cast<uint8_t*>(regs_ad);
    regsIP_hmc = reinterpret_cast<uint8_t*>(regs_hmc);
    regsIP_lmk = reinterpret_cast<uint8_t*>(regs_lmk);

    std::cout << "SPI: Инициализирован\n";
    return true;

}

void spi_writeReg(uint8_t slave_id, uint8_t addr, uint32_t data)
{
    switch (slave_id) {
    //Логика AD9122
    case 1:
        regs = regsIP_ad;
        // Адрес/команда записываем в TX_CMD
        *(regs + SPI_TX_CMD_RW) = (uint32_t)(addr);

        // Данные записываем в TX_DATA
        *(regs + SPI_TX_DATA_RW) = data;
        break;
    //Логика HMC1035
    case 2:
        regs = regsIP_hmc;
        // Адрес/команда записываем в TX_CMD (7 бит)
        *(regs + SPI_TX_CMD_RW) = (uint32_t)(addr & 0x7F);

        // Данные записываем в TX_DATA (24 << 1 бит)
        *(regs + SPI_TX_DATA_RW) = (data << 1);
        break;
    //Логика LMK01000
    case 3:
        regs = regsIP_lmk;
        // Данные записываем в TX_CMD (28 бит)
        *(regs + SPI_TX_CMD_RW) = (data & 0x0FFFFFFF);

        // Адрес/команда записываем в TX_DATA (4 бита)
        *(regs + SPI_TX_DATA_RW) = (addr & 0x0F);
        break;
    default:
        break;
    }
}

uint32_t spi_readReg(uint8_t addr)
{
    // Адрес записываем в RX_CMD
    *(regsIP_ad + SPI_RX_CMD_RW) = (uint32_t)(addr);

    // Небольшая пауза для завершения SPI транзакции
    sleep(1);

    // Чтение результата из RX_DATA
    return *(regsIP_ad + SPI_RX_DATA_RO);
}

void spi_deinit()
{
    if (regs_ad) munmap(regs_ad, sysconf(_SC_PAGE_SIZE));
    if (regs_hmc) munmap(regs_hmc, sysconf(_SC_PAGE_SIZE));
    if (regs_lmk) munmap(regs_lmk, sysconf(_SC_PAGE_SIZE));

    if (spi_fd > 0)
    {
        spi_fd = -1;
        close(spi_fd);
    }
}
    // if (map_obj)
    // {
    //     munmap(map_obj, sysconf(_SC_PAGE_SIZE));
    // }

// uint32_t readIPCR()
// {
//     uint32_t value = *(regs + SPI_IP_STNG_RO);
//     return value;
// }
