#if !defined(SENSORS_H)

    #define SENSORS_H
    #include "main.h"
    #include "driver/i2c.h"

    #define I2C_MASTER_SCL_IO           GPIO_NUM_7  // Exemple de broches I2C
    #define I2C_MASTER_SDA_IO           GPIO_NUM_6
    #define I2C_MASTER_FREQ_HZ          100000
    #define MPU9255_ADDRESS            0x68  // Adresse I2C par défaut


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

    void init_i2c();
    void scan_i2c_bus();
    void write_motion_sensors_register(uint8_t reg, uint8_t data);
    uint8_t read_motion_sensors_register(uint8_t reg);
    void init_motion_sensors();
    void read_motion_data();
    void display_motion_data();



#endif // SENSORS_H