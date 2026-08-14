#ifndef FABULOR_SSL_IO_H
#define FABULOR_SSL_IO_H

typedef struct
{
	int result;
	int ssl_error;
	int socket_error;
	unsigned long library_error;
} FabulorSslIoResult;

typedef enum
{
	FABULOR_SSL_READ_DATA,
	FABULOR_SSL_READ_RETRY,
	FABULOR_SSL_READ_RETRY_WRITE,
	FABULOR_SSL_READ_CLOSED,
	FABULOR_SSL_READ_FAILED
} FabulorSslReadDisposition;

FabulorSslReadDisposition fabulor_ssl_classify_read (
	const FabulorSslIoResult *result);

#endif
