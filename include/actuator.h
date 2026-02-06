#if !defined(ACTUATOR_H)

    #define ACTUATOR_H
    #include "main.h"
    #include "driver/gpio.h"
    #include "driver/ledc.h"
    #include "esp_rom_gpio.h"
    #include "esp_task_wdt.h"



    // Configuration du PWM pour le servo
    #define SERVO_PIN GPIO_NUM_18     // Broche de signal pour le servo
    #define SERVO_MIN_PULSEWIDTH 500    // Largeur de l'impulsion pour 0° (en microsecondes)
    #define SERVO_MAX_PULSEWIDTH 2500   // Largeur de l'impulsion pour 180° (en microsecondes)
    #define SERVO_PULSEWIDTH SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH
    
    #define SERVO_FREQ 50               // Fréquence du PWM pour le servo (50 Hz)
    #define SERVO_MAX_DEGREE 180        // Maximum angle in degrees
    #define SEVOR_PERIOD_US 20000       // Période en microsecondes (1/fréquence * 1e6)
    #define LIMIT_SWITCH_PIN GPIO_NUM_19
    #define HINGE_CLOSE 0
    #define HINGE_OPEN 90

    #define EN_PIN      GPIO_NUM_20
    #define DIR_PIN     GPIO_NUM_21
    #define STEP_PIN    GPIO_NUM_22
    #define M0_PIN      GPIO_NUM_10  // Microstepping
    #define M1_PIN      GPIO_NUM_11
    #define M2_PIN      GPIO_NUM_0
  

    // Configuration du moteur pas-à-pas

    // Configuration pour 1/8 step (1600 pas/rev)
    #define STEPS_PER_REVOLUTION 1600
    #define STEP_DELAY_US 4000    // 2 ms entre chaque pas

    #define INITIAL_DELAY 5000 // Délai initiale de 5 secondes


    void init_actuator();


    void init_servo();
    uint16_t calculate_duty(uint8_t degree);
    void set_servo_position(uint8_t degree);
    void init_motor_gpio();
    void setup_limit_switch();
    // void limit_switch_isr_handler(void* arg);

    bool check_limit_switch();

    void rotate_steps(int steps, bool direction);



#endif // ACTUATOR_H