/**
 * @file sensors.c
 * @brief Sensor implementation for Space2Corps
 * 
 * This file contains the implementation of IMU (LSM6DSOX) and GPS sensor
 * drivers including initialization, data reading, and processing functions.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#include "main.h"

/**
 * @brief Local logging tag for this module
 */
static const char *TAG = "MASTER_SENSORS";

/**
 * @brief I2C bus handle
 */
static i2c_master_bus_handle_t bus_handle;

/**
 * @brief I2C device handle
 */
static i2c_master_dev_handle_t dev_handle;

/**
 * @brief Detected LSM6DSOX I2C address
 */
static uint8_t lsm6dsox_addr = LSM6DSOX_ADDR_1;

/**
 * @brief UART buffer for GPS data
 */
uint8_t uart_buffer[BUF_SIZE];

/**
 * @brief Global GPS data structure
 * 
 * Contains all GPS position and status information
 */
GPSData gps_data = {0};

/**
 * @brief Read bytes from I2C device register
 * 
 * Internal helper function to read data from LSM6DSOX registers.
 * 
 * @param reg Register address to read from
 * @param data Pointer to buffer for read data
 * @param len Number of bytes to read
 * @return bool TRUE on success, FALSE on failure
 */
static bool i2c_read_bytes(uint8_t reg, uint8_t *data, size_t len) {
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de la lecture I2C: %s", esp_err_to_name(ret));
    }
    
    return ret == ESP_OK;
}

/**
 * @brief Initialize I2C bus
 * 
 * Configures the I2C bus for communication with the LSM6DSOX IMU sensor.
 * Automatically detects the sensor address and initializes the device.
 * 
 * @note Sets mission_Ctx->motion_data.motion_initialized on success.
 */
void init_i2c() {
    // Configure I2C bus
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
        mission_Ctx->motion_data.motion_initialized = false;
        return;
    }

    // Detect sensor I2C address
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
            mission_Ctx->motion_data.motion_initialized = false;
            return;
        }
    }

    // Configure I2C device
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = lsm6dsox_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de l'ajout du périphérique I2C: %s", esp_err_to_name(ret));
        mission_Ctx->motion_data.motion_initialized = false;
        return;
    }

    mission_Ctx->motion_data.motion_initialized = true;
}

/**
 * @brief Scan I2C bus for devices
 * 
 * Verifies communication with the LSM6DSOX sensor by reading the device ID register.
 * 
 * @note Requires I2C bus to be initialized first.
 */
void scan_i2c_bus() {
    if (!mission_Ctx->motion_data.motion_initialized) {
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
        mission_Ctx->motion_data.motion_initialized = false;
        return;
    }
}

/**
 * @brief Write to motion sensor register
 * 
 * Writes data to a specific register on the LSM6DSOX sensor.
 * 
 * @param reg Register address to write to
 * @param data Data byte to write
 * 
 * @note Requires I2C bus to be initialized first.
 */
void write_motion_sensors_register(uint8_t reg, uint8_t data) {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    uint8_t write_buf[2] = {reg, data};
    esp_err_t ret = i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lors de l'écriture I2C: %s", esp_err_to_name(ret));
    }
}

/**
 * @brief Read from motion sensor register
 * 
 * Reads a single byte from a specific register on the LSM6DSOX sensor.
 * 
 * @param reg Register address to read from
 * @return uint8_t Register value, or 0 on error
 * 
 * @note Requires I2C bus to be initialized first.
 */
