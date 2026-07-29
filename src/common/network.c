/* X-Chat
 * Copyright (C) 2001 Peter Zelezny.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* ipv4 and ipv6 networking functions with a common interface */
#define _POSIX_C_SOURCE 200112L
#ifdef _WIN32
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif

#include "config.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifndef WIN32
#include <unistd.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#endif

#define WANTSOCKET
#define WANTARPA
#define WANTDNS
#include "inet.h"

#define NETWORK_PRIVATE
#include "network.h"
#include "zoitechat.h"

extern struct zoitechatprefs prefs;

#define RAND_INT(n) ((int)(rand() / (RAND_MAX + 1.0) * (n)))


/* ================== COMMON ================= */

static void
net_set_socket_options (int sok)
{
	socklen_t sw;

	sw = 1;
	setsockopt (sok, SOL_SOCKET, SO_REUSEADDR, (char *) &sw, sizeof (sw));
	sw = 1;
	setsockopt (sok, SOL_SOCKET, SO_KEEPALIVE, (char *) &sw, sizeof (sw));
#ifdef TCP_KEEPIDLE
	{
		int keepidle = prefs.hex_net_keepalive_idle;
		if (keepidle > 0)
			setsockopt (sok, IPPROTO_TCP, TCP_KEEPIDLE, (char *) &keepidle, sizeof (keepidle));
	}
#endif
#ifdef TCP_KEEPINTVL
	{
		int keepintvl = prefs.hex_net_keepalive_interval;
		if (keepintvl > 0)
			setsockopt (sok, IPPROTO_TCP, TCP_KEEPINTVL, (char *) &keepintvl, sizeof (keepintvl));
	}
#endif
#ifdef TCP_KEEPCNT
	{
		int keepcnt = prefs.hex_net_keepalive_count;
		if (keepcnt > 0)
			setsockopt (sok, IPPROTO_TCP, TCP_KEEPCNT, (char *) &keepcnt, sizeof (keepcnt));
	}
#endif
}

char *
net_ip (uint32_t addr)
{
	static char buf[INET_ADDRSTRLEN];
	struct in_addr ia;

	ia.s_addr = htonl (addr);
	if (!inet_ntop (AF_INET, &ia, buf, sizeof (buf)))
		buf[0] = 0;

	return buf;
}

int
net_parse_ipv4 (const char *hostname, uint32_t *addr)
{
	struct in_addr ia;

	if (inet_pton (AF_INET, hostname, &ia) != 1)
		return FALSE;

	*addr = ia.s_addr;
	return TRUE;
}

int
net_lookup_ipv4 (const char *hostname, uint32_t *addr)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *sin;
	int ret;

	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	ret = getaddrinfo (hostname, NULL, &hints, &res);
	if (ret != 0)
		return FALSE;

	sin = (struct sockaddr_in *) res->ai_addr;
	*addr = sin->sin_addr.s_addr;
	freeaddrinfo (res);

	return TRUE;
}

void
net_store_destroy (netstore * ns)
{
	if (ns->ip6_hostent)
		freeaddrinfo (ns->ip6_hostent);
	free (ns);
}

netstore *
net_store_new (void)
{
	return calloc (1, sizeof (netstore));
}

/* =================== IPV6 ================== */

char *
net_resolve (netstore * ns, char *hostname, int port, char **real_host)
{
	struct addrinfo hints;
	char ipstring[MAX_HOSTNAME];
	char portstring[MAX_HOSTNAME];
	int ret;

/*	if (ns->ip6_hostent)
		freeaddrinfo (ns->ip6_hostent);*/

	sprintf (portstring, "%d", port);

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = PF_UNSPEC; /* support ipv6 and ipv4 */
	hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	if (port == 0)
		ret = getaddrinfo (hostname, NULL, &hints, &ns->ip6_hostent);
	else
		ret = getaddrinfo (hostname, portstring, &hints, &ns->ip6_hostent);
	if (ret != 0)
		return NULL;

#ifdef LOOKUPD	/* See note about lookupd above the IPv4 version of net_resolve. */
	struct addrinfo *tmp;
	int count = 0;

	for (tmp = ns->ip6_hostent; tmp; tmp = tmp->ai_next)
		count ++;

	count = RAND_INT(count);
	
	while (count--) ns->ip6_hostent = ns->ip6_hostent->ai_next;
#endif

	/* find the numeric IP number */
	ipstring[0] = 0;
	getnameinfo (ns->ip6_hostent->ai_addr,
					 (socklen_t)ns->ip6_hostent->ai_addrlen,
					 ipstring, sizeof (ipstring), NULL, 0, NI_NUMERICHOST);

	if (ns->ip6_hostent->ai_canonname)
		*real_host = g_strdup(ns->ip6_hostent->ai_canonname);
	else
		*real_host = g_strdup(hostname);

	return g_strdup(ipstring);
}

/* the only thing making this interface unclean, this shitty sok4, sok6 business */

#ifdef WIN32

#define NET_CONNECT_ATTEMPT_TIMEOUT_MS 3000

typedef struct
{
	struct sockaddr_storage address;
	int length;
	gboolean active;
} net_bind_address;

