#include "actuator.h"

static const char *TAG = "MASTER_ACTUATOR";
volatile bool limit_switch_pressed = FALSE;


void init_actuator(){
    init_servo();
    setup_limit_switch();
    init_motor_gpio();
}


void init_motor_gpio() {

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << STEP_PIN) | (1ULL << DIR_PIN) |
                        (1ULL << EN_PIN)   | (1ULL << M0_PIN)  |
                        (1ULL << M1_PIN)   | (1ULL << M2_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t ret = gpio_config(&io_conf);
     if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %d", ret);
        return;
    }


    // Activation du driver (EN active LOW)
    gpio_set_level(EN_PIN, 1);

    // Configuration pour 1/8 step
    gpio_set_level(M0_PIN, 1);
    gpio_set_level(M1_PIN, 1);
    gpio_set_level(M2_PIN, 0);

    ESP_LOGI(TAG, "DRV8825 initialisé en 1/8 step, EN activé");
}


void rotate_steps(int steps, bool direction) {
    gpio_set_level(EN_PIN, 0);
    gpio_set_level(DIR_PIN, direction ? 1 : 0 );
    for (int i = 0; i < steps; i++) {
       
        gpio_set_level(STEP_PIN, 1);
        esp_rom_delay_us(10);
        gpio_set_level(STEP_PIN, 0);
        esp_rom_delay_us(STEP_DELAY_US);
         if (check_limit_switch()){
            return;
        }

    }
    gpio_set_level(EN_PIN, 1);
}


void init_servo() {
    // Configuration du timer PWM
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    // Configuration du canal PWM
    ledc_channel_config_t pwm_channel = {
        .gpio_num = SERVO_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 500,
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);
}


uint16_t calculate_duty(uint8_t degree) {

    // Calculer la largeur de l'impulsion en microsecondes
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH + ((SERVO_PULSEWIDTH) * degree) / SERVO_MAX_DEGREE;

    // Calculer le rapport cyclique en fonction de la résolution (16 bits)
    return (pulse_width * (1 << 16)) / SEVOR_PERIOD_US;

}

void set_servo_position(uint8_t degree) {


    // Calculer la largeur de l'impulsion en fonction du degré
    uint16_t duty = calculate_duty(degree);

    // Définir le rapport cyclique
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "Position à %d° \t Duty  = %d", degree, duty);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

}

void setup_limit_switch() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LIMIT_SWITCH_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,  // Interruption sur front montant
    };
    esp_err_t ret = gpio_config(&io_conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure motor GPIOs");
        return;
    }




    // Installation du service d'interruption
    // gpio_install_isr_service(0);

    // Ajout de l'interruption
    // gpio_isr_handler_add(LIMIT_SWITCH_PIN, limit_switch_isr_handler, NULL);
}





bool check_limit_switch() {

    if ((mission_Ctx->current_status < LIMIT_SWITCH_ON) && (gpio_get_level(LIMIT_SWITCH_PIN) == FALSE)) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        transition_to_status(&mission_Ctx->current_status, LIMIT_SWITCH_ON);
        gpio_set_level(EN_PIN, 1);
        ESP_LOGI(TAG, "Limit switch déclenché, arrêt du moteur");
        return TRUE;
    }
    return FALSE;
}
