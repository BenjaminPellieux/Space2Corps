#include "main.h"

static const char *TAG = "MASTER_STATUS";


const char* get_status_name(SystemStatus status) {
    switch(status) {
        case ASCENT: return "Ascent";
        case PRE_SEPARATION_WAKE_UP: return "Pre-Separation Wake-Up";
        case SEPARATION: return "Separation";
        case POST_SEPARATION_WAKE_UP: return "Post-Separation Wake-Up";
        case MECHANICAL_DEPLOYMENTS: return "Mechanical Deployments";
        case SYSTEM_CHECKOUT: return "System Checkout";
        case CHECK_COIL_CONTROL: return "Check Coil Control";
        case COIL_WINDING: return "Coil Winding";
        case LIMIT_SWITCH_ON: return "Limit Switch On";
        case CHECK_HINGE_CONTROL: return "Check Hinge Control";
        case OPEN_HINGE: return "Open Hinge";
        case CHECK_PROPULSION: return "Check Propulsion";
        case PROPULSION_THRUST: return "Propulsion Thrust";
        case COIL_UNWINDING: return "Coil Unwinding";
        case STANDBY: return "Standby";
        case INSTRUMENT_CHECKOUT: return "Instrument Checkout";
        case EXPERIENCE_DATA: return "Experience Data";
        case DATA_DOWNLINK: return "Data Downlink";
        case SAFE_MODE: return "Safe Mode";
        case SURVIVAL: return "Survival";
        case END_OF_MISSION: return "End of Mission";
        default: return "Unknown Status";
    }
}

// Fonction pour obtenir la couleur du statut
const char* get_status_color(SystemStatus status) {
    switch(status) {
        case SAFE_MODE:
        case SURVIVAL:
        case END_OF_MISSION:
            return RED;
        case COIL_WINDING:
        case OPEN_HINGE:
        case PROPULSION_THRUST:
        case COIL_UNWINDING:
        case DATA_DOWNLINK:
            return BLUE;
        case ASCENT:
        case PRE_SEPARATION_WAKE_UP:
        case SEPARATION:
        case POST_SEPARATION_WAKE_UP:
        case MECHANICAL_DEPLOYMENTS:
        case SYSTEM_CHECKOUT:
        case CHECK_COIL_CONTROL:
        case LIMIT_SWITCH_ON:
        case CHECK_HINGE_CONTROL:
        case CHECK_PROPULSION:
        case STANDBY:
        case INSTRUMENT_CHECKOUT:
        case EXPERIENCE_DATA:
            return MAGENTA;
        default:
            return RESET;
    }
}


void check_transition(bool valid){

} 

void transition_to_status(SystemStatus *current_status, SystemStatus new_status) {
    
    bool valid = *current_status < new_status;
    ESP_LOGI(TAG, "Transition %s%s%s from %d to  %d : %s\n", valid ? GREEN : RED,
                                                             valid ? "VALID" : "UNVALID", 
                                                             RESET,
                                                             *current_status, 
                                                             new_status, 
                                                             get_status_name(new_status));
    *current_status = new_status;
}

void display_current_status(SystemStatus status) {
    ESP_LOGI(TAG, "Current Status: %s%s%s\n",
           get_status_color(status),
           get_status_name(status),
           RESET);
}