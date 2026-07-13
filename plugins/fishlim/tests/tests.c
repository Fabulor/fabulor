/*
  Copyright (c) 2020 <bakasura@protonmail.ch>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/

#include <string.h>
#include <limits.h>
#include <glib.h>

#include "fish.h"
#include "dh1080.h"
#include "utils.h"

/**
 * Auxiliary function: Generate a random string
 * @param out Preallocated string to fill
 * @param len Size of bytes to fill
 */
static void
random_string(char *out, size_t len)
{
    GRand *rand = NULL;
    size_t i = 0;

    rand = g_rand_new();
    for (i = 0; i < len; ++i) {
        out[i] = (char) g_rand_int_range(rand, 1, 256);
    }

    out[len] = 0;

    g_rand_free(rand);
}

/**
 * Check encrypt and decrypt in ECB mode
 */
static void
test_ecb(void)
{
    char *b64 = NULL;
    char *de = NULL;
    size_t key_len, message_len = 0;
    char key[57];
    char message[1000];

    /* Generate key 32–448 bits (Yes, I start with 8 bits) */
    for (key_len = 1; key_len < 57; ++key_len) {

        random_string(key, key_len);

        for (message_len = 1; message_len < 1000; ++message_len) {
            random_string(message, message_len);

            /* Encrypt */
            b64 = fish_encrypt(key, key_len, message, message_len, FISH_ECB_MODE);
            g_assert_nonnull(b64);

            /* Decrypt */
            /* Linear */
            de = fish_decrypt_str(key, key_len, b64, FISH_ECB_MODE);
			g_assert_cmpstr (de, ==, message);
            g_free(de);
	
            /* Mixed */
            de = fish_decrypt_str(key, key_len, b64, FISH_ECB_MODE);
			g_assert_cmpstr (de, ==, message);
            g_free(de);

            g_free(b64);
        }
    }
}

/**
 * Check encrypt and decrypt in CBC mode
 */
static void
test_cbc(void)
{
    char *b64 = NULL;
    char *de = NULL;
    size_t key_len, message_len = 0;
    char key[57];
    char message[1000];

    /* Generate key 32–448 bits (Yes, I start with 8 bits) */
    for (key_len = 1; key_len < 57; ++key_len) {

        random_string(key, key_len);

        for (message_len = 1; message_len < 1000; ++message_len) {
            random_string(message, message_len);

            /* Encrypt */
            b64 = fish_encrypt(key, key_len, message, message_len, FISH_CBC_MODE);
            g_assert_nonnull(b64);

            /* Decrypt */
            /* Linear */
            de = fish_decrypt_str(key, key_len, b64, FISH_CBC_MODE);
            g_assert_cmpstr (de, ==, message);
            g_free(de);

            g_free(b64);
        }
    }
}

/**
 * Check the calculation of final length from an encoded string in Base64
 */
static void
test_base64_len (void)
{
    char *b64 = NULL;
    char message[1000];
    size_t message_end = sizeof (message) - 1;

    random_string(message, message_end);

    while (TRUE) {
        message[message_end] = '\0'; /* Truncate instead of generating new strings */
        b64 = g_base64_encode((const unsigned char *) message, message_end);
        g_assert_nonnull(b64);
        g_assert_cmpuint(strlen(b64), == , base64_len(message_end));
        g_free(b64);
        if (message_end == 0)
            break;
        message_end--;
    }
}

/**
 * Check the calculation of final length from an encoded string in BlowcryptBase64
 */
static void
test_base64_fish_len (void)
{
    char *b64 = NULL;
    size_t message_len = 0;
    char message[1000];

    for (message_len = 1; message_len < 1000; ++message_len) {
        random_string(message, message_len);
        b64 = fish_base64_encode(message, message_len);
        g_assert_nonnull(b64);
        g_assert_cmpuint(strlen(b64), == , base64_fish_len(message_len));
        g_free(b64);
    }
}

