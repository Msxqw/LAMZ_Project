#ifndef SPI_CONTROLLER_H
#define SPI_CONTROLLER_H

#include <cstdint>

/*Базовые адреса SPI IP микросхем*/
static constexpr uint32_t BASE_ADDR_SPI_AD = 0x43C10000;
static constexpr uint32_t BASE_ADDR_SPI_HMC = 0x43C20000;
static constexpr uint32_t BASE_ADDR_SPI_LMK = 0x43C30000;

/*Смещение регистров*/
static constexpr uint32_t SPI_TX_CMD_RW = 0x00;
static constexpr uint32_t SPI_TX_DATA_RW = 0x04;
static constexpr uint32_t SPI_RX_CMD_RW = 0x08;
static constexpr uint32_t SPI_RX_DATA_RO = 0x0C;

/*Функции взаимодействия Сервер -> SPI IP*/
uint32_t spi_readReg(uint8_t addr);
void spi_writeReg(uint8_t addr, uint32_t data);

/*Функции инициализации/деинициализации*/
bool spi_init();
void spi_deinit();

#endif // SPI_CONTROLLER_H
