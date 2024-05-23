#ifndef BITMAP_H
#define BITMAP_H
#include <stdint.h>

/* bitmap_t, BITMAP_BITS and BITMAP_SHIFT should really be set to the natural
   word size of the architecture.  For most architectures, the following is good. */
typedef uint32_t bitmap_t;
#define BITMAP_BITS (sizeof(bitmap_t)*8)
#define BITMAP_SHIFT 5

/* set a bit in the bitmap */
void set_bit(bitmap_t *bitmap, uint32_t bit);

/* clear a bit in the bitmap */
void clear_bit(bitmap_t *bitmap, uint32_t bit);

/* get the value of a specific bit in a bitmap */
uint32_t get_bit(bitmap_t *bitmap, uint32_t bit);

/* get the index of the first zero bit in the bitmap */
int32_t first_cleared(bitmap_t *bitmap, uint32_t size);


#endif
