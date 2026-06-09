#include "i2c_controler.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <cstring>

bool i2c_init(uint8_t slaveAddr)
{
    //Открываем файл устройства I2C
    i2c_fd = open("/dev/i2c-3", O_RDWR);
    if (i2c_fd < 0)
    {
        std::cerr << "I2C: Ошибка открытия\n";
        return false;
    }

    //Указываем драйверу, с каким адресом на шине мы будем работать (адрес slave-устройства)
    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, slaveAddr) < 0) {
        std::cerr << "I2C: Ошибка установки адреса Slave";
        close(i2c_fd);
        i2c_fd = -1;
        return false;
    }

    current_slave_addr = slaveAddr;

    std::cout << "I2C: Инициализирован\n";
    return true;
}

bool i2c_read_buffer(uint8_t *data, size_t len, uint8_t regAddr)
{
    if (i2c_fd < 0) return false;

    struct i2c_msg msgs[2];
    uint8_t reg = regAddr;

    msgs[0].addr = current_slave_addr;
    msgs[0].flags = 0; // Запись адреса
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = current_slave_addr;
    msgs[1].flags = I2C_M_RD; // Чтение
    msgs[1].len = len;
    msgs[1].buf = data;

    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = 2;

    return (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) >= 0);
}

bool i2c_write_buffer(const uint8_t* data, size_t len, uint8_t startRegAddr)
{
    if (i2c_fd < 0) return false;

    size_t bytes_written = 0;

    while (bytes_written < len)
    {
        uint8_t current_addr = startRegAddr + bytes_written;

        // Готовим буфер: [Адрес регистра] + [Данные страницы]
        uint8_t buffer[PAGE_SIZE + 1];
        buffer[0] = current_addr;
        memcpy(&buffer[1], &data[bytes_written], PAGE_SIZE);

        // Отправляем транзакцию
        if (write(i2c_fd, buffer, PAGE_SIZE + 1) != PAGE_SIZE + 1)
        {
            std::cerr << "I2C: Ошибка записи страницы по адресу " << (int)current_addr << std::endl;
            return false;
        }

        bytes_written += PAGE_SIZE;

        usleep(5000);
    }
    return true;
}

void i2c_deinit()
{
    if (i2c_fd >=0)
    {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

//Чтение с первого регистра EEPROM
// bool i2c_read_buffer(uint8_t *data, size_t len)
// {
//     return i2c_read_buffer(0, data, len);
// }

// //Запись с первого регистра EEPROM
// bool i2c_write_buffer(const uint8_t* data, size_t len)
// {
//     return i2c_write_buffer(0, data, len);
// }

// bool i2c_read(uint8_t slaveAddr, uint8_t regAddr, uint8_t &data)
// {
//     if (i2c_fd < 0) return false;

//     struct i2c_msg msgs[2];
//     uint8_t reg = regAddr;

//     // Первое сообщение: отправить адрес регистра (запись)
//     msgs[0].addr = slaveAddr;          // адрес EEPROM
//     msgs[0].flags = 0;            // 0 = запись
//     msgs[0].len = 1;
//     msgs[0].buf = &reg;

//     // Второе сообщение: прочитать байт данных (чтение)
//     msgs[1].addr = slaveAddr;
//     msgs[1].flags = I2C_M_RD;     // флаг чтения
//     msgs[1].len = 1;
//     msgs[1].buf = &data;

//     struct i2c_rdwr_ioctl_data ioctl_data;
//     ioctl_data.msgs = msgs;
//     ioctl_data.nmsgs = 2;

//     if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
//         std::cerr << "I2C: Ошибка чтения" << std::endl;
//         return false;
//     }
//     std::cout << "I2C: Чтение выполнена успешно" << std::endl;
//     return true;
// }

// // Передавать массив данных или вектор! (две записи сперва слэйв, после адрес)
// // Для отладки i2c-tools
// bool i2c_write(uint8_t slaveAddr, uint8_t regAddr, uint8_t data)
// {
//     if (i2c_fd < 0) return false;

//     uint8_t buffer[2] = {regAddr, data};

//     struct i2c_msg msg;

//     msg.addr  = slaveAddr;
//     msg.flags = 0;
//     msg.len   = sizeof(buffer);
//     msg.buf   = buffer;

//     struct i2c_rdwr_ioctl_data ioctl_data;

//     ioctl_data.msgs  = &msg;
//     ioctl_data.nmsgs = 1;

//     if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0)
//     {
//         std::cerr << "I2C: Ошибка записи" << std::endl;
//         return false;
//     }

//     // EEPROM busy delay
//     usleep(5000);

//     std::cout << "I2C: Запись выполнена успешно" << std::endl;

//     return true;
// }
