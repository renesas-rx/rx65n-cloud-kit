#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H

#include <stdint.h>

/*
 * PEM-encoded client certificate.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
#define keyCLIENT_CERTIFICATE_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDWjCCAkKgAwIBAgIVAND+gmODMCHx5tXZmayxkgLpHwzEMA0GCSqGSIb3DQEB\n"\
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"\
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xOTA0MDQxODQ2\n"\
"NTZaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"\
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDbdxIh1ekJhW0grI3W\n"\
"us4vtk3hGoOBYq2B4nFPTJs/U42qAfHYBUJDpPT5R7NiOWnyq4rGdHytGDrtvQm/\n"\
"1RwRMZrh3PRDtnpkBchwef02/1pazQzVk4/yvlDpxYnpXZhS/bV9W95VToSbA3pc\n"\
"Vfclrdpf77YrS+eKPfa1fKh4OeDhG5f0gw/qt5FFwyLp7gpDgdawWZwUNYmSBmJB\n"\
"F/i6nQk+YSKcKoYxMgYdh6u/ZeNVdNtQMIh0LUc+wyP23rgbYwvYFCIkNkth5rrS\n"\
"hHLVFFbmDbcaXo66ayBxBebTrFlJiTlwb3fBVSx5SVDNK9q1gWtZCJV5A4MotGwe\n"\
"BljTAgMBAAGjYDBeMB8GA1UdIwQYMBaAFAI/OZlHrcRbi4Wvov1DzG18zk5DMB0G\n"\
"A1UdDgQWBBTnw8F+u8OIPik81GV0fdgkpulzczAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"\
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAS8b/IuHMm3DEmudo3Chfo2wx\n"\
"nFEVIM0RgdIuqGRDaERJsONcapOW58lyBcegeG5ZJvOu4QwDbVWSghW6J+htpXcB\n"\
"VkqANgJDkFbwC2rptuPXFT6MmXthA13KesxvzLjeDa6h0a5+ryn491IL0akeRo4n\n"\
"gh/8jOn7jP2TJmICIKyCUw2PBijzl0dTXQC+ESYpgtNvxZEBcm8hw7bliVPSgCsq\n"\
"YovGZrjzqDqF8W1oHzG5tMO2ssYixqWU+r1a7aA2bYuyD3+zYYRgg+bPjOnodGP4\n"\
"QWwlWAFGxILdkJ+RokpL/978E1DhQJ/+n+LLLjbPzd0dM5CBSY2rSy2CnTRn4g==\n"\
"-----END CERTIFICATE-----"

/*
 * PEM-encoded client private key.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----"
 */