/**
 * Check the calculation of final length from an encrypted string in ECB mode
 */
static void
test_base64_ecb_len(void)
{
    char *b64 = NULL;
    size_t key_len, message_len = 0;
    char key[57];
    char message[1000];

    /* Generate key 32–448 bits (Yes, I start with 8 bits) */
    for (key_len = 1; key_len < 57; ++key_len) {

        random_string(key, key_len);

        for (message_len = 1; message_len < 1000; ++message_len) {
            random_string(message, message_len);
            b64 = fish_encrypt(key, key_len, message, message_len, FISH_ECB_MODE);
            g_assert_nonnull(b64);
            g_assert_cmpuint(strlen(b64), == , ecb_len(message_len));
            g_free(b64);
        }
    }
}

/**
 * Check the calculation of final length from an encrypted string in CBC mode
 */
static void
test_base64_cbc_len(void)
{
    char *b64 = NULL;
    size_t key_len, message_len = 0;
    char key[57];
    char message[1000];

    /* Generate key 32–448 bits (Yes, I start with 8 bits) */
    for (key_len = 1; key_len < 57; ++key_len) {

        random_string(key, key_len);

        for (message_len = 1; message_len < 1000; ++message_len) {
            random_string(message, message_len);
            b64 = fish_encrypt(key, key_len, message, message_len, FISH_CBC_MODE);
            g_assert_nonnull(b64);
            g_assert_cmpuint(strlen(b64), == , cbc_len(message_len));
            g_free(b64);
        }
    }
}

/**
 * Check the calculation of length limit for a plaintext in each encryption mode
 */
static void
test_max_text_command_len(void)
{
    size_t plaintext_len;
    size_t max_encoded_len;
    enum fish_mode mode;

    for (max_encoded_len = 0; max_encoded_len < 10000; ++max_encoded_len) {
        for (mode = FISH_ECB_MODE; mode <= FISH_CBC_MODE; ++mode) {
            plaintext_len = max_text_command_len(max_encoded_len, mode);
            if (plaintext_len == 0)
                g_assert_cmpuint(max_encoded_len, <, encoded_len(1, mode));
            else
                g_assert_cmpuint(encoded_len(plaintext_len, mode), <= , max_encoded_len);
        }
    }
}

/**
 * Check the calculation of length limit for a plaintext in each encryption mode
 */
static void
test_foreach_utf8_data_chunks(void)
{
    GRand *rand = NULL;
    GString *chunks = NULL;
    size_t max_chunks_len, chunks_len;
    char ascii_message[1001];
    char *data_chunk = NULL;

    rand = g_rand_new();
    max_chunks_len = (size_t) g_rand_int_range(rand, 2, 301);
    random_string(ascii_message, 1000);

    data_chunk = ascii_message;

    chunks = g_string_new(NULL);

    while (foreach_utf8_data_chunks(data_chunk, max_chunks_len, &chunks_len)) {
        g_string_append_len(chunks, data_chunk, (gssize) chunks_len);
        /* Next chunk */
        data_chunk += chunks_len;
    }
    /* Check data loss */
    g_assert_cmpstr(chunks->str, == , ascii_message);

    g_string_free(chunks, TRUE);
    g_rand_free (rand);
}

static void
test_length_boundaries(void)
{
    const char byte = 'x';
    char *encoded;
    size_t chunk_len = 99;

    g_assert_cmpuint(base64_len(G_MAXSIZE), ==, 0);
    g_assert_cmpuint(base64_fish_len(G_MAXSIZE), ==, 0);
    g_assert_cmpuint(cbc_len(G_MAXSIZE), ==, 0);
    g_assert_cmpuint(max_text_command_len(0, FISH_ECB_MODE), ==, 0);
    g_assert_null(foreach_utf8_data_chunks("text", 0, &chunk_len));
    g_assert_cmpuint(chunk_len, ==, 0);
    g_assert_null(foreach_utf8_data_chunks("\342\202\254", 2, &chunk_len));
    g_assert_cmpuint(chunk_len, ==, 0);

    encoded = fish_base64_encode(&byte, 1);
    g_assert_nonnull(encoded);
    g_assert_cmpuint(strlen(encoded), ==, 12);
    g_free(encoded);

    g_assert_null(fish_encrypt("key", (size_t) INT_MAX + 1,
                               &byte, 1, FISH_ECB_MODE));
}

