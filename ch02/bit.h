#ifndef BIT_H
#define BIT_H

#include <cstdint>

#define GET_BIT(array, bit) \
  (array[(int)((bit) / 8)] & (0x80 >> ((bit) % 8)))

#define SET_BIT(array, bit) \
  (array[(int)((bit) / 8)] |= (0x80 >> ((bit) % 8)))

#define CLEAR_BIT(array, bit) \
  (array[(int)((bit) / 8)] &= ~(0x80 >> ((bit) % 8)))

typedef uint8_t byte;
typedef enum { LEFT, RIGHT } rotate_direction;

void xor_array(byte *target, const byte *src, int len_in_byte);
void permute(byte *target, const byte *src, const int *permute_table, int len_in_byte);
void rotate(byte *target, int start_bit, int end_bit, rotate_direction direction);
int get_bits(byte *target, int start_bit, int bits); // bits <= 8
void byte_swap(byte *target, int len);

#endif
