#if !defined(MAIN_H)

    #define MAIN_H

    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <string.h>

    #include <inttypes.h>
    #include "sdkconfig.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_chip_info.h"
    #include "esp_flash.h"
    #include "esp_system.h"

    #include "esp_wifi.h"
    #include "esp_mac.h"
    #include "esp_event.h"
    #include "esp_log.h"
    #include "nvs_flash.h"
    #include "esp_netif.h"

    #include "lwip/sockets.h"
    #include "lwip/netdb.h"
    #include "lwip/err.h"
    #include "lwip/sys.h"


    #include "actuator.h"
    #include "wifi.h"
    #include "status.h"

    // #include <EEPROM.h>

    #define RESET   "\033[0m"
    #define RED     "\033[31m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define GREEN   "\033[32m"
 

    #define TRUE 1
    #define FALSE 0
    #define MAX_SIZE_NAME 64


    typedef struct Mission_Ctx{
        char mission_name[MAX_SIZE_NAME];
        SystemStatus current_status;
    } Mission_Ctx;



    extern Mission_Ctx* mission_Ctx;  

    void app_main();
    void control_task(void *pvParameters);
    void init_context();
    int handle_misson_status(SystemStatus status);


#endif