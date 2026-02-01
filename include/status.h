#if !defined(STATUS_H)
    #define STATUS_H
    #include "main.h"


    typedef enum {
        ASCENT,
        PRE_SEPARATION_WAKE_UP,
        SEPARATION,
        POST_SEPARATION_WAKE_UP,
        MECHANICAL_DEPLOYMENTS,
        SYSTEM_CHECKOUT,
        CHECK_COIL_CONTROL,
        COIL_WINDING,
        LIMIT_SWITCH_ON,
        CHECK_HINGE_CONTROL,
        OPEN_HINGE,
        CHECK_PROPULSION,
        PROPULSION_THRUST,
        COIL_UNWINDING,
        STANDBY,
        INSTRUMENT_CHECKOUT,
        EXPERIENCE_DATA,
        DATA_DOWNLINK,
        SAFE_MODE,
        SURVIVAL,
        END_OF_MISSION
    } SystemStatus;


    const char* get_status_name(SystemStatus status);
    const char* get_status_color(SystemStatus status);
    void transition_to_status(SystemStatus *current_status, SystemStatus new_status);
    void display_current_status(SystemStatus status);

#endif // STATUS_H