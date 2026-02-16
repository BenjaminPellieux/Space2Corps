#include "main.h"

static const char *TAG = "MASTER_SENSORS";


static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;
static uint8_t lsm6dsox_addr = LSM6DSOX_ADDR_1;

motion_data = {0};

static bool i2c_read_bytes(uint8_t reg, uint8_t *data, size_t len) {
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de la lecture I2C: %s", esp_err_to_name(ret));
    }
    
    return ret == ESP_OK;
}



void init_i2c() {
    // Configuration du bus I2C
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de l'initialisation du bus I2C: %s", esp_err_to_name(ret));
        motion_data.motion_initialized = false;
        return;
    }

    // Détection de l'adresse I2C du capteur
    ret = i2c_master_probe(bus_handle, LSM6DSOX_ADDR_1, -1);
    if (ret == ESP_OK) {
        lsm6dsox_addr = LSM6DSOX_ADDR_1;
        ESP_LOGI(TAG, "Capteur détecté à l'adresse 0x6A");
    } else {
        ret = i2c_master_probe(bus_handle, LSM6DSOX_ADDR_2, -1);
        if (ret == ESP_OK) {
            lsm6dsox_addr = LSM6DSOX_ADDR_2;
            ESP_LOGI(TAG, "Capteur détecté à l'adresse 0x6B");
        } else {
            ESP_LOGE(TAG, "Aucun capteur LSM6DSOX détecté sur 0x6A ou 0x6B");
            motion_data.motion_initialized = false;
            return;
        }
    }

    // Configuration du périphérique I2C
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = lsm6dsox_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de l'ajout du périphérique I2C: %s", esp_err_to_name(ret));
        motion_data.motion_initialized = false;
        return;
    }

    motion_data.motion_initialized = true;
}



void scan_i2c_bus() {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }
    uint8_t who_am_i;
    if (!i2c_read_bytes(WHO_AM_I_REG, &who_am_i, 1)) {
        ESP_LOGE(TAG, "Erreur de lecture I2C lors de la vérification de l'ID");
        return;
    }

    ESP_LOGI(TAG, "LSM6DSOX ID: 0x%02X", who_am_i);
    if (who_am_i != 0x6C) {
        ESP_LOGE(TAG, "Erreur: Capteur non détecté ou ID incorrect");
        motion_data.motion_initialized = false;
        return;
    }
}

void write_motion_sensors_register(uint8_t reg, uint8_t data) {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    uint8_t write_buf[2] = {reg, data};
    esp_err_t ret = i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de l'écriture I2C: %s", esp_err_to_name(ret));
    }
}

uint8_t read_motion_sensors_register(uint8_t reg) {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return 0;
    }

    uint8_t data;
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg, 1, &data, 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de la lecture I2C: %s", esp_err_to_name(ret));
        return 0;
    }
    return data;
}

void init_motion_sensors() {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }
    // Effectuer un reset logiciel du capteur
    write_motion_sensors_register(CTRL3_C_REG, 0x01);
    vTaskDelay(pdMS_TO_TICKS(100)); // Attendre que le reset soit effectué

    // Configuration de l'accéléromètre (ODR=1.66kHz, FS=±2g)
    write_motion_sensors_register(CTRL1_XL_REG, ACCEL_ODR_1660Hz);

    // Configuration du gyroscope (ODR=1.66kHz, FS=±2000dps)
    write_motion_sensors_register(CTRL2_G_REG, GYRO_ODR_1660Hz);

    // Activer le filtre passe-haut pour le gyroscope
    write_motion_sensors_register(CTRL7_G_REG, 0x00);

    // Vérification des registres de configuration
    uint8_t ctrl1_xl_val = read_motion_sensors_register(CTRL1_XL_REG);
    uint8_t ctrl2_g_val = read_motion_sensors_register(CTRL2_G_REG);

    ESP_LOGI(TAG, "CTRL1_XL configuré à: 0x%02X", ctrl1_xl_val);
    ESP_LOGI(TAG, "CTRL2_G configuré à: 0x%02X", ctrl2_g_val);
}

void read_motion_data() {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    uint8_t data[6];

    // Lecture des données d'accélération
    if (!i2c_read_bytes(OUTX_L_XL_REG, data, 6)) {
        ESP_LOGE(TAG, "Erreur de lecture des données d'accélération");
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }

    int16_t accel_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t accel_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t accel_z = (int16_t)((data[5] << 8) | data[4]);

    // Lecture des données de gyroscope
    if (!i2c_read_bytes(OUTX_L_G_REG, data, 6)) {
        ESP_LOGE(TAG, "Erreur de lecture des données de gyroscope");
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }
   
    int16_t gyro_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t gyro_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t gyro_z = (int16_t)((data[5] << 8) | data[4]);

    // Mise à jour des valeurs dans la structure
    motion_data.accel_x = accel_x / 16384.0; // Conversion en g (pour une plage de ±2g)
    motion_data.accel_y = accel_y / 16384.0;
    motion_data.accel_z = accel_z / 16384.0;
    motion_data.gyro_x = gyro_x / 16.4; // Conversion en dps (pour une plage de ±2000 dps)
    motion_data.gyro_y = gyro_y / 16.4;
    motion_data.gyro_z = gyro_z / 16.4;
    
}

void display_motion_data() {
    if (!motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    printf("Accel: X=%.2f g Y=%.2f g Z=%.2f g | Gyro: X=%.2f dps Y=%.2f dps Z=%.2f dps\n",
           motion_data.accel_x, motion_data.accel_y, motion_data.accel_z,
           motion_data.gyro_x, motion_data.gyro_y, motion_data.gyro_z);
}