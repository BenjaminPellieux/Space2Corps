#if !defined(SENSORS_H)

    #define SENSORS_H
    #include "main.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/i2c_master.h"
    #include "esp_log.h"
    #include "esp_err.h"

    #define TAG "LSM6DSOX"
    #define I2C_MASTER_SCL_IO GPIO_NUM_7  // GPIO7 pour SCL
    #define I2C_MASTER_SDA_IO GPIO_NUM_6  // GPIO6 pour SDA
    #define I2C_MASTER_FREQ_HZ 100000     // 100 kHz

    // Adresses possibles pour le LSM6DSOX
    #define LSM6DSOX_ADDR_1 0x6A
    #define LSM6DSOX_ADDR_2 0x6B

    // Registres du LSM6DSOX
    #define WHO_AM_I_REG     0x0F
    #define CTRL1_XL_REG     0x10
    #define CTRL2_G_REG      0x11
    #define CTRL3_C_REG      0x12
    #define CTRL7_G_REG      0x16
    #define OUTX_L_G_REG     0x22
    #define OUTX_L_XL_REG    0x28

    // Configuration pour l'accéléromètre et le gyroscope
    #define ACCEL_ODR_1660Hz 0x60
    #define GYRO_ODR_1660Hz  0x60


    typedef struct MotionData {
        bool motion_initialized;
        float accel_x;
        float accel_y;
        float accel_z;
        float gyro_x;
        float gyro_y;
        float gyro_z;
        float temp;
    } MotionData;


    extern MotionData motion_data;
    void init_i2c();
    void scan_i2c_bus();
    void write_motion_sensors_register(uint8_t reg, uint8_t data);
    uint8_t read_motion_sensors_register(uint8_t reg);
    void init_motion_sensors();
    void read_motion_data();
    void display_motion_data();



#endif // SENSORS_H