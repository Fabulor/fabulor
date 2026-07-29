/* Copyright (C) 2026 Fabulor contributors */

#ifndef FABULOR_PROXY_POLICY_H
#define FABULOR_PROXY_POLICY_H

typedef enum
{
	FABULOR_PROXY_DISABLED = 0,
	FABULOR_PROXY_RETIRED_WINGATE = 1,
	FABULOR_PROXY_SOCKS4 = 2,
	FABULOR_PROXY_SOCKS5 = 3,
	FABULOR_PROXY_HTTP = 4,
	FABULOR_PROXY_AUTO = 5
} FabulorProxyType;

static inline int
fabulor_proxy_type_normalize (int proxy_type)
{
	switch (proxy_type)
	{
	case FABULOR_PROXY_SOCKS4:
	case FABULOR_PROXY_SOCKS5:
	case FABULOR_PROXY_HTTP:
	case FABULOR_PROXY_AUTO:
		return proxy_type;
	default:
		return FABULOR_PROXY_DISABLED;
	}
}

static inline int
fabulor_proxy_type_from_menu_index (int menu_index)
{
	switch (menu_index)
	{
	case 1:
		return FABULOR_PROXY_SOCKS4;
	case 2:
		return FABULOR_PROXY_SOCKS5;
	case 3:
		return FABULOR_PROXY_HTTP;
	case 4:
		return FABULOR_PROXY_AUTO;
	default:
		return FABULOR_PROXY_DISABLED;
	}
}

static inline int
fabulor_proxy_type_to_menu_index (int proxy_type)
{
	switch (fabulor_proxy_type_normalize (proxy_type))
	{
	case FABULOR_PROXY_SOCKS4:
		return 1;
	case FABULOR_PROXY_SOCKS5:
		return 2;
	case FABULOR_PROXY_HTTP:
		return 3;
	case FABULOR_PROXY_AUTO:
		return 4;
	default:
		return 0;
	}
}

static inline int
fabulor_proxy_type_supports_auth (int proxy_type)
{
	proxy_type = fabulor_proxy_type_normalize (proxy_type);
	return proxy_type == FABULOR_PROXY_SOCKS5 ||
		proxy_type == FABULOR_PROXY_HTTP ||
		proxy_type == FABULOR_PROXY_AUTO;
}

static inline int
fabulor_proxy_type_uses_dcc_proxy (int proxy_type)
{
	proxy_type = fabulor_proxy_type_normalize (proxy_type);
	return proxy_type >= FABULOR_PROXY_SOCKS4 &&
		proxy_type <= FABULOR_PROXY_HTTP;
}

#endif
