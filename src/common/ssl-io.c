#include <openssl/ssl.h>

#include "ssl-io.h"

FabulorSslReadDisposition
fabulor_ssl_classify_read (const FabulorSslIoResult *result)
{
	if (!result)
		return FABULOR_SSL_READ_FAILED;
	if (result->result > 0)
		return FABULOR_SSL_READ_DATA;

	switch (result->ssl_error)
	{
	case SSL_ERROR_WANT_READ:
		return FABULOR_SSL_READ_RETRY;
	case SSL_ERROR_WANT_WRITE:
		return FABULOR_SSL_READ_RETRY_WRITE;
	case SSL_ERROR_ZERO_RETURN:
		return FABULOR_SSL_READ_CLOSED;
	default:
		return FABULOR_SSL_READ_FAILED;
	}
}
