#include "main.h"

static const char *TAG = "MASTER_SENSORS";


// Initialisation de l'I2C
void init_i2c() {
    i2c_config_t conf = {};

    conf = (i2c_config_t){
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C: %d", ret);
        return;
    }
    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %d", ret);
    }
    ESP_LOGI(TAG, "I2C driver installed %d:  sda: %d scl: %d freq: %d",I2C_MODE_MASTER, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
}

void scan_i2c_bus() {
    ESP_LOGI(TAG,"Scanning I2C bus...\n");
    for (uint8_t i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, i << 1 | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        ESP_LOGI(TAG, "SCAN %d", i); 

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Device found at address 0x%02X\n", i);
        
        }
    }
    
}


// Fonction pour lire un registre de l'ICM-20948
void write_motion_sensors_register(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU9255_ADDRESS << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_write_byte(cmd, data, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to Motion sensors register 0x%02X: %d", reg, ret);
    }
    ESP_LOGI(TAG, "Write %d in motion senors at reg: %d", data, reg);
}

uint8_t read_motion_sensors_register(uint8_t reg) {
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU9255_ADDRESS << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU9255_ADDRESS << 1 | I2C_MASTER_READ, 1);
    i2c_master_read_byte(cmd, &data, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from Motion sensors register 0x%02X: %d", reg, ret);
        return 0;
    }
    ESP_LOGI(TAG, "Read %d in motion sensors at reg: %d", data, reg);
    return data;
}

// Initialisation de l'ICM-20948
void init_motion_sensors() {
    // Vérifier la connexion avec le MPU-9255
    uint8_t who_am_i = read_motion_sensors_register(0x75); // WHO_AM_I register
    if (who_am_i != 0x73) {
        ESP_LOGE(TAG, "MPU-9255 not detected (WHO_AM_I = 0x%02X)", who_am_i);
        mission_Ctx->motion_data.motion_initialized = false;
        return;
    }

    // Réveiller le MPU-9255
    write_motion_sensors_register(0x6B, 0x00); // PWR_MGMT_1 register

    // Configurer le gyroscope
    write_motion_sensors_register(0x1B, 0x00); // GYRO_CONFIG register, ±250 dps

    // Configurer l'accéléromètre
    write_motion_sensors_register(0x1C, 0x00); // ACCEL_CONFIG register, ±2g

    // Configurer la fréquence d'échantillonnage
    write_motion_sensors_register(0x19, 0x07); // SMPLRT_DIV register, 1 kHz

    mission_Ctx->motion_data.motion_initialized = true;
    ESP_LOGI(TAG, "MPU-9255 initialized successfully");
}

void read_motion_data() {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "IMU not initialized");
        return;
    }

    uint8_t data[14];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU9255_ADDRESS << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, 0x3B, 1); // ACCEL_XOUT_H register
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU9255_ADDRESS << 1 | I2C_MASTER_READ, 1);
    i2c_master_read(cmd, data, 14, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read IMU data: %d", ret);
        return;
    }

    // Lire les données de l'accéléromètre
    mission_Ctx->motion_data.accel_x = (int16_t)((data[0] << 8) | data[1]) / 16384.0;
    mission_Ctx->motion_data.accel_y = (int16_t)((data[2] << 8) | data[3]) / 16384.0;
    mission_Ctx->motion_data.accel_z = (int16_t)((data[4] << 8) | data[5]) / 16384.0;

    // Lire les données du gyroscope
    mission_Ctx->motion_data.gyro_x = (int16_t)((data[8] << 8) | data[9]) / 131.0;
    mission_Ctx->motion_data.gyro_y = (int16_t)((data[10] << 8) | data[11]) / 131.0;
    mission_Ctx->motion_data.gyro_z = (int16_t)((data[12] << 8) | data[13]) / 131.0;

    // Lire la température
    int16_t temp_raw = (int16_t)((data[6] << 8) | data[7]);
    mission_Ctx->motion_data.temp = (temp_raw / 340.0) + 36.53;
}

void display_motion_data() {
    read_motion_data();
    ESP_LOGI(TAG, "IMU Data - Accel: (%.2fg, %.2fg, %.2fg), Gyro: (%.2f°, %.2f°, %.2f°)",
            mission_Ctx->motion_data.accel_x,
            mission_Ctx->motion_data.accel_y,
            mission_Ctx->motion_data.accel_z,
            mission_Ctx->motion_data.gyro_x,
            mission_Ctx->motion_data.gyro_y,
            mission_Ctx->motion_data.gyro_z);
}