# Space2Corps Actuator Control System - Technical Documentation

## Table of Contents
1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [State Machine](#state-machine)
4. [Hardware Interface](#hardware-interface)
5. [Software Modules](#software-modules)
6. [Communication Protocol](#communication-protocol)
7. [Development Guidelines](#development-guidelines)
8. [Deployment Procedure](#deployment-procedure)
9. [Troubleshooting](#troubleshooting)
10. [Future Enhancements](#future-enhancements)

## Project Overview

The Space2Corps Actuator Control System is an embedded system designed for CubeSat missions. It provides precise control over deployment mechanisms and system status management throughout the mission lifecycle.

### Mission Objectives
- Reliable deployment of mechanical systems in space
- Real-time monitoring of system status
- Remote control and telemetry capabilities
- Fault detection and recovery mechanisms

### Key Features
- **Precision Actuation**: PWM-controlled servo and stepper motor
- **State Management**: Finite state machine with 20 distinct states
- **Sensor Integration**: IMU (LSM6DSOX) and GPS modules
- **Wireless Control**: WiFi AP mode with UDP communication
- **Multi-tasking**: FreeRTOS-based concurrent operations

## System Architecture

### High-Level Architecture
```
+-------------------+
|   Mission Context |
+-------------------+
          |
          v
+-------------------+
|   State Machine   |
+-------------------+
          |
    +-----+----+----+
    |          |    |
    v          v    v
+--------+  +-------+  +--------+
| Actu- |
| ators |  | Sens- |  | WiFi   |
+--------+  | ors   |  | Comms  |
            +-------+  +--------+
```

### Software Layering
1. **Hardware Abstraction Layer**: Direct hardware control
2. **Device Driver Layer**: Sensor and actuator drivers
3. **State Management Layer**: Mission state machine
4. **Application Layer**: Main control logic
5. **Communication Layer**: WiFi interface

## State Machine

### State Diagram
```
[FLOOR] → [ASCENT] → [PRE_SEPARATION_WAKE_UP] → [SEPARATION]
    ↓
[POST_SEPARATION_WAKE_UP] → [MECHANICAL_DEPLOYMENTS]
    ↓
[SYSTEM_CHECKOUT] → [CHECK_COIL_CONTROL] → [COIL_WINDING]
    ↓
[LIMIT_SWITCH_ON] → [CHECK_HINGE_CONTROL] → [OPEN_HINGE]
    ↓
[CHECK_PROPULSION] → [PROPULSION_THRUST] → [COIL_UNWINDING]
    ↓
[STANDBY] → [INSTRUMENT_CHECKOUT] → [EXPERIENCE_DATA]
    ↓
[DATA_DOWNLINK] → [END_OF_MISSION]
    ↓
[SAFE_MODE] ← [SURVIVAL] (error states)
```

### State Descriptions

| State | Description | Entry Actions |
|-------|-------------|---------------|
| FLOOR | Ground operations | System initialization |
| ASCENT | Launch phase | Sensor monitoring |
| SEPARATION | Satellite deployment | Critical systems check |
| COIL_WINDING | Deployment mechanism | Motor activation |
| OPEN_HINGE | Hinge deployment | Servo positioning |
| PROPULSION_THRUST | Propulsion test | Thrust sequence |
| STANDBY | Low power mode | Minimal operations |
| SAFE_MODE | Error recovery | System diagnostics |

### State Transition Rules
- States progress sequentially in normal operation
- Transitions are validated: `current_status < new_status`
- Error states (SAFE_MODE, SURVIVAL) can be entered from any state
- Manual overrides possible via WiFi commands

## Hardware Interface

### Pin Assignment

#### ESP32-C6 GPIO Configuration

| Function | GPIO | Direction | Notes |
|----------|------|-----------|-------|
| Servo PWM | 18 | Output | 50Hz PWM signal |
| Limit Switch | 19 | Input | Active low |
| Stepper EN | 20 | Output | Enable (active low) |
| Stepper DIR | 21 | Output | Direction control |
| Stepper STEP | 22 | Output | Step pulse |
| Stepper M0 | 10 | Output | Microstep config |
| Stepper M1 | 11 | Output | Microstep config |
| Stepper M2 | 0 | Output | Microstep config |
| I2C SCL | 7 | Bidirectional | IMU communication |
| I2C SDA | 6 | Bidirectional | IMU communication |
| UART TX (GPS) | 15 | Output | GPS transmission |
| UART RX (GPS) | 23 | Input | GPS reception |

### Peripheral Configuration

#### Servo Motor
- **Frequency**: 50Hz
- **Pulse Width**: 500-2500μs (0-180°)
- **Resolution**: 16-bit PWM
- **Default Positions**:
  - HINGE_CLOSE: 0°
  - HINGE_OPEN: 90°

#### Stepper Motor (DRV8825)
- **Microstepping**: 1/8 step
- **Steps/Revolution**: 1600
- **Step Delay**: 4000μs (4ms)
- **Default Configuration**: M0=1, M1=1, M2=0

#### IMU (LSM6DSOX)
- **I2C Address**: 0x6A or 0x6B
- **Frequency**: 100kHz
- **Accelerometer**: ±2g, 1660Hz ODR
- **Gyroscope**: ±2000dps, 1660Hz ODR

#### GPS Module
- **UART**: UART_NUM_1
- **Baud Rate**: 9600
- **Protocol**: NMEA 0183
- **Supported Sentences**: GPRMC, GPGGA

## Software Modules

### Main Module (`main.h`, `main.c`)

#### Data Structures

```c
typedef struct Mission_Ctx {
    char mission_name[MAX_SIZE_NAME];      // Mission identifier
    SystemStatus current_status;           // Current state
    MotionData motion_data;                // IMU data
    GPSData gps_data;                      // GPS data
} Mission_Ctx;
```

#### Key Functions

| Function | Description | Parameters | Return |
|----------|-------------|------------|--------|
| `app_main()` | Entry point | void | void |
| `control_task()` | Main control loop | Task parameters | void |
| `init_context()` | Initialize mission | void | void |
| `handle_misson_status()` | State handler | SystemStatus | int |

#### Implementation Details
- Uses FreeRTOS tasks for concurrent operations
- Implements task watchdog timer (5s timeout)
- Color-coded logging for different status levels

### Actuator Module (`actuator.h`, `actuator.c`)

#### Configuration Macros

```c
// Servo Configuration
#define SERVO_PIN GPIO_NUM_18
#define SERVO_MIN_PULSEWIDTH 500
#define SERVO_MAX_PULSEWIDTH 2500
#define SERVO_FREQ 50
#define SERVO_MAX_DEGREE 180

// Stepper Configuration
#define STEPS_PER_REVOLUTION 1600
#define STEP_DELAY_US 4000
#define INITIAL_DELAY 5000
```

#### Key Functions

| Function | Description | Parameters | Return |
|----------|-------------|------------|--------|
| `init_actuator()` | Initialize all actuators | void | void |
| `init_servo()` | Configure PWM | void | void |
| `set_servo_position()` | Set servo angle | uint8_t degree | void |
| `init_motor_gpio()` | Configure stepper GPIOs | void | void |
| `rotate_steps()` | Rotate motor | int steps, bool direction | void |
| `check_limit_switch()` | Check limit switch | void | bool |

#### Implementation Details
- Uses LEDC for precise PWM control
- Implements limit switch debouncing
- Stepper motor uses GPIO bit-banging for precise timing
- Motor enable controlled to save power

### Status Module (`status.h`, `status.c`)

#### State Enumeration

```c
typedef enum {
    FLOOR,
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
```

#### Key Functions

| Function | Description | Parameters | Return |
|----------|-------------|------------|--------|
| `get_status_name()` | Get state name | SystemStatus | const char* |
| `get_status_color()` | Get display color | SystemStatus | const char* |
| `transition_to_status()` | Change state | SystemStatus*, SystemStatus | void |
| `display_current_status()` | Log current state | SystemStatus | void |

#### Implementation Details
- Color-coded status display using ANSI escape codes
- State transition validation
- Comprehensive logging of all state changes

### Sensors Module (`sensors.h`, `sensors.c`)

#### Data Structures

```c
typedef struct MotionData {
    bool motion_initialized;
    float accel_x, accel_y, accel_z;  // g units
    float gyro_x, gyro_y, gyro_z;    // degrees per second
    float temp;                       // temperature
} MotionData;

typedef struct GPSData {
    float latitude, longitude;        // decimal degrees
    float altitude;                   // meters
    float speed;                     // knots
    float hdop;                      // horizontal dilution
    int satellites;                  // number of satellites
    char time[15], date[15];         // strings
    bool has_fix;                    // valid fix flag
} GPSData;
```

#### Key Functions

| Function | Description | Parameters | Return |
|----------|-------------|------------|--------|
| `init_i2c()` | Initialize I2C bus | void | void |
| `init_uart()` | Initialize UART | void | void |
| `init_motion_sensors()` | Configure IMU | void | void |
| `read_motion_data()` | Read IMU data | void | void |
| `read_gps_data()` | Read GPS data | void | void |
| `parse_nmea_sentence()` | Parse GPS data | char* | void |

#### Implementation Details
- Automatic I2C address detection
- NMEA sentence validation with checksum
- Data conversion from raw to engineering units
- Comprehensive error handling and recovery

### WiFi Module (`wifi.h`, `wifi.c`)

#### Configuration

```c
#define SSID "ESP32-C6_AP"
#define PASSWORD "password123"
#define PORT 3333
```

#### Key Functions

| Function | Description | Parameters | Return |
|----------|-------------|------------|--------|
| `wifi_init_softap()` | Initialize AP mode | void | void |
| `wifi_task()` | WiFi communication task | Task parameters | void |

#### Implementation Details
- Uses ESP-IDF WiFi stack
- UDP socket for command/control
- Event-driven connection management
- Simple ACK response protocol

## Communication Protocol

### WiFi Interface

#### Network Configuration
- **SSID**: ESP32-C6_AP
- **Password**: password123
- **IP Address**: DHCP assigned
- **Port**: 3333 (UDP)

#### Supported Commands

| Command | Description | Response |
|---------|-------------|----------|
| STATUS | Query current status | Current state name |
| OPEN_HINGE | Open hinge mechanism | ACK/NACK |
| CLOSE_HINGE | Close hinge mechanism | ACK/NACK |
| STANDBY | Enter standby mode | ACK/NACK |

#### Message Format
- **Request**: ASCII string terminated by newline
- **Response**: ASCII string "ACK" or error message
- **Maximum Length**: 100 bytes

### Serial Interface

#### Logging Format
- **Level**: INFO, ERROR, DEBUG
- **Format**: `[TAG] message`
- **Colors**: State-dependent coloring

#### Example Output
```
[MASTER_STATUS] Current Status: BLUECoil WindingRESET
[MASTER_ACTUATOR] Position at 90° Duty = 32768
[MASTER_SENSORS] Accel: X=0.12 g Y=-0.05 g Z=0.98 g | Gyro: X=1.23 dps Y=-0.45 dps Z=2.10 dps
```

## Development Guidelines

### Coding Standards

#### Naming Conventions
- **Variables**: `snake_case`
- **Functions**: `snake_case`
- **Macros**: `UPPER_SNAKE_CASE`
- **Types**: `PascalCase`
- **Constants**: `UPPER_SNAKE_CASE`

#### File Organization
- **Header Files**: `.h` in `include/` directory
- **Source Files**: `.c` in `src/` directory
- **One Class/Module per File Pair**

#### Documentation
- **Function Comments**: Doxygen-style
- **File Headers**: Copyright, description, author
- **Inline Comments**: Explain complex logic

### Build Configuration

#### PlatformIO Configuration
```ini
[env:esp32-c6-devkitc-1]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
monitor_speed = 115200
```

#### Required Dependencies
- ESP-IDF v5.5.0
- FreeRTOS
- LEDC Driver
- I2C Master Driver
- UART Driver

### Testing Strategy

#### Unit Testing
- **Framework**: Unity or custom
- **Coverage**: Critical functions (state transitions, actuator control)
- **Mocking**: Hardware dependencies

#### Integration Testing
- **State Machine**: Verify all transitions
- **Sensor Fusion**: Validate data integration
- **Communication**: Test command protocol

#### System Testing
- **End-to-End**: Full mission simulation
- **Failure Modes**: Test error recovery
- **Performance**: Timing and resource usage

## Deployment Procedure

### Hardware Setup

1. **Power Connection**
   - ESP32-C6: 5V via USB
   - Servo Motor: 6V external supply
   - Sensors: 3.3V from ESP32

2. **Wiring Verification**
   - Check all GPIO connections
   - Verify I2C pull-up resistors
   - Confirm UART cross-connect

3. **Mechanical Setup**
   - Secure all components
   - Verify limit switch positioning
   - Check hinge mechanism range

### Software Deployment

1. **Build Process**
   ```bash
   pio run -t menuconfig  # Configure if needed
   pio run               # Build project
   pio run -t upload     # Flash to device
   ```

2. **Initialization Sequence**
   - Power on device
   - Wait for WiFi AP to start (~10s)
   - Connect to ESP32-C6_AP network
   - Verify serial output

3. **Pre-Flight Checklist**
   - [ ] All sensors reporting valid data
   - [ ] Actuators responding to commands
   - [ ] State machine in FLOOR state
   - [ ] WiFi communication functional
   - [ ] Battery voltage nominal

### Operational Procedures

#### Normal Operation
1. Monitor system status via serial or WiFi
2. Verify state transitions occur as expected
3. Check sensor data for anomalies
4. Respond to any error conditions promptly

#### Emergency Procedures
1. **Safe Mode Entry**
   - Send STANDBY command
   - Verify transition to SAFE_MODE
   - Check error logs

2. **Manual Override**
   - Use WiFi commands to control actuators
   - Bypass state machine if necessary
   - Reset system if required

3. **Recovery**
   - Power cycle if unresponsive
   - Reflash firmware if corrupted
   - Check hardware connections

## Troubleshooting

### Common Issues

#### WiFi Connection Problems
- **Symptom**: Cannot connect to AP
- **Causes**:
  - WiFi stack not initialized
  - Incorrect credentials
  - RF interference
- **Solutions**:
  - Check `wifi_init_softap()` return codes
  - Verify SSID and password
  - Move away from interference sources

#### Sensor Communication Errors
- **Symptom**: No IMU or GPS data
- **Causes**:
  - I2C/UART not initialized
  - Incorrect wiring
  - Sensor not powered
- **Solutions**:
  - Check `init_i2c()` and `init_uart()`
  - Verify voltage levels
  - Scan I2C bus for devices

#### Actuator Malfunction
- **Symptom**: Servo or motor not responding
- **Causes**:
  - GPIO configuration error
  - Power supply issue
  - Mechanical obstruction
- **Solutions**:
  - Verify `init_actuator()` execution
  - Check power connections
  - Test without mechanical load

#### State Machine Issues
- **Symptom**: Unexpected state transitions
- **Causes**:
  - Invalid transition logic
  - Race conditions
  - Sensor data errors
- **Solutions**:
  - Review `handle_misson_status()`
  - Add debug logging
  - Verify sensor inputs

### Debugging Techniques

1. **Serial Logging**
   - Enable verbose logging
   - Monitor all state transitions
   - Check for error messages

2. **Hardware Checks**
   - Verify all voltages
   - Check continuity of connections
   - Test individual components

3. **Software Verification**
   - Review task priorities
   - Check for stack overflows
   - Verify memory allocation

## Future Enhancements

### Planned Features

1. **Automatic Calibration**
   - Servo position calibration
   - IMU bias compensation
   - GPS antenna characterization

2. **Enhanced Communication**
   - TCP/IP support
   - Encrypted communication
   - Telemetry data streaming

3. **Improved Fault Detection**
   - Anomaly detection algorithms
   - Predictive maintenance
   - Automated recovery procedures

4. **Power Management**
   - Low power modes
   - Sleep/wake cycles
   - Energy monitoring

### Architectural Improvements

1. **Modularization**
   - Separate hardware abstraction layer
   - Platform-independent interfaces
   - Driver isolation

2. **Testing Framework**
   - Unit test suite
   - Continuous integration
   - Automated testing

3. **Documentation**
   - API reference
   - User manual
   - Development guide

## Appendix

### Acronyms
- **AP**: Access Point
- **IMU**: Inertial Measurement Unit
- **GPS**: Global Positioning System
- **PWM**: Pulse Width Modulation
- **UDP**: User Datagram Protocol
- **I2C**: Inter-Integrated Circuit
- **UART**: Universal Asynchronous Receiver-Transmitter
- **NMEA**: National Marine Electronics Association

### References
- ESP32-C6 Technical Reference Manual
- ESP-IDF Programming Guide
- FreeRTOS API Reference
- LSM6DSOX Datasheet
- DRV8825 Stepper Driver Datasheet

### Revision History
- **v1.0**: Initial documentation (2024)
- **v1.1**: Added troubleshooting section
- **v1.2**: Enhanced state machine documentation

---

*This documentation is generated and maintained by Mistral Vibe*
*Last updated: 2024*
