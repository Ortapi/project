#ifndef _LM75_H
#define _LM75_H

#define I2C_MASTER_SDA_IO   16
#define I2C_MASTER_SCL_IO   17
#define I2C_MASTER_FREQ_HZ  100000
#define LM75_ADD            0x48

void i2c_lm75_init();

float readTemperature();
uint8_t readTemperatureUint8();

#endif