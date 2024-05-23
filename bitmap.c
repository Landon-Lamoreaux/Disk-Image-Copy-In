// Warning!  This code has not been thoroughly tested

#include "bitmap.h"

/* set a specific bit within a bitmap_t */
void set_bit_unit(bitmap_t *word, uint32_t bit)
{
  bitmap_t mask = 1;
  mask <<= bit; 
  *word |= mask;
}

/* clear a specific bit within a bitmap_t */
void clear_bit_unit(bitmap_t *word, uint32_t bit)
{
  bitmap_t mask = 1;
  mask <<= bit; 
  *word &= ~mask;
}

/* get the value of a specific bit within a bitmap_t */
uint32_t get_bit_unit(bitmap_t *word, uint32_t bit)
{
  bitmap_t mask = 1;
  mask <<= bit; 
  return ((*word & mask)>>bit);
}

/* set a bit in the bitmap */
void set_bit(bitmap_t *bitmap, uint32_t bit)
{
  /* find out which element the bit is in */
  uint32_t w = (bit>>BITMAP_SHIFT);
  /* find out which bit within the element */
  uint32_t b = (bit&((1<<BITMAP_SHIFT)-1));
  /* set the correct bit within that word */
  set_bit_unit(bitmap+w,b);
}

/* clear a bit in the bitmap */
void clear_bit(bitmap_t *bitmap, uint32_t bit)
{
  /* find out which word the bit is in */
  uint32_t w = (bit>>BITMAP_SHIFT);
  uint32_t b = (bit&((1<<BITMAP_SHIFT)-1));
  /* clear the correct bit within that word */
  clear_bit_unit(bitmap+w,b);
}

/* get the value of a specific bit within a uint32_t */
uint32_t get_bit(bitmap_t *bitmap, uint32_t bit)
{
  /* find out which word the bit is in */
  uint32_t w = (bit>>BITMAP_SHIFT);
  uint32_t b = (bit&((1<<BITMAP_SHIFT)-1));
  /* return the correct bit within that word */
  return get_bit_unit(bitmap+w,b);
}

/* get the index of the first zero bit in the bitmap */
int32_t first_cleared(bitmap_t *bitmap, uint32_t size)
{
  int i=0, j=0;
  // Find a unit that is not FFF..F
  while((i<(size/sizeof(bitmap_t)))&&(bitmap[i]==-1))
    i++;
  if(i >= (size/sizeof(bitmap_t)))
    return -1;
  // Found a unit with at least one zero in it.  Find the zero bit.
  while((j<(8*sizeof(bitmap_t)))&&get_bit_unit(bitmap+i,j))
    j++;
  // calculate the bit position in the bitmap
  i = i*sizeof(bitmap_t)*8 + j;
  if(i >= size)
    return -1;
  return i;
}

