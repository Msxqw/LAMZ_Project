#include "i2c_controler.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

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

    std::cout << "I2C: Инициализирован\n";
    return true;
}

bool i2c_read(uint8_t slaveAddr, uint8_t regAddr, uint8_t &data)
{
    if (i2c_fd < 0) return false;

    struct i2c_msg msgs[2];
    uint8_t reg = regAddr;

    // Первое сообщение: отправить адрес регистра (запись)
    msgs[0].addr = slaveAddr;          // адрес EEPROM
    msgs[0].flags = 0;            // 0 = запись
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    // Второе сообщение: прочитать байт данных (чтение)
    msgs[1].addr = slaveAddr;
    msgs[1].flags = I2C_M_RD;     // флаг чтения
    msgs[1].len = 1;
    msgs[1].buf = &data;

    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
        std::cerr << "I2C: Ошибка чтения" << std::endl;
        return false;
    }
    std::cout << "I2C: Чтение выполнена успешно" << std::endl;
    return true;
}

// Передавать массив данных или вектор! (две записи сперва слэйв, после адрес)
// Для отладки i2c-tools
bool i2c_write(uint8_t slaveAddr, uint8_t regAddr, uint8_t data)
{
    if (i2c_fd < 0) return false;

    uint8_t buffer[2] = {regAddr, data};

    struct i2c_msg msg;

    msg.addr  = slaveAddr;
    msg.flags = 0;
    msg.len   = sizeof(buffer);
    msg.buf   = buffer;

    struct i2c_rdwr_ioctl_data ioctl_data;

    ioctl_data.msgs  = &msg;
    ioctl_data.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0)
    {
        std::cerr << "I2C: Ошибка записи" << std::endl;
        return false;
    }

    // EEPROM busy delay
    usleep(5000);

    std::cout << "I2C: Запись выполнена успешно" << std::endl;

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
