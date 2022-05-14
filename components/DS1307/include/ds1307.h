#ifndef _DS1307_H
#define _DS1307_H

#define I2C_MASTER_SCL_IO       25      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO       26      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_FREQ_HZ      100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS   1000
#define DS1307_SENSOR_ADDR      0x68        /*!< Slave address of the MPU9250 sensor */



uint8_t decToBcd(uint8_t val);
uint8_t bcdToDec(uint8_t val);
void i2c_init();
uint8_t readDS1307(uint8_t reg_add);
void setupTime(uint8_t hours, uint8_t minute, uint8_t second);


#endif