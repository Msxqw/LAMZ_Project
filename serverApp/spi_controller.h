#ifndef SPI_CONTROLLER_H
#define SPI_CONTROLLER_H

#include <cstdint>

/*Биты для записи/чтения*/
static constexpr uint8_t SPI_READ_BIT = 0x80;
static constexpr uint8_t SPI_WRITE_BIT = 0x00;

/*Базовый адрес SPI IP*/
static constexpr uint32_t BASE_ADDR_SPIAD = 0x4010000;
// static constexpr uint32_t BASE_ADDR_SPIHMC = 0x40000000;
// static constexpr uint32_t BASE_ADDR_SPILMK = 0x40000000;

/*Смещение регистров*/
static constexpr uint32_t SPI_TX_CMD_RW = 0x00;
static constexpr uint32_t SPI_TX_DATA_RW = 0x04;
static constexpr uint32_t SPI_RX_CMD_RW = 0x08;
static constexpr uint32_t SPI_RX_DATA_RO = 0x0C;

/*Функции взаимодействия Сервер -> SPI IP*/
uint32_t readReg(uint8_t addr);
void writeReg(uint8_t addr, uint32_t data);

/*Функции инициализации/деинициализации*/
bool init();
void deinit();

#endif // SPI_CONTROLLER_H
