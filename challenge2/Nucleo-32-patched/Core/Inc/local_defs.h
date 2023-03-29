// Header file to store all common defs
#ifndef LOCAL_DEFS_H
#define LOCAL_DEFS_H

#define HIGH true  // Carry over from Arduino nomenclature/easier to read
#define LOW false  // These are currently only used in the Bumper class

/* Direct PIN PWM */
// #define PWM_LOW 90
// #define PWM_HIGH 10

/* PWM Driver */
#define PWM_LOW 1
#define PWM_HIGH 20  // starts to get blinding above 20

// These are the PGN values, not the EXT ID values. The PGN is extracted in the recv
// segment of code, and the priority/source address are ignored.
#define PGN_CruiseControlVehicleSpeed1     0xFEF1
#define PGN_OperatorsExternalLightControls 0xFDCC

#define TicksPerSecond 999  // This value is derived from timer 3 counter period, -1 due to 0 index

#endif /* LOCAL_DEFS_H */
