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

#include "utils.h"
#include "fish.h"

/**
 * Calculate the length of Base64-encoded string
 *
 * @param plaintext_len Size of clear text to encode
 * @return Size of encoded string
 */
size_t base64_len(size_t plaintext_len) {
    size_t blocks = plaintext_len / 3 + (plaintext_len % 3 != 0);

    if (blocks > G_MAXSIZE / 4)
        return 0;

    return blocks * 4;
}

/**
 * Calculate the length of BlowcryptBase64-encoded string
 *
 * @param plaintext_len Size of clear text to encode
 * @return Size of encoded string
 */
size_t base64_fish_len(size_t plaintext_len) {
    size_t blocks = plaintext_len / 8 + (plaintext_len % 8 != 0);

    if (blocks > G_MAXSIZE / 12)
        return 0;

    return blocks * 12;
}

/**
 * Calculate the length of fish-encrypted string in CBC mode
 *
 * @param plaintext_len  Size of clear text to encode
 * @return Size of encoded string
 */
size_t cbc_len(size_t plaintext_len) {
    size_t padded_len = plaintext_len;
    size_t remainder = plaintext_len % 8;

    if (remainder != 0)
    {
        if (padded_len > G_MAXSIZE - (8 - remainder))
            return 0;
        padded_len += 8 - remainder;
    }

    if (padded_len > G_MAXSIZE - 8)
        return 0;

    return base64_len(padded_len + 8);
}

/**
 * Calculate the length of fish-encrypted string in ECB mode
 *
 * @param plaintext_len  Size of clear text to encode
 * @return Size of encoded string
 */
size_t ecb_len(size_t plaintext_len) {
    return base64_fish_len(plaintext_len);
}

/**
 * Calculate the length of encrypted string in 'mode' mode
 *
 * @param plaintext_len Length of plaintext
 * @param mode          Encryption mode
 * @return Size of encoded string
 */
size_t encoded_len(size_t plaintext_len, enum fish_mode mode) {
    switch (mode) {

        case FISH_CBC_MODE:
            return cbc_len(plaintext_len);

        case FISH_ECB_MODE:
            return ecb_len(plaintext_len);
    }

    return 0;
}

/**
 * Determine the maximum length of plaintext for a 'max_len' limit taking care the overload of encryption
 *
 * @param max_len   Limit for plaintext
 * @param mode      Encryption mode
 * @return Maximum allowed plaintext length
 */
size_t max_text_command_len(size_t max_len, enum fish_mode mode) {
    size_t len = max_len;
    size_t encrypted_len;

    if (mode != FISH_CBC_MODE && mode != FISH_ECB_MODE)
        return 0;

    while (TRUE) {
        encrypted_len = encoded_len(len, mode);
        if (encrypted_len != 0 && encrypted_len <= max_len)
            return len;
        if (len == 0)
            return 0;
        len--;
    }
}

/**
 * Iterate over 'data' in chunks of 'max_chunk_len' taking care the UTF-8 characters
 *
 * @param data              Data to iterate
 * @param max_chunk_len     Size of biggest chunk
 * @param [out] chunk_len   Current chunk length
 * @return Pointer to current chunk position or NULL if not have more chunks
 */
const char *foreach_utf8_data_chunks(const char *data, size_t max_chunk_len, size_t *chunk_len) {
    size_t data_len, last_chunk_len = 0;
    const char *utf8_character;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (chunk_len != NULL, NULL);

    *chunk_len = 0;
    if (!*data || max_chunk_len == 0) {
        return NULL;
    }

    /* Last chunk of data */
    data_len = strlen(data);
    if (data_len <= max_chunk_len) {
        *chunk_len = data_len;
        return data;
    }

    utf8_character = data;

    /* Not valid UTF-8, but maybe valid text, just split into max length */
    if (!g_utf8_validate(data, -1, NULL)) {
        *chunk_len = max_chunk_len;
        return data;
    }

    while (*utf8_character) {
        size_t next_len = (size_t) (g_utf8_next_char(utf8_character) - data);

        if (next_len > max_chunk_len)
            break;
        last_chunk_len = next_len;
        utf8_character = g_utf8_next_char(utf8_character);
    }

    *chunk_len = last_chunk_len;
    return last_chunk_len != 0 ? data : NULL;
}
