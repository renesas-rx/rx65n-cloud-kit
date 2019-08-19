#include "platform.h"
#include "renesas_provisioning.h"
#include "serial_term_uart.h"
#include "r_flash_rx_if.h"

const rx_provisioning_info *rx_provisioning_data = RX_PROVISIONING_ADDR;

uint8_t write_provisioning_data(rx_provisioning_info *record);
static void reboot();

uint8_t rx_check_provisioning() {
	uint16_t offset, len;
	uint8_t state = 1, goback = 0;
	uint8_t err;
	uint8_t reprovision = 0;
	uint32_t countdown = 10000000;
	char choice;

	if (RX_PROVISIONING_MAGIC == rx_provisioning_data->magic) {
		uart_string_printf("Provisioning information found.\r\nPlease press any button to reprovision.\r\n");
		// Note: calling vTaskDelay() here results in unspecified behavior, so we are using a loop
		while(countdown-- > 0) {
			if (uart_has_input() > 0) {
				reprovision = 1;
				break;
			}
		}
		uart_string_printf("\r\n");
		if (reprovision == 0) {
			return 0;
		} else {
			uart_string_printf("Clearing provisioning record...\r\n");
			rx_clean_provisioning();
			uart_string_printf("Please wait...\r\n");
			uart_flush();
			reboot();
		}
	}

	volatile rx_provisioning_info candidate;

	uart_string_printf("Welcome to TB-RX65N!\r\n");
	uart_string_printf("Please follow the next steps to provision your device.\r\n");

	while(1) {
		switch(state) {
		case 1:
			uart_string_printf("Please input your WiFi SSID and press Enter.\r\n");
			candidate.wifi_ssid_len = (uint8_t)uart_input(candidate.wifi_ssid, sizeof(candidate.wifi_ssid));
			state++;
			break;
		case 2:
			uart_string_printf("Input your WiFi password and press Enter.\r\n");
			candidate.wifi_pass_len = (uint8_t)uart_input(candidate.wifi_pass, sizeof(candidate.wifi_pass));
			state++;
			break;

		case 3:
			uart_string_printf("Input the AWS device name and press Enter.\r\n");
			candidate.device_name_len = uart_input(candidate.device_name, sizeof(candidate.device_name));
			state++;
			break;

		case 4:
			uart_string_printf("Input the AWS endpoint address and press Enter.\r\n");
			candidate.endpoint_len = uart_input(candidate.endpoint, sizeof(candidate.endpoint));
			state++;
			break;

		case 5:
			memset(candidate.certificate, 0, sizeof(candidate.certificate));
			uart_string_printf("Copy and paste the certificate and press Enter.\r\n");
			offset = 0;
			while(NULL == strstr(candidate.certificate, "-END CERTIFICATE-")) {
				len = uart_input(candidate.certificate + offset, sizeof(candidate.certificate) - offset);
				candidate.certificate[offset+len] = '\n';
				offset += len + 1;
			}
			offset--;
			candidate.certificate[offset] = 0;
			candidate.certificate_len = offset;
			state++;
			break;


		case 6:
			memset(candidate.private_key, 0, sizeof(candidate.private_key));
			uart_string_printf("Copy and paste the private key and press Enter.\r\n");
			offset = 0;
			while(NULL == strstr(candidate.private_key, "-END RSA PRIVATE KEY-")) {
				len = uart_input(candidate.private_key + offset, sizeof(candidate.private_key) - offset);
				candidate.private_key[offset+len] = '\n';
				offset += len + 1;
			}
			offset--;
			candidate.private_key[offset] = 0;
			candidate.private_key_len = offset;
			state++;
			break;

		case 7:
			uart_string_printf("\r\n\r\nWould you like to change any of the settings?\r\n");
			uart_string_printf("1) WiFi SSID (");
			uart_string_printf(candidate.wifi_ssid);
			uart_string_printf(")\r\n2) WiFi Password (");
			uart_string_printf(candidate.wifi_pass);
			uart_string_printf(")\r\n3) Device Name (");
			uart_string_printf(candidate.device_name);
			uart_string_printf(")\r\n4) AWS Endpoint (");
			uart_string_printf(candidate.endpoint);
			uart_string_printf(")\r\n5) Certificate (entered)");
			uart_string_printf("\r\n6) Private key (entered)");
			uart_string_printf("\r\n\r\n0) Save settings and exit\r\n");

			uart_input(&choice, &len, 1);
			if (choice >= '0' && choice <= '6') {
				state = choice - '0';
				if (state == 0) {
					state = 8;
				}
				goback = state;
			}
			break;

		case 8:

			uart_string_printf("Writing settings to flash...\r\n");
			uart_flush();
			candidate.magic = RX_PROVISIONING_MAGIC;

			if (0 == write_provisioning_data(&candidate)) {
				uart_string_printf("Please wait...\r\n");
				uart_flush();
				reboot();
			} else {
				uart_string_printf("Error during writing flash memory.\r\n");
				uart_flush();
				while(1) { }
			}
		}

		if (state < 7 && goback > 0 && goback != state) {
			state = 7;
		}
	}
}