static void
net_capture_bind_address (int sok, int family, net_bind_address *bound)
{
	int length = sizeof (bound->address);

	memset (bound, 0, sizeof (*bound));
	if (sok == -1 ||
		 getsockname ((SOCKET)sok, (struct sockaddr *)&bound->address,
						  &length) != 0)
		return;

	if (family == AF_INET)
	{
		struct sockaddr_in *address =
			(struct sockaddr_in *)&bound->address;

		if (address->sin_addr.s_addr == htonl (INADDR_ANY))
			return;
		address->sin_port = 0;
	}
	else if (family == AF_INET6)
	{
		struct sockaddr_in6 *address =
			(struct sockaddr_in6 *)&bound->address;

		if (IN6_IS_ADDR_UNSPECIFIED (&address->sin6_addr))
			return;
		address->sin6_port = 0;
	}
	else
		return;

	bound->length = length;
	bound->active = TRUE;
}

static int
net_replace_socket (int *sok, const struct addrinfo *address,
						  const net_bind_address *bound)
{
	if (*sok != -1)
		closesocket ((SOCKET)*sok);

	*sok = zc_socket_create (address->ai_family, address->ai_socktype,
								  address->ai_protocol);
	if (*sok == -1)
		return -1;

	net_set_socket_options (*sok);
	if (bound->active &&
		 bind ((SOCKET)*sok, (const struct sockaddr *)&bound->address,
				 bound->length) != 0)
	{
		int error = WSAGetLastError ();

		closesocket ((SOCKET)*sok);
		*sok = -1;
		WSASetLastError (error);
		return -1;
	}

	return *sok;
}

static int
net_connect_with_timeout (int sok, const struct addrinfo *address)
{
	fd_set write_set;
	fd_set error_set;
	struct timeval timeout;
	int error;
	int error_length = sizeof (error);
	int selected;

	set_nonblocking (sok);
	if (connect ((SOCKET)sok, address->ai_addr,
					(int)address->ai_addrlen) == 0)
	{
		set_blocking (sok);
		return 0;
	}

	error = WSAGetLastError ();
	if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
		return error;

	FD_ZERO (&write_set);
	FD_ZERO (&error_set);
	FD_SET ((SOCKET)sok, &write_set);
	FD_SET ((SOCKET)sok, &error_set);
	timeout.tv_sec = NET_CONNECT_ATTEMPT_TIMEOUT_MS / 1000;
	timeout.tv_usec = (NET_CONNECT_ATTEMPT_TIMEOUT_MS % 1000) * 1000;

	selected = select (0, NULL, &write_set, &error_set, &timeout);
	if (selected == 0)
		return WSAETIMEDOUT;
	if (selected == SOCKET_ERROR)
		return WSAGetLastError ();

	if (getsockopt ((SOCKET)sok, SOL_SOCKET, SO_ERROR, (char *)&error,
						&error_length) != 0)
		return WSAGetLastError ();
	if (error != 0)
		return error;

	set_blocking (sok);
	return 0;
}

#endif

int
net_connect (netstore * ns, int *sok4, int *sok6, int *sok_return)
{
	struct addrinfo *res;
	int error = -1;

#ifdef WIN32
	net_bind_address bound4;
	net_bind_address bound6;
	gboolean attempted4 = FALSE;
	gboolean attempted6 = FALSE;

	net_capture_bind_address (*sok4, AF_INET, &bound4);
	net_capture_bind_address (*sok6, AF_INET6, &bound6);
#endif

	for (res = ns->ip6_hostent; res; res = res->ai_next)
	{
		int *family_socket;

		switch (res->ai_family)
		{
		case AF_INET:
			family_socket = sok4;
			break;
		case AF_INET6:
			family_socket = sok6;
			break;
		default:
			continue;
		}

#ifdef WIN32
		{
			gboolean *attempted =
				res->ai_family == AF_INET ? &attempted4 : &attempted6;
			const net_bind_address *bound =
				res->ai_family == AF_INET ? &bound4 : &bound6;

			if ((*attempted || *family_socket == -1) &&
				net_replace_socket (family_socket, res, bound) == -1)
			{
				error = WSAGetLastError ();
				continue;
			}
			*attempted = TRUE;
			if (*family_socket == -1)
			{
				error = WSAENOTSOCK;
				continue;
			}

			error = net_connect_with_timeout (*family_socket, res);
			*sok_return = *family_socket;
			if (error == 0)
				return 0;
		}
#else
		error = connect (*family_socket, res->ai_addr, res->ai_addrlen);
		*sok_return = *family_socket;
		if (error == 0)
			return 0;
#endif
	}

#ifdef WIN32
	WSASetLastError (error);
#endif
	return error;
}

void
net_bind (netstore * tobindto, int sok4, int sok6)
{
	bind (sok4, tobindto->ip6_hostent->ai_addr,
			(socklen_t)tobindto->ip6_hostent->ai_addrlen);
	bind (sok6, tobindto->ip6_hostent->ai_addr,
			(socklen_t)tobindto->ip6_hostent->ai_addrlen);
}

void
net_sockets (int *sok4, int *sok6)
{
	*sok4 = zc_socket_create (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	*sok6 = zc_socket_create (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (*sok4 != -1)
		net_set_socket_options (*sok4);
	if (*sok6 != -1)
		net_set_socket_options (*sok6);
}

void
udp_sockets (int *sok4, int *sok6)
{
	*sok4 = zc_socket_create (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	*sok6 = zc_socket_create (AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
}
