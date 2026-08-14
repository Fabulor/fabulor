#include <glib.h>
#include <openssl/ssl.h>

#include "../ssl-io.h"

static void
test_ssl_read_dispositions (void)
{
	FabulorSslIoResult result = { 1, SSL_ERROR_NONE, 0, 0 };

	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_DATA);

	result.result = -1;
	result.ssl_error = SSL_ERROR_WANT_READ;
	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_RETRY);

	result.ssl_error = SSL_ERROR_WANT_WRITE;
	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_RETRY_WRITE);

	result.result = 0;
	result.ssl_error = SSL_ERROR_ZERO_RETURN;
	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_CLOSED);

	result.result = -1;
	result.ssl_error = SSL_ERROR_SYSCALL;
	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_FAILED);

	result.ssl_error = SSL_ERROR_SSL;
	g_assert_cmpint (fabulor_ssl_classify_read (&result), ==,
					 FABULOR_SSL_READ_FAILED);
	g_assert_cmpint (fabulor_ssl_classify_read (NULL), ==,
					 FABULOR_SSL_READ_FAILED);
}

void
ssl_io_register_tests (void)
{
	g_test_add_func ("/ssl-io/read-dispositions", test_ssl_read_dispositions);
}
