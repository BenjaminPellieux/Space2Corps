#include "main.h"

static const char *TAG = "MASTER_SENSORS";


static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;
static uint8_t lsm6dsox_addr = LSM6DSOX_ADDR_1;
uint8_t uart_buffer[BUF_SIZE];


//MotionData motion_data = {0};
GPSData gps_data = {0};


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
        mission_Ctx->motion_data.motion_initialized = false;
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
            mission_Ctx->motion_data.motion_initialized = false;
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
        mission_Ctx->motion_data.motion_initialized = false;
        return;
    }

    mission_Ctx->motion_data.motion_initialized = true;
}



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

void init_motion_sensors() {
    if (!mission_Ctx->motion_data.motion_initialized) {
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
    if (!mission_Ctx->motion_data.motion_initialized) {
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
    mission_Ctx->motion_data.accel_x = accel_x / 16384.0; // Conversion en g (pour une plage de ±2g)
    mission_Ctx->motion_data.accel_y = accel_y / 16384.0;
    mission_Ctx->motion_data.accel_z = accel_z / 16384.0;
    mission_Ctx->motion_data.gyro_x = gyro_x / 16.4; // Conversion en dps (pour une plage de ±2000 dps)
    mission_Ctx->motion_data.gyro_y = gyro_y / 16.4;
    mission_Ctx->motion_data.gyro_z = gyro_z / 16.4;
    
}

void display_motion_data() {
    if (!mission_Ctx->motion_data.motion_initialized) {
        ESP_LOGE(TAG, "Bus I2C non initialisé");
        return;
    }

    printf("Accel: X=%.2f g Y=%.2f g Z=%.2f g | Gyro: X=%.2f dps Y=%.2f dps Z=%.2f dps\n",
           mission_Ctx->motion_data.accel_x, mission_Ctx->motion_data.accel_y, mission_Ctx->motion_data.accel_z,
           mission_Ctx->motion_data.gyro_x, mission_Ctx->motion_data.gyro_y, mission_Ctx->motion_data.gyro_z);
}


static float convert_nmea_to_decimal(float nmea_coordinate) {
    int degrees = (int)(nmea_coordinate / 100);
    float minutes = nmea_coordinate - (degrees * 100);
    return degrees + (minutes / 60.0);
}

void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = GPS_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Installer le driver UART
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));

    // Configurer les broches UART
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    // Configurer les broches GPIO
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, GPS_TX_GPIO, GPS_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART initialisé avec succès");
}


int sentence_checksum(char* sentence){
    if (sentence[0] != '$') {
        ESP_LOGE(TAG, "Sentence NMEA invalide");
        return 1;
    }
    // Vérifier le checksum
    char *checksum_pos = strchr(sentence, '*');
    if (checksum_pos == NULL) {
        ESP_LOGE(TAG, "Checksum manquant");
        return 2;
    }

    // Calculer le checksum
    uint8_t calculated_checksum = 0;
    for (char *p = sentence + 1; p < checksum_pos; p++) {
        calculated_checksum ^= *p;
    }

    // Comparer avec le checksum reçu
    uint8_t received_checksum = strtol(checksum_pos + 1, NULL, 16);
    if (calculated_checksum != received_checksum) {
        ESP_LOGE(TAG, "Checksum invalide");
        return 2;
    }
    return 0;

}


// Fonction pour parser une ligne NMEA
void parse_nmea_sentence(char *sentence) {
    // Vérifier si la sentence commence par '$'
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
                case 1: // Heure UTC
                    if (strlen(token) == 6) { // Format HHMMSS
                        strncpy(mission_Ctx->gps_data.time, token, sizeof(mission_Ctx->gps_data.time) - 1);
                    }
                    break;
                case 2: // Statut (A=valide, V=invalide)
                    mission_Ctx->gps_data.has_fix = (token[0] == 'A');
                    break;
                case 3: // Latitude (format DDDMM.MMMM)
                    if (strlen(token) > 0) {
                        float lat = atof(token);
                        mission_Ctx->gps_data.latitude = convert_nmea_to_decimal(lat);
                    }
                    break;
                case 4: // Indicateurs de latitude (N/S)
                    if (token[0] == 'S') {
                        mission_Ctx->gps_data.latitude = -mission_Ctx->gps_data.latitude;
                    }
                    break;
                case 5: // Longitude (format DDDMM.MMMM)
                    if (strlen(token) > 0) {
                        float lon = atof(token);
                        mission_Ctx->gps_data.longitude = convert_nmea_to_decimal(lon);
                    }
                    break;
                case 6: // Indicateurs de longitude (E/W)
                    if (token[0] == 'W') {
                        mission_Ctx->gps_data.longitude = -mission_Ctx->gps_data.longitude;
                    }
                    break;
                case 7: // Vitesse (noeuds)
                    mission_Ctx->gps_data.speed = atof(token);
                    break;
                case 9: // Date (format DDMMYY)
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
                case 6: // Qualité du fix
                    mission_Ctx->gps_data.has_fix = (atoi(token) > 0);
                    break;
                case 7: // Nombre de satellites
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

            // Diviser les données en lignes
            parse_nmea_sentence((char *)uart_buffer);
        }
    } else {
        ESP_LOGD(TAG, "Aucune donnée disponible");
    }
}

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
