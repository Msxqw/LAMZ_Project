#ifndef I2C_CONTROLER_H
#define I2C_CONTROLER_H

#include <cstdint>
inline int i2c_fd = -1;

/*Инициализация шины*/
bool i2c_init(uint8_t slaveAddr);

/*Чтение из промки*/
bool i2c_read(uint8_t regAddr, uint8_t &data);

/*Запись в промку*/
bool i2c_write(uint8_t regAddr, uint8_t data);

/*Деинициализация*/
void i2c_deinit();

#endif // I2C_CONTROLER_H
