/**
 * @file sensors.h
 * @brief Sensor interface module for Space2Corps
 * 
 * This module provides interfaces for IMU (LSM6DSOX) and GPS sensors,
 * including initialization, data reading, and processing.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"

/**
 * @brief I2C configuration
 */
//#define SENSOR_TAG "LSM6DSOX"
#define I2C_MASTER_SCL_IO GPIO_NUM_7  /**< GPIO7 for SCL */
#define I2C_MASTER_SDA_IO GPIO_NUM_6  /**< GPIO6 for SDA */
#define I2C_MASTER_FREQ_HZ 100000     /**< 100 kHz */

/**
 * @brief LSM6DSOX IMU addresses and registers
 */
// Adresses possibles pour le LSM6DSOX
#define LSM6DSOX_ADDR_1 0x6A        /**< Primary I2C address */
#define LSM6DSOX_ADDR_2 0x6B        /**< Secondary I2C address */

// Registres du LSM6DSOX
#define WHO_AM_I_REG     0x0F      /**< Device ID register */
#define CTRL1_XL_REG     0x10      /**< Accelerometer control */
#define CTRL2_G_REG      0x11      /**< Gyroscope control */
#define CTRL3_C_REG      0x12      /**< Configuration control */
#define CTRL7_G_REG      0x16      /**< Gyroscope configuration */
#define OUTX_L_G_REG     0x22      /**< Gyroscope X output */
#define OUTX_L_XL_REG    0x28      /**< Accelerometer X output */

/**
 * @brief Sensor configuration values
 */
// Configuration pour l'accéléromètre et le gyroscope
#define ACCEL_ODR_1660Hz 0x60     /**< 1660Hz output data rate */
#define GYRO_ODR_1660Hz  0x60     /**< 1660Hz output data rate */

/**
 * @brief UART configuration for GPS
 */
// Configuration UART
#define UART_NUM UART_NUM_1       /**< UART port number */
#define BUF_SIZE 4096             /**< Buffer size */
#define GPS_TX_GPIO GPIO_NUM_15   /**< GPIO15 for TX */
#define GPS_RX_GPIO GPIO_NUM_23   /**< GPIO23 for RX */
#define GPS_BAUD 9600             /**< GPS baud rate */


typedef struct MotionGyro{
    float gyro_x;                  /**< X-axis rotation (dps) */
    float gyro_y;                  /**< Y-axis rotation (dps) */
    float gyro_z;                  /**< Z-axis rotation (dps) */
} MotionGyro;

typedef struct MotionAccel{
    float accel_x;                 /**< X-axis acceleration (g) */
    float accel_y;                 /**< Y-axis acceleration (g) */
    float accel_z;                 /**< Z-axis acceleration (g) */
} MotionAccel;
/**
 * @brief Motion sensor data structure
 * 
 * Contains accelerometer, gyroscope, and temperature data
 */
typedef struct MotionData {
    bool motion_initialized;       /**< Initialization flag */
    MotionGyro motiongyro;
    MotionAccel motionaccel;
    float temp;                    /**< Temperature (°C) */
} MotionData;

/**
 * @brief GPS data structure
 * 
 * Contains position, velocity, and time information
 */
typedef struct {
    float latitude;                /**< Latitude in decimal degrees */
    float longitude;               /**< Longitude in decimal degrees */
    float altitude;                /**< Altitude in meters */
    float speed;                   /**< Speed in knots */
    float hdop;                    /**< Horizontal dilution of precision */
    int satellites;                /**< Number of satellites */
    char time[15];                 /**< UTC time string */
    char date[15];                 /**< Date string */
    bool has_fix;                  /**< Valid fix flag */

} GPSData;

/**
 * @brief Global sensor data variables
 */
extern MotionData motion_data;
extern GPSData gps_data;

/**
 * @brief Initialize UART for GPS
 * 
 * Configures UART interface for GPS communication
 */
void init_uart();

/**
 * @brief Initialize I2C bus
 * 
 * Configures I2C interface for IMU communication
 */
void init_i2c();

/**
 * @brief Scan I2C bus for devices
 * 
 * Detects connected I2C devices
 */
void scan_i2c_bus();

/**
 * @brief Write to IMU register
 * 
 * @param reg Register address
 * @param data Data to write
 */
void write_motion_sensors_register(uint8_t reg, uint8_t data);

/**
 * @brief Read from IMU register
 * 
 * @param reg Register address
 * @return uint8_t Register value
 */
uint8_t read_motion_sensors_register(uint8_t reg);

/**
 * @brief Initialize motion sensors
 * 
 * Configures LSM6DSOX IMU
 */
void init_motion_sensors();

/**
 * @brief Read motion sensor data
 * 
 * Updates global motion_data structure
 */
void read_motion_data();

/**
 * @brief Display motion sensor data
 * 
 * Outputs formatted sensor data to console
 */
void display_motion_data();

/**
 * @brief Calculate NMEA sentence checksum
 * 
 * @param sentence NMEA sentence string
 * @return int 0 if valid, non-zero if invalid
 */
int sentence_checksum(char* sentence);

/**
 * @brief Parse NMEA sentence
 * 
 * @param sentence NMEA sentence to parse
 */
void parse_nmea_sentence(char *sentence);

/**
 * @brief Read GPS data
 * 
 * Reads and processes GPS data from UART
 */
void read_gps_data();

/**
 * @brief Display GPS data
 * 
 * Outputs formatted GPS data to console
 */
void display_gps_data();

#endif // SENSORS_H
