#if !defined(WIFI_H)
    #define WIFI_H
    #include "main.h"


    #define SSID "ESP32-C6_AP"
    #define PASSWORD "password123"
    #define PORT 3333

    
    void wifi_init_softap(void);
    void wifi_task(void *pvParameters);
#endif // WIFI_H