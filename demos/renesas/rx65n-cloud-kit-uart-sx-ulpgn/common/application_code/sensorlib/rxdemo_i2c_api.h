/*
 * synergy_i2c_api.h
 *
 *  Created on: Jan 8, 2018
 *      Author: Rajkumar.Thiagarajan
 */

#ifndef SYNERGY_I2C_API_H_
#define SYNERGY_I2C_API_H_

#include "platform.h"
#include "r_smc_entry.h"
#include "r_riic_rx_if.h"

int8_t rxdemo_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t rxdemo_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
void rxdemo_delay_ms(uint32_t delay);

#endif /* SYNERGY_I2C_API_H_ */