uint8_t read_motion_sensors_register(uint8_t reg) {
    if (!mission_Ctx->motion_data.motion_initialized) {
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

/**
 * @brief Initialize motion sensors
 * 
 * Configures the LSM6DSOX IMU sensor with appropriate settings for
 * accelerometer and gyroscope operation.
 * 
 * @note Performs a software reset and configures output data rates.
 */
void init_motion_sensors() {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }
    // Perform software reset
    write_motion_sensors_register(CTRL3_C_REG, 0x01);
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for reset to complete

    // Configure accelerometer (ODR=1.66kHz, FS=±2g)
    write_motion_sensors_register(CTRL1_XL_REG, ACCEL_ODR_1660Hz);

    // Configure gyroscope (ODR=1.66kHz, FS=±2000dps)
    write_motion_sensors_register(CTRL2_G_REG, GYRO_ODR_1660Hz);

    // Enable high-pass filter for gyroscope
    write_motion_sensors_register(CTRL7_G_REG, 0x00);

    // Verify configuration registers
    uint8_t ctrl1_xl_val = read_motion_sensors_register(CTRL1_XL_REG);
    uint8_t ctrl2_g_val = read_motion_sensors_register(CTRL2_G_REG);

    ESP_LOGI(TAG, "CTRL1_XL configuré à: 0x%02X", ctrl1_xl_val);
    ESP_LOGI(TAG, "CTRL2_G configuré à: 0x%02X", ctrl2_g_val);
}

/**
 * @brief Read motion sensor data
 * 
 * Reads accelerometer and gyroscope data from the LSM6DSOX sensor and
 * updates the global motion_data structure with converted values.
 * 
 * @note Converts raw sensor data to engineering units (g for acceleration,
 *       degrees per second for rotation).
 */
void read_motion_data() {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    uint8_t data[6];

    // Read acceleration data
    if (!i2c_read_bytes(OUTX_L_XL_REG, data, 6)) {
        ESP_LOGE(TAG, "Erreur de lecture des données d'accélération");
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }

    int16_t accel_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t accel_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t accel_z = (int16_t)((data[5] << 8) | data[4]);

    // Read gyroscope data
    if (!i2c_read_bytes(OUTX_L_G_REG, data, 6)) {
        ESP_LOGE(TAG, "Erreur de lecture des données de gyroscope");
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }
    
    int16_t gyro_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t gyro_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t gyro_z = (int16_t)((data[5] << 8) | data[4]);

    // Update values in motion data structure
    mission_Ctx->motion_data.motionaccel.accel_x = accel_x / 16384.0; // Convert to g (for ±2g range)
    mission_Ctx->motion_data.motionaccel.accel_y = accel_y / 16384.0;
    mission_Ctx->motion_data.motionaccel.accel_z = accel_z / 16384.0;
    mission_Ctx->motion_data.motiongyro.gyro_x = gyro_x / 16.4; // Convert to dps (for ±2000 dps range)
    mission_Ctx->motion_data.motiongyro.gyro_y = gyro_y / 16.4;
    mission_Ctx->motion_data.motiongyro.gyro_z = gyro_z / 16.4;
    
}

/**
 * @brief Display motion sensor data
 * 
 * Outputs formatted accelerometer and gyroscope data to the console.
 * 
 * @note Requires motion data to be initialized and updated.
 */
void display_motion_data() {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    printf("Accel: X=%.2f g Y=%.2f g Z=%.2f g | Gyro: X=%.2f dps Y=%.2f dps Z=%.2f dps\n",
           mission_Ctx->motion_data.motionaccel.accel_x, mission_Ctx->motion_data.motionaccel.accel_y, mission_Ctx->motion_data.motionaccel.accel_z,
           mission_Ctx->motion_data.motiongyro.gyro_x, mission_Ctx->motion_data.motiongyro.gyro_y, mission_Ctx->motion_data.motiongyro.gyro_z);
}

/**
 * @brief Convert NMEA coordinates to decimal degrees
 * 
 * Converts NMEA format coordinates (DDDMM.MMMM) to decimal degrees.
 * 
 * @param nmea_coordinate Coordinate in NMEA format
 * @return float Coordinate in decimal degrees
 */
static float convert_nmea_to_decimal(float nmea_coordinate) {
    int degrees = (int)(nmea_coordinate / 100);
    float minutes = nmea_coordinate - (degrees * 100);
    return degrees + (minutes / 60.0);
}

/**
 * @brief Initialize UART for GPS communication
 * 
 * Configures the UART interface for communication with the GPS module.
 * 
 * @note Uses UART_NUM_1 with 9600 baud rate.
 */
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = GPS_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    // Configure GPIO pins
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, GPS_TX_GPIO, GPS_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART initialisé avec succès");
}

/**
 * @brief Calculate NMEA sentence checksum
 * 
 * Validates the checksum of a NMEA sentence to ensure data integrity.
 * 
 * @param sentence NMEA sentence string
 * @return int 0 if checksum is valid, non-zero if invalid
 */
int sentence_checksum(char* sentence){
    if (sentence[0] != '$') {
        ESP_LOGE(TAG, "Sentence NMEA invalide");
        return 1;
    }
    // Check for checksum delimiter
    char *checksum_pos = strchr(sentence, '*');
    if (checksum_pos == NULL) {
        ESP_LOGE(TAG, "Checksum manquant");
        return 2;
    }

    // Calculate checksum
    uint8_t calculated_checksum = 0;
    for (char *p = sentence + 1; p < checksum_pos; p++) {
        calculated_checksum ^= *p;
    }

    // Compare with received checksum
    uint8_t received_checksum = strtol(checksum_pos + 1, NULL, 16);
    if (calculated_checksum != received_checksum) {
        ESP_LOGE(TAG, "Checksum invalide");
        return 2;
    }
    return 0;

}

/**
 * @brief Parse NMEA sentence
 * 
 * Parses NMEA sentences (GPRMC and GPGGA) and updates the global GPS data structure.
 * 
 * @param sentence NMEA sentence string to parse
 * 
 * @note Handles both GPRMC (position/time) and GPGGA (altitude/satellites) sentences.
 */
