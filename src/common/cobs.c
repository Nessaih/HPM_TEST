
/*COBS Consistent Overhead Byte Stuffing */

/*
 * cobs_encode byte stuffs "length" bytes of data
 * at the location pointed to by "ptr", writing
 * the output to the location pointed to by "dst".
 *
 * Returns the length of the encoded data.
 */

#define start_block()  (code_ptr = dst++, code = 1)
#define finish_block() (*code_ptr = code)

unsigned int cobs_encode(const unsigned char *src, unsigned int len, unsigned char *dst)
{
    const unsigned char *start = dst, *end = src + len;
    unsigned char        c, code, *code_ptr;

    start_block();

    while(src < end) {
        if(code != 0xFF) {
            c = *src++;
            if(c != 0) {
                *dst++ = c;
                code++;
                continue;
            }
        }
        finish_block();
        start_block();
    }
    finish_block();
    return dst - start;
}

/*
 * cobs_decode decodes "length" bytes of data at
 * the location pointed to by "ptr", writing the
 * output to the location pointed to by "dst".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 */
unsigned int cobs_decode(const unsigned char *src, unsigned int len, unsigned char *dst)
{
    const unsigned char *start = dst, *end = src + len;
    unsigned char        code = 0xFF, copy = 0;

    for(; src < end; copy--) {
        if(copy != 0) {
            *dst++ = *src++;
        } else {
            if(code != 0xFF) {
                *dst++ = 0;
            }

            copy = code = *src++;

            if(code == 0) {
                break; /* Source len too long */
            }
        }
    }
    return dst - start;
}
