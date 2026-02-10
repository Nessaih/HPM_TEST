#ifndef TBOX_BITS_H
#define TBOX_BITS_H

#ifdef __cplusplus
extern "C" {
#endif

#define BITS_BYTEINDEX(bits)                ((bits) >> 3U)
#define BITS_BYTENUM(bits)                  (((bits) >> 3U) + 1U)
#define BYTES_BITNUM(bytes)                 ((bytes) << 3U)
#define BITS_SET(value, bits)               ((value) |= ((1U << (bits))))
#define BITS_CLEAR(value, bits)             ((value) &= ~((1U << (bits))))
#define BITS_IS_SET(value, bits)            (((value) & (1U << (bits))) != 0U)
#define BITS_IS_CLEAR(value, bits)          (((value) & (1U << (bits)))  == 0U)
#define BITARRAY_SET(value, bits)           ((value)[BITS_BYTEINDEX(bits)] |= (1U << (bits) % 8U))
#define BITARRAY_CLEAR(value, bits)         ((value)[BITS_BYTEINDEX(bits)] &= ~(1U << (bits) % 8U))
#define BITARRAY_IS_SET(value, bits)        (((value)[BITS_BYTEINDEX(bits)] & (1U << (bits) % 8U)) != 0U)
#define BITARRAY_IS_CLEAR(value, bits)      (((value)[BITS_BYTEINDEX(bits)] & (1U << (bits) % 8U))  == 0U)

#ifdef __cplusplus
}
#endif

#endif /* TBOX_BITS_H */