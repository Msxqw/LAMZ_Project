#include "i2c_controler.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int i2c_fd = -1;

bool i2c_init(uint8_t slaveAddr)
{
    //Открываем файл устройства I2C
    i2c_fd = open("/dev/i2c-0", O_RDWR);
    if (i2c_fd < 0)
    {
        std::cerr << "I2C: Ошибка открытия\n";
        return false;
    }

    //Указываем драйверу, с каким адресом на шине мы будем работать (адрес slave-устройства)
    if (ioctl(i2c_fd, I2C_SLAVE, slaveAddr) < 0) {
        std::cerr << "I2C: Ошибка установки адреса Slave";
        close(i2c_fd);
        i2c_fd = -1;
        return false;
    }

    std::cout << "I2C: Инициализирован\n";
    return true;
}

bool i2c_read(uint8_t regAddr, uint8_t &data)
{
    if (i2c_fd < 0) return false;

    if (write(i2c_fd, &regAddr, 1) != 1)
    {
        std::cerr << "I2C: Ошибка выбора регистра";
        return false;
    }

    if (read(i2c_fd, &data, 1) != 1)
    {
        std::cerr << "I2C: Ошибка чтения данных";
        return false;
    }

    std::cout << "I2C: Операция выполнена успешно";
    return true;
}

bool i2c_write(uint8_t regAddr, uint8_t data)
{
    if (i2c_fd < 0) return false;

    uint8_t buffer[2] = {regAddr, data};

    if (write(i2c_fd, buffer, 2) != 2)
    {
        std::cerr << "I2C: Ошибка записи данных";
        return false;
    }

    std::cout << "I2C: Операция выполнена успешно";
    return true;
}

void i2c_deinit()
{
    if (i2c_fd >=0) close(i2c_fd);
}
