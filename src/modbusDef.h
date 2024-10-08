#ifndef MODBUSDEF_H
#define MODBUSDEF_H

//Device modbus input registers
#define MODBUS_INPUTREG_STATUS 0
#define MODBUS_INPUTREG_POSITION 1
#define MODBUS_INPUTREG_OPENING_TIME 2
#define MODBUS_INPUTREG_COLLISION_THRESHOLD 3

//Device modbus holding registers
#define MODBUS_HOLDINGREG_COMMAND 0
#define MODBUS_HOLDINGREG_TARGET_POS 1

typedef enum {
    STOPPED,
    MOVING_UP,
    MOVING_DOWN,
    COLLISION,
    CALIBRATION
} ShutterState;


typedef enum {
    STOP,
    MOVEUP,
    MOVEDOWN,
    GOTARGET,
    CALIBRATE
} ShutterCommand;

#endif //MODBUSDEF_H