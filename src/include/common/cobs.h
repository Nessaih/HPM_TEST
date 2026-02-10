#ifndef __COBS_H__
#define __COBS_H__

#ifdef __cpluscplus
extern "C" {
#endif

unsigned int cobs_encode(const unsigned char *src, unsigned int len, unsigned char *dst);
unsigned int cobs_decode(const unsigned char *src, unsigned int len, unsigned char *dst);

#ifdef __cpluscplus
}
#endif

#endif //__COBS_H__