// Fonction pour parser une ligne NMEA
void parse_nmea_sentence(char *sentence) {
    // Check if sentence starts with '$'
    ESP_LOGI(TAG, "parse_nmea_sentence %s", sentence);
    
    if (sentence_checksum(sentence)){
        return;
    }
    ESP_LOGI(TAG, "Sentence valide");

    if (strstr(sentence, "GPRMC") != NULL) {
        char *token = strtok(sentence, ",");
        int index = 0;

        while (token != NULL) {
            switch (index) {
                case 1: // UTC Time
                    if (strlen(token) == 6) { // Format HHMMSS
                        strncpy(mission_Ctx->gps_data.time, token, sizeof(mission_Ctx->gps_data.time) - 1);
                    }
                    break;
                case 2: // Status (A=valid, V=invalid)
                    mission_Ctx->gps_data.has_fix = (token[0] == 'A');
                    break;
                case 3: // Latitude (DDDMM.MMMM format)
                    if (strlen(token) > 0) {
                        float lat = atof(token);
                        mission_Ctx->gps_data.latitude = convert_nmea_to_decimal(lat);
                    }
                    break;
                case 4: // Latitude direction (N/S)
                    if (token[0] == 'S') {
                        mission_Ctx->gps_data.latitude = -mission_Ctx->gps_data.latitude;
                    }
                    break;
                case 5: // Longitude (DDDMM.MMMM format)
                    if (strlen(token) > 0) {
                        float lon = atof(token);
                        mission_Ctx->gps_data.longitude = convert_nmea_to_decimal(lon);
                    }
                    break;
                case 6: // Longitude direction (E/W)
                    if (token[0] == 'W') {
                        mission_Ctx->gps_data.longitude = -mission_Ctx->gps_data.longitude;
                    }
                    break;
                case 7: // Speed (knots)
                    mission_Ctx->gps_data.speed = atof(token);
                    break;
                case 9: // Date (DDMMYY format)
                    if (strlen(token) == 6) {
                        strncpy(mission_Ctx->gps_data.date, token, sizeof(mission_Ctx->gps_data.date) - 1);
                    }
                    break;
            }
            token = strtok(NULL, ",");
            index++;
        }
    } else if (strstr(sentence, "GPGGA") != NULL) {
        char *token = strtok(sentence, ",");
        int index = 0;
        ESP_LOGI(TAG, "GPGGA");

        while (token != NULL) {
            switch (index) {
                case 6: // Fix quality
                    mission_Ctx->gps_data.has_fix = (atoi(token) > 0);
                    break;
                case 7: // Number of satellites
                    mission_Ctx->gps_data.satellites = atoi(token);
                    break;
                case 8: // HDOP
                    mission_Ctx->gps_data.hdop = atof(token);
                    break;
                case 9: // Altitude
                    mission_Ctx->gps_data.altitude = atof(token);
                    break;
            }
            token = strtok(NULL, ",");
            index++;
        }
    }
}

/**
 * @brief Read GPS data
 * 
 * Reads data from the GPS module via UART and processes NMEA sentences.
 * 
 * @note Handles buffered data and calls parse_nmea_sentence for each complete sentence.
 */
// Fonction pour lire les données GPS
void read_gps_data() {
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t*)&length));
    ESP_LOGI(TAG, "lenght  %d", length);
    if (length > 0) {
        int read_length = uart_read_bytes(UART_NUM, uart_buffer, length, pdMS_TO_TICKS(100));
        if (read_length > 0) {
            uart_buffer[read_length] = '\0';
            ESP_LOGI(TAG, "Reçu: %s", uart_buffer);

            // Parse received data
            parse_nmea_sentence((char *)uart_buffer);
        }
    } else {
        ESP_LOGD(TAG, "Aucune donnée disponible");
    }
}

/**
 * @brief Display GPS data
 * 
 * Outputs formatted GPS data to the console including position, velocity,
 * and satellite information.
 * 
 * @note Provides warning if no valid GPS fix is available.
 */
// Fonction pour afficher les données GPS
void display_gps_data() {
    printf("\n--- Données GPS ---\n");
    printf("Fix: %s\n", mission_Ctx->gps_data.has_fix ? "Oui" : "Non");
    printf("Satellites: %d\n", mission_Ctx->gps_data.satellites);
    printf("HDOP: %.2f\n", mission_Ctx->gps_data.hdop);
    printf("Latitude: %.6f°\n", mission_Ctx->gps_data.latitude);
    printf("Longitude: %.6f°\n", mission_Ctx->gps_data.longitude);
    printf("Altitude: %.1f m\n", mission_Ctx->gps_data.altitude);
    printf("Vitesse: %.1f knots\n", mission_Ctx->gps_data.speed);
    printf("Heure: %s\n", mission_Ctx->gps_data.time);
    printf("Date: %s\n", mission_Ctx->gps_data.date);

    if (!mission_Ctx->gps_data.has_fix) {
        printf("\n[AVERTISSEMENT] Pas de fix GPS. Vérifiez:\n");
        printf("- L'antenne GPS est correctement connectée\n");
        printf("- Le module a une vue dégagée sur le ciel\n");
        printf("- Attendez quelques minutes pour l'acquisition\n");
    }
}
