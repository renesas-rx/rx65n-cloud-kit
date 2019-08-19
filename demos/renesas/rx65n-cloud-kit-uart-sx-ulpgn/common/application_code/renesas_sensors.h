/*
 * sensors.h
 *
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <sensorlib/rxdemo_i2c_api.h>
#include "sensorlib/bmi160_sensor.h"
#include "sensorlib/bme680_sensor.h"
#include "sensorlib/isl29035_sensor.h"


/** BMI160 values */
struct bmi160_data {
    double     xacc;       ///< Acceleration on X-axis
    double     yacc;       ///< Acceleration on Y-axis
    double     zacc;       ///< Acceleration on Z-axis
};

/** LED's on the board */
struct leds_state {
    int     red;        ///< State of red LED
    int     yellow;     ///< State of yellow LED
    int     green;      ///< State of green LED
};

/** Sensor and LED values */
typedef struct sensors {
    struct bmi160_data      accel;       ///< Accelerometer
    struct bmi160_data      gyro;        ///< Gyroscope
    double                  temperature; ///< Temperature
    double                  humidity;    ///< Humidity
    double                  pressure;    ///< Barometric pressure
    double                  als;         ///< Ambient Light sensor
} rx_sensor_data;

void read_sensor(struct sensors *sens);
uint8_t init_sensors(void);

#endif /* SENSORS_H_ */
