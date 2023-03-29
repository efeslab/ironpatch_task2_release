// Header file to store all common defs
#ifndef LOCAL_DEFS_H
#define LOCAL_DEFS_H

/* Direct PIN PWM */
// #define PWM_LOW 90
// #define PWM_HIGH 10

/* PWM Driver */
#define PWM_PERIOD 1000
#define PWM_LOW 10
#define PWM_HIGH 100  // starts to get blinding above 20
#define PWM_PERIOD_STR "1000"
#define PWM_LOW_STR "10"
#define PWM_HIGH_STR "100"

// These are the PGN values, not the EXT ID values. The PGN is extracted in the recv
// segment of code, and the priority/source address are ignored.
#define PGN_CruiseControlVehicleSpeed1     0xFEF1
#define PGN_OperatorsExternalLightControls 0xFDCC

#endif /* LOCAL_DEFS_H */
