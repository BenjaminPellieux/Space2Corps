#include "hinge.h"

static const char *TAG = "ESP32-C6_HINGE";


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


uint32_t calculate_duty(uint32_t angle) {
    if (angle < 0 || angle > SERVO_MAX_DEGREE) {
        return 0;
    }

    // Calculer la largeur de l'impulsion en microsecondes
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH + ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle) / SERVO_MAX_DEGREE;

    // Calculer la période en microsecondes (1/fréquence * 1e6)

    // Calculer le rapport cyclique en fonction de la résolution (16 bits)
    uint32_t duty = (pulse_width * (1 << 16)) / SEVOR_PERIOD_US;

    return duty;
}

void set_servo_position(uint32_t degree) {
    if (degree < 0 || degree > 180) {
        ESP_LOGE(TAG, "Degré hors de la plage valide (0-180)");
        return;
    }

    // Calculer la largeur de l'impulsion en fonction du degré
    uint32_t duty = calculate_duty(degree);

    // Définir le rapport cyclique
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "Position à %d° \n Duty  = %d", degree, duty);
}

// void app_main(void) {
//     // Initialiser le servo
//     init_servo();

//     ESP_LOGI(TAG, "Début du contrôle du servo moteur");

    
//         // Tourner à 0°
//         set_servo_position(0);
//         vTaskDelay(3000 / portTICK_PERIOD_MS);

//         // Tourner à 90°
//         set_servo_position(110);
//         vTaskDelay(3000 / portTICK_PERIOD_MS);

// }