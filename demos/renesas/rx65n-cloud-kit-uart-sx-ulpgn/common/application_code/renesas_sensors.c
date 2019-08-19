/*
 * sensors.c
 *
 */

#include <math.h>
#include "renesas_sensors.h"
#include "r_riic_rx_if.h"


struct bmi160_dev bmi160;
struct bme680_dev gas_sensor;
struct isl29035_dev isl_dev;

/*wrapper function to match the signature of bmm150.read */
static int8_t bmi160_aux_rd(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
    int8_t rslt;
    (void) ((id));

    /* Discarding the parameter id as it is redundant*/
    rslt = bmi160_aux_read(reg_addr, aux_data, len, &bmi160);

    return rslt;
}

/*wrapper function to match the signature of bmm150.write */
static int8_t bmi160_aux_wr(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
    int8_t rslt;
    (void) ((id));

    /* Discarding the parameter id as it is redundant */
    rslt = bmi160_aux_write(reg_addr, aux_data, len, &bmi160);

    return rslt;
}


static int8_t isl29035_Initialize(void)
{
    int8_t status = 0;

    /* ISL29035 Sensor Initialization */
    isl_dev.id = ISL29035_I2C_ADDR;
    isl_dev.interface = ISL29035_I2C_INTF;
    isl_dev.read = rxdemo_i2c_read;
    isl_dev.write = rxdemo_i2c_write;
    isl_dev.delay_ms = rxdemo_delay_ms;

    status = isl29035_init(&isl_dev);
    if( status != ISL29035_OK)
    {
        //print_to_console("ISL 29035 sensor init failed!!!\r\n");
        return status;
    }
    /* Configure ISL29035 ALS Sensor */
    status = isl29035_configure(&isl_dev);
    if( status != ISL29035_OK)
    {
        //print_to_console("ISL 29035 sensor sensor config failed!!!\r\n");
        return status;
    }

    return status;
}

static int8_t bmi160_Initialize(void)
{
    int8_t status;

    /* BMI160 Sensor Initialization */
    bmi160.id = BMI160_I2C_ADDR;
    bmi160.interface = BMI160_I2C_INTF;
    bmi160.read = rxdemo_i2c_read;
    bmi160.write = rxdemo_i2c_write;
    bmi160.delay_ms = rxdemo_delay_ms;

    status = bmi160_init(&bmi160);
    if( status != BMI160_OK)
    	return status;

    /* Configure BMI160 Accel and Gyro sensor */

    /* Select the Output data rate, range of accelerometer sensor */
    bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_4G;
    bmi160.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

    /* Select the power mode of accelerometer sensor */
    bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Select the Output data rate, range of Gyroscope sensor */
    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

    /* Select the power mode of Gyroscope sensor */
    bmi160.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    /* Set the sensor configuration */
    status = bmi160_set_sens_conf(&bmi160);

    return status;
}

static int8_t bme680_Initialize(void)
{
    uint8_t set_required_settings;
    int8_t status;

    /* BME680 Sensor Initialization */
    gas_sensor.dev_id = BME680_I2C_ADDR_PRIMARY;
    gas_sensor.intf = BME680_I2C_INTF;
    gas_sensor.read = rxdemo_i2c_read;
    gas_sensor.write = rxdemo_i2c_write;
    gas_sensor.delay_ms = rxdemo_delay_ms;

    status = bme680_init(&gas_sensor);
    if( status != BME680_OK)
        return status;

    /* Configure BME680 Sensor */
    /* Set the temperature, pressure and humidity settings */
    gas_sensor.tph_sett.os_hum = BME680_OS_2X;
    gas_sensor.tph_sett.os_pres = BME680_OS_4X;
    gas_sensor.tph_sett.os_temp = BME680_OS_8X;
    gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

    /* Set the remaining gas sensor settings and link the heating profile */
    gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
    /* Create a ramp heat waveform in 3 steps */
    gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
    gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */

    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    gas_sensor.power_mode = BME680_FORCED_MODE;

    /* Set the required sensor settings needed */
    set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL
                            | BME680_GAS_SENSOR_SEL;

    /* Set the desired sensor configuration */
    status = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
    if( status != BME680_OK)
        return status;

    /* Set the power mode */
    status = bme680_set_sensor_mode(&gas_sensor);
    if( status != BME680_OK)
        return status;

    /* Get the total measurement duration so as to sleep or wait till the
     * measurement is complete */
    bme680_get_profile_dur(&gas_sensor.meas_period, &gas_sensor);

    return status;
}

/*
 * Convert the units for temperature from celcius to Fahrenheit
 */
static double convert_celsius_2_Fahrenheit(double *temp_celsius)
{
    return (((*temp_celsius * 9) / 5) + 32);
}


uint8_t init_sensors(void)
{
	uint8_t err = 0;

    /* Open I2C driver instance */
	riic_info_t iic_info;
	iic_info.dev_sts = RIIC_NO_INIT;
	iic_info.ch_no = 0;
	err = R_RIIC_Open(&iic_info);

	if (err == 0) {
		/* Initialize & Configure the BMI160 Sensor */
		err = bmi160_Initialize();
	}

    /* Initialize & Configure the BME680 Sensor */
    if (err == 0) {
    	err = bme680_Initialize();
    }

    /* Initialize & Configure the ISL29035 Sensor */
    if (err == 0) {
    	err = isl29035_Initialize();
    }

    return err;
}


void read_sensor(rx_sensor_data *sens)
{
    struct bme680_field_data  bme680_data;
    bmi160_data accel_data;
    bmi160_data gyro_data;
    uint8_t mag_data[8];
    double temp_value;
    uint8_t status;

    /* To read both Accel and Gyro data */
    status = bmi160_get_sensor_data(BMI160_BOTH_ACCEL_AND_GYRO, &accel_data, &gyro_data, &bmi160);
    if(status == BMI160_OK)
    {
        sens->accel.xacc = accel_data.x_axis;
        sens->accel.yacc = accel_data.y_axis;
        sens->accel.zacc = accel_data.z_axis;

        sens->gyro.xacc = gyro_data.x_axis;
        sens->gyro.yacc = gyro_data.y_axis;
        sens->gyro.zacc = gyro_data.z_axis;
    }

    //Read temperature, pressure and humidity data
    status = bme680_read_sensor(&bme680_data, &gas_sensor);
    if(status == BME680_OK)
    {
        temp_value = bme680_data.temperature/100.0f;
        sens->temperature = convert_celsius_2_Fahrenheit(&temp_value);
        sens->humidity = ((double)bme680_data.humidity/1000.0f);
        sens->pressure = ((double)bme680_data.pressure/100.0f);
    }

    status = isl29035_read_als_data(&isl_dev, &sens->als);

    return status;
}

