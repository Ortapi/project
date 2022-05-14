#include <stdio.h>
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "ds1307.h"

//Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(uint8_t val)
{
  return val = val/10*16 + val%10 ;
}

// Convert binary coded decimal to normal decimal numbers
uint8_t bcdToDec(uint8_t val)
{
  return val = val/16*10 + val%16; 
}


void i2c_init()
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

uint8_t readDS1307(uint8_t reg_add){
    uint8_t data_buff;
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (DS1307_SENSOR_ADDR << 1 | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(cmd_handle, reg_add, true);
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (DS1307_SENSOR_ADDR << 1 | I2C_MASTER_READ), true);
    i2c_master_read_byte(cmd_handle, &data_buff, I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    return bcdToDec(data_buff);
}



void setupTime(uint8_t hours, uint8_t minute, uint8_t second){
    uint8_t BCDsecond   = decToBcd(second) ;
    uint8_t BCDminute   = decToBcd(minute);
    uint8_t BCDhours    = decToBcd(hours);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (DS1307_SENSOR_ADDR << 1 | I2C_MASTER_WRITE), true);
    uint8_t tx_buff[7] = {0,BCDsecond,BCDminute,BCDhours,0,0,0};
    i2c_master_write(cmd_handle, tx_buff, 7, true);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
}