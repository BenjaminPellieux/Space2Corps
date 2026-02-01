#if !defined(ACTUATOR_H)

    #define ACTUATOR_H
    #include "main.h"
    #include "driver/gpio.h"
    #include "driver/ledc.h"


    // Configuration du PWM pour le servo
    #define SERVO_PIN GPIO_NUM_9     // Broche de signal pour le servo
    #define SERVO_MIN_PULSEWIDTH 500    // Largeur de l'impulsion pour 0° (en microsecondes)
    #define SERVO_MAX_PULSEWIDTH 2500   // Largeur de l'impulsion pour 180° (en microsecondes)
    #define SERVO_PULSEWIDTH SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH
    
    #define SERVO_FREQ 50               // Fréquence du PWM pour le servo (50 Hz)
    #define SERVO_MAX_DEGREE 180        // Maximum angle in degrees
    #define SEVOR_PERIOD_US 20000       // Période en microsecondes (1/fréquence * 1e6)
    #define LIMIT_SWITCH_PIN GPIO_NUM_18
    #define HINGE_CLOSE 0
    #define HINGE_OPEN 115


    void init_actuator();


    void init_servo();
    uint16_t calculate_duty(uint8_t degree);
    void set_servo_position(uint8_t degree);

    void setup_limit_switch();
    void check_limit_switch();


#endif // ACTUATOR_H