uint8_t rx_clean_provisioning() {
	rx_provisioning_info blank_record;
	return write_provisioning_data(&blank_record);
}

uint8_t write_provisioning_data(rx_provisioning_info *record) {
	uint8_t err;
	size_t offset;

	//taskENTER_CRITICAL();

	err = R_FLASH_Open();

	flash_access_window_config_t access_info;
	uint32_t address = RX_PROVISIONING_ADDR & ~(FLASH_CF_MEDIUM_BLOCK_SIZE - 1);

	err = R_FLASH_Control(FLASH_CMD_STATUS_GET, NULL);
	if (err == 0) {
		err = R_FLASH_Control(FLASH_CMD_ACCESSWINDOW_GET, (void *)&access_info);
	}

	if (err == 0) {
		access_info.start_addr = (uint32_t) FLASH_CF_BLOCK_37;
		access_info.end_addr = (uint32_t) FLASH_CF_BLOCK_0;
		err = R_FLASH_Control(FLASH_CMD_ACCESSWINDOW_SET, (void *)&access_info);
	}

	if (err == 0) {
		err = R_FLASH_Erase((flash_block_address_t) address, 1);
	}

	for (offset = 0; offset < sizeof(rx_provisioning_info); offset += 128) {
		if (err == 0) {
			err = R_FLASH_Write((uint32_t)record + offset, RX_PROVISIONING_ADDR + offset, 128);
			nop();
			nop();
			nop();
			nop();
			nop();
		}
	}
	//taskEXIT_CRITICAL();

	if (err == 0) {
		for (int i = 0; i < sizeof(rx_provisioning_info); i++) {
			char a = ((char *)rx_provisioning_data)[i];
			char b = ((char *)record)[i];
			if (a != b) {
				uart_string_printf("Error writing flash");
			}
		}
	}


	if (err == 0) {
		if (rx_provisioning_data->magic != RX_PROVISIONING_MAGIC) {
			err = 1;
		}
	}
	return err;
}

static void reboot() {
    //WDT Control Register (WDTCR)
    WDT.WDTCR.BIT.TOPS = 0;
    WDT.WDTCR.BIT.CKS  = 1;
    WDT.WDTCR.BIT.RPES = 3;
    WDT.WDTCR.BIT.RPSS = 3;
    //WDT Status Register
    WDT.WDTSR.BIT.CNTVAL = 0;
    WDT.WDTSR.BIT.REFEF  = 0;
    WDT.WDTSR.BIT.UNDFF  = 0;
    //WDT Reset Control Register
    WDT.WDTRCR.BIT.RSTIRQS = 1;
    //Non-Maskable Interrupt Enable Register (NMIER)
    ICU.NMIER.BIT.WDTEN    = 0;

    WDT.WDTRR = 0;
    WDT.WDTRR = 0xff;

    while (1); // Wait for Watchdog to kick in
}
