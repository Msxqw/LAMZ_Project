#ifndef I2C_CONTROLER_H
#define I2C_CONTROLER_H

#include <cstdint>
#include <iostream>

inline int i2c_fd = -1;
constexpr uint8_t PAGE_SIZE = 16;
static uint8_t current_slave_addr = 0;

/*Инициализация шины*/
bool i2c_init(uint8_t slaveAddr);

/*Чтение из промки*/
bool i2c_read_buffer(uint8_t *data, size_t len, uint8_t regAddr = 0);

/*Запись в промку*/
bool i2c_write_buffer(const uint8_t* data, size_t len, uint8_t startRegAddr = 0);

/*Деинициализация*/
void i2c_deinit();

#endif // I2C_CONTROLER_H