static void
test_command_boundaries(void)
{
    const size_t command_len = 20;
    char message[1201];
    enum fish_mode mode;
    GSList *encrypted;
    GSList *item;
    GString *decrypted = g_string_new(NULL);

    memset(message, 'x', sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';

    g_assert_null(fish_encrypt_for_nick("nick", message, &mode, 510));

    encrypted = fish_encrypt_for_nick("nick", message, &mode, command_len);
    g_assert_nonnull(encrypted);
    g_assert_cmpint(mode, ==, FISH_CBC_MODE);

    for (item = encrypted; item != NULL; item = item->next) {
        const char *line = item->data;
        char *plain;

        g_assert_cmpint(line[0], ==, '*');
        g_assert_cmpuint(command_len + strlen(line), <=, 510);
        plain = fish_decrypt_str("test-key", strlen("test-key"), line + 1, mode);
        g_assert_nonnull(plain);
        g_string_append(decrypted, plain);
        g_free(plain);
    }

    g_assert_cmpstr(decrypted->str, ==, message);
    g_string_free(decrypted, TRUE);
    g_slist_free_full(encrypted, g_free);
}

static void
test_dh1080_boundaries(void)
{
    char *private_a = NULL;
    char *public_a = NULL;
    char *private_b = NULL;
    char *public_b = NULL;
    char *secret_a = NULL;
    char *secret_b = NULL;
    char oversized_key[183];

    g_assert_true(dh1080_init());
    g_assert_true(dh1080_generate_key(&private_a, &public_a));
    g_assert_true(dh1080_generate_key(&private_b, &public_b));
    g_assert_true(dh1080_compute_key(private_a, public_b, &secret_a));
    g_assert_true(dh1080_compute_key(private_b, public_a, &secret_b));
    g_assert_cmpstr(secret_a, ==, secret_b);

    g_clear_pointer(&secret_a, g_free);
    g_assert_false(dh1080_compute_key(private_a, "not-base64!", &secret_a));
    g_assert_null(secret_a);

    memset(oversized_key, 'A', sizeof(oversized_key) - 1);
    oversized_key[sizeof(oversized_key) - 1] = '\0';
    g_assert_false(dh1080_compute_key(private_a, oversized_key, &secret_a));
    g_assert_null(secret_a);

    dh1080_deinit();
    g_free(private_a);
    g_free(public_a);
    g_free(private_b);
    g_free(public_b);
    g_free(secret_b);
}

int
main(int argc, char *argv[]) {

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/fishlim/ecb", test_ecb);
    g_test_add_func("/fishlim/cbc", test_cbc);
    g_test_add_func("/fishlim/base64_len", test_base64_len);
    g_test_add_func("/fishlim/base64_fish_len", test_base64_fish_len);
    g_test_add_func("/fishlim/base64_ecb_len", test_base64_ecb_len);
    g_test_add_func("/fishlim/base64_cbc_len", test_base64_cbc_len);
    g_test_add_func("/fishlim/max_text_command_len", test_max_text_command_len);
    g_test_add_func("/fishlim/foreach_utf8_data_chunks", test_foreach_utf8_data_chunks);
    g_test_add_func("/fishlim/length_boundaries", test_length_boundaries);
    g_test_add_func("/fishlim/command_boundaries", test_command_boundaries);
    g_test_add_func("/fishlim/dh1080_boundaries", test_dh1080_boundaries);

    fish_init();
    int ret = g_test_run();
    fish_deinit();
    return ret;
}
