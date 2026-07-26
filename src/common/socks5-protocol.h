/* Copyright (C) 2026 Fabulor contributors */

#ifndef FABULOR_SOCKS5_PROTOCOL_H
#define FABULOR_SOCKS5_PROTOCOL_H

#include <stddef.h>
#include <string.h>

#define FABULOR_SOCKS5_VERSION 5
#define FABULOR_SOCKS5_AUTH_VERSION 1
#define FABULOR_SOCKS5_METHOD_NONE 0
#define FABULOR_SOCKS5_METHOD_USERNAME_PASSWORD 2
#define FABULOR_SOCKS5_METHOD_UNACCEPTABLE 255
#define FABULOR_SOCKS5_COMMAND_CONNECT 1
#define FABULOR_SOCKS5_ADDRESS_IPV4 1
#define FABULOR_SOCKS5_ADDRESS_DOMAIN 3
#define FABULOR_SOCKS5_ADDRESS_IPV6 4
#define FABULOR_SOCKS5_MAX_FIELD_LENGTH 255
#define FABULOR_SOCKS5_MAX_AUTH_REQUEST 513
#define FABULOR_SOCKS5_MAX_CONNECT_REQUEST 262

static inline int
fabulor_socks5_credentials_valid (int auth_required,
								  const char *username,
								  const char *password)
{
	size_t username_length;
	size_t password_length;

	if (!auth_required)
		return 1;
	if (!username || !password)
		return 0;

	username_length = strlen (username);
	password_length = strlen (password);
	return username_length > 0 &&
		username_length <= FABULOR_SOCKS5_MAX_FIELD_LENGTH &&
		password_length > 0 &&
		password_length <= FABULOR_SOCKS5_MAX_FIELD_LENGTH;
}

static inline size_t
fabulor_socks5_build_method_request (unsigned char *output,
									 size_t output_size,
									 int auth_required)
{
	if (!output || output_size < 3)
		return 0;

	output[0] = FABULOR_SOCKS5_VERSION;
	output[1] = 1;
	output[2] = auth_required ?
		FABULOR_SOCKS5_METHOD_USERNAME_PASSWORD :
		FABULOR_SOCKS5_METHOD_NONE;
	return 3;
}

static inline int
fabulor_socks5_method_response_valid (const unsigned char *response,
									  size_t response_size,
									  int auth_required)
{
	unsigned char required_method;

	if (!response || response_size != 2 ||
		response[0] != FABULOR_SOCKS5_VERSION)
		return 0;

	required_method = auth_required ?
		FABULOR_SOCKS5_METHOD_USERNAME_PASSWORD :
		FABULOR_SOCKS5_METHOD_NONE;
	return response[1] == required_method;
}

static inline size_t
fabulor_socks5_build_auth_request (unsigned char *output,
								   size_t output_size,
								   const char *username,
								   const char *password)
{
	size_t username_length;
	size_t password_length;
	size_t request_size;

	if (!fabulor_socks5_credentials_valid (1, username, password))
		return 0;

	username_length = strlen (username);
	password_length = strlen (password);
	request_size = 3 + username_length + password_length;
	if (!output || output_size < request_size)
		return 0;

	output[0] = FABULOR_SOCKS5_AUTH_VERSION;
	output[1] = (unsigned char) username_length;
	memcpy (output + 2, username, username_length);
	output[2 + username_length] = (unsigned char) password_length;
	memcpy (output + 3 + username_length, password, password_length);
	return request_size;
}

static inline int
fabulor_socks5_auth_response_valid (const unsigned char *response,
									size_t response_size)
{
	return response && response_size == 2 &&
		response[0] == FABULOR_SOCKS5_AUTH_VERSION &&
		response[1] == 0;
}

static inline size_t
fabulor_socks5_build_domain_connect_request (unsigned char *output,
											 size_t output_size,
											 const char *hostname,
											 unsigned int port)
{
	size_t hostname_length;
	size_t request_size;

	if (!hostname || port == 0 || port > 65535)
		return 0;

	hostname_length = strlen (hostname);
	if (hostname_length == 0 ||
		hostname_length > FABULOR_SOCKS5_MAX_FIELD_LENGTH)
		return 0;

	request_size = 7 + hostname_length;
	if (!output || output_size < request_size)
		return 0;

	output[0] = FABULOR_SOCKS5_VERSION;
	output[1] = FABULOR_SOCKS5_COMMAND_CONNECT;
	output[2] = 0;
	output[3] = FABULOR_SOCKS5_ADDRESS_DOMAIN;
	output[4] = (unsigned char) hostname_length;
	memcpy (output + 5, hostname, hostname_length);
	output[5 + hostname_length] = (unsigned char) (port >> 8);
	output[6 + hostname_length] = (unsigned char) port;
	return request_size;
}

static inline size_t
fabulor_socks5_build_ipv4_connect_request (unsigned char *output,
										   size_t output_size,
										   unsigned int address,
										   unsigned int port)
{
	if (!output || output_size < 10 || port == 0 || port > 65535)
		return 0;

	output[0] = FABULOR_SOCKS5_VERSION;
	output[1] = FABULOR_SOCKS5_COMMAND_CONNECT;
	output[2] = 0;
	output[3] = FABULOR_SOCKS5_ADDRESS_IPV4;
	output[4] = (unsigned char) (address >> 24);
	output[5] = (unsigned char) (address >> 16);
	output[6] = (unsigned char) (address >> 8);
	output[7] = (unsigned char) address;
	output[8] = (unsigned char) (port >> 8);
	output[9] = (unsigned char) port;
	return 10;
}

static inline int
fabulor_socks5_reply_header_valid (const unsigned char *response,
								   size_t response_size)
{
	if (!response || response_size != 4 ||
		response[0] != FABULOR_SOCKS5_VERSION ||
		response[2] != 0)
		return 0;

	return response[3] == FABULOR_SOCKS5_ADDRESS_IPV4 ||
		response[3] == FABULOR_SOCKS5_ADDRESS_DOMAIN ||
		response[3] == FABULOR_SOCKS5_ADDRESS_IPV6;
}

static inline size_t
fabulor_socks5_fixed_reply_tail_size (unsigned char address_type)
{
	switch (address_type)
	{
	case FABULOR_SOCKS5_ADDRESS_IPV4:
		return 6;
	case FABULOR_SOCKS5_ADDRESS_IPV6:
		return 18;
	default:
		return 0;
	}
}

static inline int
fabulor_socks5_domain_length_valid (unsigned char domain_length)
{
	return domain_length > 0;
}

#endif
