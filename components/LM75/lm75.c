#include <stdio.h>
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "lm75.h"


void i2c_lm75_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

float readTemperature()
{
    uint8_t data_buff[2];
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (LM75_ADD << 1 | I2C_MASTER_READ), true);
    i2c_master_read(cmd_handle, &data_buff, 2, I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    bool negTemp = false;
    if (data_buff[0] & 0x80) //check the MSB - if 1 is negative
    {
        negTemp = true;
        data_buff[0] = data_buff[0] & 0x7f; // 2'c... 011111111 on everything
    }


    uint16_t data = (data_buff[0] << 8 | data_buff[1]) >> 5;
    float temperature;
    if (negTemp)
    {
        temperature = (data * 0.125) * -1;
    }
    else
    {
        temperature = (data * 0.125);
    }
    
    return temperature;
}

uint8_t readTemperatureUint8()
{
    uint8_t data_buff[2];
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (LM75_ADD << 1 | I2C_MASTER_READ), true);
    i2c_master_read(cmd_handle, &data_buff, 2, I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    bool negTemp = false;
    if (data_buff[0] & 0x80) //check the MSB - if 1 is negative
    {
        negTemp = true;
        data_buff[0] = data_buff[0] & 0x7f; // 2'c... 011111111 on everything
    }


    uint16_t data = (data_buff[0] << 8 | data_buff[1]) >> 5;
    uint8_t temperature;
    if (negTemp)
    {
        temperature = (data * 0.125) * -1;
    }
    else
    {
        temperature = (data * 0.125);
    }
    
    return temperature;
}