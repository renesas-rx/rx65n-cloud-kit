#ifndef RENESAS_PROVISIONING_H_
#define RENESAS_PROVISIONING_H_

#define RX_PROVISIONING_MAGIC (uint16_t)61125
#define RX_PROVISIONING_ADDR (uint32_t)0xFFFF8000

typedef struct {

	char wifi_ssid[128];
	uint8_t wifi_ssid_len;
	char wifi_pass[128];
	uint8_t wifi_pass_len;
	char endpoint[256];
	uint16_t endpoint_len;
	char device_name[256];
	uint16_t device_name_len;
	char private_key[2048];
	uint16_t private_key_len;
	char certificate[2048];
	uint16_t certificate_len;
	uint16_t magic;

	// flash writes need to be in multiples of 128 bytes
	char _padding[128-sizeof(uint16_t)*6];
} rx_provisioning_info;

uint8_t rx_check_provisioning();
uint8_t rx_clean_provisioning();

#endif /* RENESAS_PROVISIONING_H_ */

