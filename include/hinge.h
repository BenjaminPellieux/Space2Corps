#if !defined(HINGE_H)

    #define HINGE_H
    #include "main.h"
    #include "driver/gpio.h"
    #include "driver/ledc.h"


    // Configuration du PWM pour le servo
    #define SERVO_PIN GPIO_NUM_18       // Broche de signal pour le servo
    #define SERVO_MIN_PULSEWIDTH 500    // Largeur de l'impulsion pour 0° (en microsecondes)
    #define SERVO_MAX_PULSEWIDTH 2500   // Largeur de l'impulsion pour 180° (en microsecondes)
    #define SERVO_FREQ 50               // Fréquence du PWM pour le servo (50 Hz)
    #define SERVO_MAX_DEGREE 180        // Maximum angle in degrees
    #define SEVOR_PERIOD_US 20000          // Période en microsecondes (1/fréquence * 1e6)


        
    void init_servo();
    uint32_t calculate_duty(uint32_t angle);
    void set_servo_position(uint32_t degree);

#endif