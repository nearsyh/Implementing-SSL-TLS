#include "bit.h"
#include <memory>
#include <iostream>

void xor_array(byte* target, const byte* src, int len) {
  while(len --)
    target[len] ^= src[len];
}

void permute(byte *target, const byte *src, const int *permute_table, int len) {
  for(int i = 0; i < len * 8; i ++) {
    if(GET_BIT(src, permute_table[i] - 1) > 0) SET_BIT(target, i);
    else CLEAR_BIT(target, i);
    int temp = permute_table[i] - 1;
  }
}

void rotate_left(byte *target, int start_bit, int end_bit) {
  int carry = GET_BIT(target, start_bit), start_index = start_bit / 8, start_offset = start_bit % 8;
  int end_index = end_bit / 8, end_offset = end_bit % 8;
  if(start_index == end_index) {
    for(int i = start_bit; i < end_bit; i ++)
      if(GET_BIT(target, i + 1)) SET_BIT(target, i);
      else CLEAR_BIT(target, i);
  } else {
    for(int i = 0; i < 8 - start_offset; i ++)
      if(GET_BIT(target, start_bit + i + 1)) SET_BIT(target, start_bit + i);
      else CLEAR_BIT(target, start_bit + i);
    for(int i = start_index + 1; i < end_index; i ++)
      target[i] = (target[i] << 1) | ((target[i + 1] & 0x80) >> 7);
    for(int i = 0; i < end_offset; i ++)
      if(GET_BIT(target, end_bit - end_offset + i + 1)) SET_BIT(target, end_bit - end_offset + i);
      else CLEAR_BIT(target, end_bit - end_offset + i);
  }
  if(carry) SET_BIT(target, end_bit);
  else CLEAR_BIT(target, end_bit);
}

void rotate_right(byte *target, int start_bit, int end_bit) {
  int carry = GET_BIT(target, end_bit), start_index = start_bit / 8, start_offset = start_bit % 8;
  int end_index = end_bit / 8, end_offset = end_bit % 8;
  if(start_index == end_index) {
    for(int i = end_bit; i > start_bit; i --)
      if(GET_BIT(target, i - 1)) SET_BIT(target, i);
      else CLEAR_BIT(target, i);
  } else {
    for(int i = end_offset; i >= 0; i --)
      if(GET_BIT(target, end_bit - end_offset + i - 1)) SET_BIT(target, end_bit - end_offset + i);
      else CLEAR_BIT(target, end_bit - end_offset + i);
    for(int i = end_index - 1; i > start_index; i --)
      target[i] = (target[i] >> 1) | ((target[i - 1] & 0x01) << 7);
    for(int i = 7 - start_offset; i > 0; i --)
      if(GET_BIT(target, start_bit + i - 1)) SET_BIT(target, start_bit + i);
      else CLEAR_BIT(target, start_bit + i);
  }
  if(carry) SET_BIT(target, start_bit);
  else CLEAR_BIT(target, start_bit);
}

void rotate(byte *target, int start_bit, int end_bit, rotate_direction direction) {
  switch(direction) {
    case  LEFT: rotate_left(target, start_bit, end_bit); break;
    case RIGHT: rotate_right(target, start_bit, end_bit); break;
    default: break;
  }
}

int get_bits(byte *array, int start_bit, int bits) {
  int ret = 0;
  for(int i = 0; i < bits; i ++) {
    ret <<= 1;
    if(GET_BIT(array, start_bit + i)) ret += 1;
  }
  return ret;
}

void byte_swap(byte *target, int len) {
  byte temp;
  for(int i = 0; i < len / 2; i ++) {
    memcpy(&temp, target + i, sizeof(byte));
    memcpy(target + i, target + len - i - 1, sizeof(byte));
    memcpy(target + len - i - 1, &temp, sizeof(byte));
  }
}
