/*
 * rxdemo_i2c_api.c
 *
 *  Created on: Jan 8, 2018
 *      Author: Rajkumar.Thiagarajan
 */

#include "rxdemo_i2c_api.h"

void rxdemo_delay_ms(uint32_t delay)
{
    vTaskDelay(delay);
}

static void _sensor_callback(void) {

}

int8_t rxdemo_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	uint8_t wBuf[2];
	uint8_t err;
	wBuf[0] = reg_addr;
	riic_info_t riic_info;
	riic_info.dev_sts = RIIC_NO_INIT;
	riic_info.ch_no = 0;
	riic_info.p_data1st = &reg_addr;
	riic_info.cnt1st = 1;
	riic_info.p_data2nd = data;
	riic_info.cnt2nd = len;
	riic_info.callbackfunc = _sensor_callback;
	riic_info.p_slv_adr = &dev_addr;


	err = R_RIIC_MasterReceive(&riic_info);

	if (RIIC_SUCCESS == err)
	{
	  while(RIIC_FINISH != riic_info.dev_sts) {
		  vTaskDelay(0);
	  }
	}

	return err;

	/*
    volatile ssp_err_t ssp_err = SSP_SUCCESS;

    //Lock the bus for exclusive access
    tx_mutex_get(&g_i2c_mutex, TX_WAIT_FOREVER);

    ssp_err = g_i2c0.p_api->slaveAddressSet(g_i2c0.p_ctrl, dev_addr, I2C_ADDR_MODE_7BIT);
    if(ssp_err != SSP_SUCCESS)
    {
        tx_mutex_put(&g_i2c_mutex);
        APP_ERR_TRAP(ssp_err);
    }

    wBuf[0] = reg_addr;
    ssp_err = g_i2c0.p_api->write(g_i2c0.p_ctrl, wBuf,1,true);
    if(ssp_err != SSP_SUCCESS)
    {
        tx_mutex_put(&g_i2c_mutex);
        APP_ERR_TRAP(ssp_err);
    }

    ssp_err = g_i2c0.p_api->read(g_i2c0.p_ctrl, data, len, false );
    if(ssp_err != SSP_SUCCESS)
    {
        tx_mutex_put(&g_i2c_mutex);
        APP_ERR_TRAP(ssp_err);
    }

    //unlock the bus
    tx_mutex_put(&g_i2c_mutex);
    return 0;
	*/
}

int8_t rxdemo_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	int8_t err;
	riic_info_t riic_info;
	riic_info.ch_no = 0;
	riic_info.dev_sts = RIIC_NO_INIT;
	riic_info.p_data1st = &reg_addr;
	riic_info.cnt1st = 1;
	riic_info.p_data2nd = data;
	riic_info.cnt2nd = len;
	riic_info.p_slv_adr = &dev_addr;
	riic_info.callbackfunc = _sensor_callback;

	err = R_RIIC_MasterSend(&riic_info);

	if (RIIC_SUCCESS == err)
	{
	  while(RIIC_FINISH != riic_info.dev_sts);
	}

	return err;

    /*
    ssp_err = g_i2c0.p_api->slaveAddressSet(g_i2c0.p_ctrl, dev_addr, I2C_ADDR_MODE_7BIT);
    if(ssp_err != SSP_SUCCESS)
    {
        tx_mutex_put(&g_i2c_mutex);
        APP_ERR_TRAP(ssp_err);
    }

    wBuf[j++] = reg_addr;

    for(j = 1; j <= len; j++,i++)
        wBuf[j] = data[i];

    ssp_err = g_i2c0.p_api->write(g_i2c0.p_ctrl, wBuf, (uint32_t)(len+1), false);
    if(ssp_err != SSP_SUCCESS)
    {
        tx_mutex_put(&g_i2c_mutex);
        APP_ERR_TRAP(ssp_err);
    }

    //unlock the bus
    tx_mutex_put(&g_i2c_mutex);
	*/
}