#define keyCLIENT_PRIVATE_KEY_PEM \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEowIBAAKCAQEA23cSIdXpCYVtIKyN1rrOL7ZN4RqDgWKtgeJxT0ybP1ONqgHx\n"\
"2AVCQ6T0+UezYjlp8quKxnR8rRg67b0Jv9UcETGa4dz0Q7Z6ZAXIcHn9Nv9aWs0M\n"\
"1ZOP8r5Q6cWJ6V2YUv21fVveVU6EmwN6XFX3Ja3aX++2K0vnij32tXyoeDng4RuX\n"\
"9IMP6reRRcMi6e4KQ4HWsFmcFDWJkgZiQRf4up0JPmEinCqGMTIGHYerv2XjVXTb\n"\
"UDCIdC1HPsMj9t64G2ML2BQiJDZLYea60oRy1RRW5g23Gl6OumsgcQXm06xZSYk5\n"\
"cG93wVUseUlQzSvatYFrWQiVeQODKLRsHgZY0wIDAQABAoIBABxuPnEXrwrGuC5l\n"\
"bDKHHLLdoRUqRCL8z5Fl7lvhhrMbddyyR8w+UilFM18fxMNSwMDpLB8tdAFQhEHM\n"\
"HXk2eF5Z2MCtyQ8EkrJmdWLsyrrXNbNRoPoCFcHSmdTO/GC4+US3DU3iGtleRJt4\n"\
"Rd+0W0IMVZ/cA/Nz79je0hkMvx5j06m4u7IRuQcjPcikXE7PF5lpGYy7Gzo2dju/\n"\
"1qvIBPR4rYcPKee156vww1A9s4Xl39SK4LySbqUMlPhdly1P/EC3mkNRmh9P4IHd\n"\
"U6cUzlKY7OENwiL++UybnOwAYVp+9r2cYU2dF2gzv9CQPUV/Z8cgaUzCrDCd0SaY\n"\
"JpDTRVECgYEA8toWQz9mYx6U2HCfjod7hO4rXvDeID5KC07dYItHQdaGJ5WJBnST\n"\
"6TMuSlNkIlimMki/QCo21pUIzXXPfd6d+xNFJqzA6iJMjlIMtZw2Nlyith4wGhqw\n"\
"0wcn+CV/YXnTqW2AgjZteOXExAKYsXMCPTWuzkMW13D6r205DIumQZ0CgYEA51jY\n"\
"LdiW8WZkEPGJ83PF+OzdxBnNI7+tH4POaf29ZOuEo41S3UoHZ5DJcJ4lHJ747jAH\n"\
"CozxdNt8tAMhX+5s9LvF/9tsYiJMKewnxQizRGN63uWT0qYHs4LehT0SsSrMSXEy\n"\
"qUvJRuJ25eQk5duuEdBi2nEwQFB2SjjCUZyScS8CgYEAvcQS/2tsS2nbL6IZ3Bsc\n"\
"NwyREXgwaouh+mIe9NEA1n6WiBn+X+/6oJBgzK2LIpeZDtPvtmkGPvloTnyPrbCm\n"\
"TEqLj6QQfyRcJB4TVlqBtkr+zkaamdtfbxc5cVuDxRyYe91YPOfg9Ua+Udghjw/q\n"\
"oP/w37ZnA+SBz1EO8jGxsbUCgYAUagORcBZJsOZNlHK+1811XLlo73npmKVXLZNO\n"\
"SQSoXhvzLqpMOpcSrXUJCftuXhzLL9O/O5am4CNN4yTTyT82DUe4fw0+T5Z28I7F\n"\
"1TG0bC7QcmOr59iwH3ExQqVems8x/9oflD9rkNo7CPm0isD+c2v1qJ1AX5lAjWeg\n"\
"Ofe2IwKBgBeHT4BLvWNfhsasyyM5PdnRZOWQOW+3mlQwc5cx6WwAUOH3Nq8wu7X3\n"\
"xSwa8hRCGGQhIlH9MHg1VPGvOEI8DV4sVAtK5CB+hyVOHGSE9FYwnRFe7iaf0anF\n"\
"PB25pjKEULnZPYTygG2xfIlF5F8X+VrMQQqzFeFWMG6+WVXVEW+m\n"\
"-----END RSA PRIVATE KEY-----"

/*
 * PEM-encoded Just-in-Time Registration (JITR) certificate (optional).
 *
 * If used, must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  NULL

/* The constants above are set to const char * pointers defined in aws_dev_mode_key_provisioning.c,
 * and externed here for use in C files.  NOTE!  THIS IS DONE FOR CONVENIENCE
 * DURING AN EVALUATION PHASE AND IS NOT GOOD PRACTICE FOR PRODUCTION SYSTEMS
 * WHICH MUST STORE KEYS SECURELY. */
extern const char clientcredentialCLIENT_CERTIFICATE_PEM[];
extern const char* clientcredentialJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM;
extern const char clientcredentialCLIENT_PRIVATE_KEY_PEM[];
extern const uint32_t clientcredentialCLIENT_CERTIFICATE_LENGTH;
extern const uint32_t clientcredentialCLIENT_PRIVATE_KEY_LENGTH;

#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
