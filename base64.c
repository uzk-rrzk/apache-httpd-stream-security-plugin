//Decodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "httpd.h"

/**
 * Determine the length of the decoded message.
 */
size_t calc_decode_length(const char* b64input) { //Calculates the length of a decoded string
	size_t len = strlen(b64input),
	padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
	padding = 2;
	else if (b64input[len-1] == '=') //last char is =
	padding = 1;

	return (len*3)/4 - padding;
}

/**
 * Adds padding of "=" characters to end of Base 64 string if necessary.
 * @param p
 *      The pool to allocate memory to that will be cleaned upon end of session.
 * @param b64message
 *      The Base 64 encoded message.
 */
char *addPadding(apr_pool_t *p, char *b64message) {
    size_t decodeLen = calc_decode_length(b64message);
    size_t encodedLen = strlen(b64message);

    if (decodeLen % 3 == 0) {
        return b64message;
    }

    if (3 - (decodeLen % 3) == 1 && b64message[encodedLen - 1] == '=') {
        char *withOnePadding = (char*) apr_palloc(p, sizeof(char) * (encodedLen + 2));
        strncpy(withOnePadding, b64message, encodedLen);
        withOnePadding[encodedLen] = '=';
        withOnePadding[encodedLen + 1] = '\0';
        return withOnePadding;
    } else {
        char *withTwoPadding = (char*) apr_palloc(p, sizeof(char) * (encodedLen + 3));
        strncpy(withTwoPadding, b64message, encodedLen);
        withTwoPadding[encodedLen] = '=';
        withTwoPadding[encodedLen + 1] = '=';
        withTwoPadding[encodedLen + 2] = '\0';
        return withTwoPadding;
    }
}

/**
 * Decodes a string into plain text.
 * @param p
 *          The pool to allocate memory to that will be cleaned upon end of session.
 * @param b64message
 *          The Base 64 encoded message
 * @param buffer
 *          The pointer to assign the plain text result to.
 * @param length
 *          The length of the plain text result.
 */
int base_64_decode(apr_pool_t *p, char* b64message, uint8_t** buffer, size_t* length) { //Decodes a base64 encoded string
	BIO *bio, *b64;
	int decodeLen = calc_decode_length(b64message);
	*buffer = (uint8_t*)apr_palloc(p, decodeLen);

    char *withPadding = addPadding(p, b64message);

	bio = BIO_new_mem_buf(withPadding, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	*length = BIO_read(bio, *buffer, strlen(withPadding));
	assert(*length == decodeLen); //length should equal decodeLen, else something went horribly wrong
	BIO_free_all(bio);
	return (0); //success
}
