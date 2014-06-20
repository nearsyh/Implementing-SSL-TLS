#include "des.h"
#include <memory>
#include <cassert>
#include <iostream>
using namespace std;

static const int initial_permutation_table[] = {
  58, 50, 42, 34, 26, 18, 10, 2,
  60, 52, 44, 36, 28, 20, 12, 4,
  62, 54, 46, 38, 30, 22, 14, 6,
  64, 56, 48, 40, 32, 24, 16, 8,
  57, 49, 41, 33, 25, 17, 9,  1,
  59, 51, 43, 35, 27, 19, 11, 3,
  61, 53, 45, 37, 29, 21, 13, 5,
  63, 55, 47, 39, 31, 23, 15, 7 };

static const int final_permutation_table[] = {
  40, 8, 48, 16, 56, 24, 64, 32,
  39, 7, 47, 15, 55, 23, 63, 31,
  38, 6, 46, 14, 54, 22, 62, 30,
  37, 5, 45, 13, 53, 21, 61, 29,
  36, 4, 44, 12, 52, 20, 60, 28,
  35, 3, 43, 11, 51, 19, 59, 27,
  34, 2, 42, 10, 50, 18, 58, 26,
  33, 1, 41,  9, 49, 17, 57, 25 };

static const int pc1_table[] = {
  57, 49, 41, 33, 25, 17,  9, 1,
  58, 50, 42, 34, 26, 18, 10, 2,
  59, 51, 43, 35, 27, 19, 11, 3,
  60, 52, 44, 36,
  63, 55, 47, 39, 31, 23, 15, 7,
  62, 54, 46, 38, 30, 22, 14, 6,
  61, 53, 45, 37, 29, 21, 13, 5,
  28, 20, 12,  4 };

static const int pc2_table[] = {
  14, 17, 11, 24,  1,  5,
   3, 28, 15,  6, 21, 10,
  23, 19, 12,  4, 26,  8,
  16,  7, 27, 20, 13,  2,
  41, 52, 31, 37, 47, 55,
  30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53,
  46, 42, 50, 36, 29, 32 };

static const int expansion_table[] = {
  32,  1,  2,  3,  4,  5,
   4,  5,  6,  7,  8,  9,
   8,  9, 10, 11, 12, 13,
  12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21,
  20, 21, 22, 23, 24, 25,
  24, 25, 26, 27, 28, 29,
  28, 29, 30, 31, 32,  1 };

static const int sbox[8][64] = {
  { 14, 0, 4, 15, 13, 7, 1, 4, 2, 14, 15, 2, 11, 13, 8, 1,
    3, 10, 10, 6, 6, 12, 12, 11, 5, 9, 9, 5, 0, 3, 7, 8,
    4, 15, 1, 12, 14, 8, 8, 2, 13, 4, 6, 9, 2, 1, 11, 7,
    15, 5, 12, 11, 9, 3, 7, 14, 3, 10, 10, 0, 5, 6, 0, 13 },
  { 15, 3, 1, 13, 8, 4, 14, 7, 6, 15, 11, 2, 3, 8, 4, 14,
    9, 12, 7, 0, 2, 1, 13, 10, 12, 6, 0, 9, 5, 11, 10, 5,
    0, 13, 14, 8, 7, 10, 11, 1, 10, 3, 4, 15, 13, 4, 1, 2,
    5, 11, 8, 6, 12, 7, 6, 12, 9, 0, 3, 5, 2, 14, 15, 9 },
  { 10, 13, 0, 7, 9, 0, 14, 9, 6, 3, 3, 4, 15, 6, 5, 10,
    1, 2, 13, 8, 12, 5, 7, 14, 11, 12, 4, 11, 2, 15, 8, 1,
    13, 1, 6, 10, 4, 13, 9, 0, 8, 6, 15, 9, 3, 8, 0, 7,
    11, 4, 1, 15, 2, 14, 12, 3, 5, 11, 10, 5, 14, 2, 7, 12 },
  { 7, 13, 13, 8, 14, 11, 3, 5, 0, 6, 6, 15, 9, 0, 10, 3,
    1, 4, 2, 7, 8, 2, 5, 12, 11, 1, 12, 10, 4, 14, 15, 9,
    10, 3, 6, 15, 9, 0, 0, 6, 12, 10, 11, 1, 7, 13, 13, 8,
    15, 9, 1, 4, 3, 5, 14, 11, 5, 12, 2, 7, 8, 2, 4, 14 },
  { 2, 14, 12, 11, 4, 2, 1, 12, 7, 4, 10, 7, 11, 13, 6, 1,
    8, 5, 5, 0, 3, 15, 15, 10, 13, 3, 0, 9, 14, 8, 9, 6,
    4, 11, 2, 8, 1, 12, 11, 7, 10, 1, 13, 14, 7, 2, 8, 13,
    15, 6, 9, 15, 12, 0, 5, 9, 6, 10, 3, 4, 0, 5, 14, 3 },
  { 12, 10, 1, 15, 10, 4, 15, 2, 9, 7, 2, 12, 6, 9, 8, 5,
    0, 6, 13, 1, 3, 13, 4, 14, 14, 0, 7, 11, 5, 3, 11, 8,
    9, 4, 14, 3, 15, 2, 5, 12, 2, 9, 8, 5, 12, 15, 3, 10,
    7, 11, 0, 14, 4, 1, 10, 7, 1, 6, 13, 0, 11, 8, 6, 13 },
  { 4, 13, 11, 0, 2, 11, 14, 7, 15, 4, 0, 9, 8, 1, 13, 10,
    3, 14, 12, 3, 9, 5, 7, 12, 5, 2, 10, 15, 6, 8, 1, 6,
    1, 6, 4, 11, 11, 13, 13, 8, 12, 1, 3, 4, 7, 10, 14, 7,
    10, 9, 15, 5, 6, 0, 8, 15, 0, 14, 5, 2, 9, 3, 2, 12 },
  { 13, 1, 2, 15, 8, 13, 4, 8, 6, 10, 15, 3, 11, 7, 1, 4,
    10, 12, 9, 5, 3, 6, 14, 11, 5, 0, 0, 14, 12, 9, 7, 2,
    7, 2, 11, 1, 4, 14, 1, 7, 9, 4, 12, 10, 14, 8, 2, 13,
    0, 15, 6, 12, 10, 9, 13, 0, 15, 3, 3, 5, 5, 6, 8, 11 }
};

static const int p_table[] = {
  16,  7, 20, 21,
  29, 12, 28, 17,
   1, 15, 23, 26,
   5, 18, 31, 10,
   2,  8, 24, 14,
  32, 27,  3,  9,
  19, 13, 30,  6,
  22, 11,  4, 25 };

void debug_info(const byte *output, int len, const char *tag) {
  cout << tag << " : ";
  for(int i = 0; i < len; i ++) {
    cout << dec << (int)output[i] << " ";
  }
  cout << endl;
}

void des_round_operate(byte *output, byte *key, int round, op_type type) {
  byte key_48bits[SUBKEY_SIZE], expand_r[EXPANSION_BLOCK_SIZE], new_right[DES_BLOCK_SIZE / 2];
  byte *left = output, *right = output + DES_BLOCK_SIZE / 2;
  rotate_direction direction = type == OP_ENCRYPT ? LEFT : RIGHT;
  // Step 1 : Expand right of output to 48 bits
  permute(expand_r, right, expansion_table, EXPANSION_BLOCK_SIZE);
  // Step 2 : Get Key_round and xor with the above 48 bits
  if(type == OP_ENCRYPT) {
    rotate(key, 0, 27, direction);
    rotate(key, 28, 55, direction);
    if(!(round == 1 || round == 2 || round == 9 || round == 16)) {
      rotate(key, 0, 27, direction);
      rotate(key, 28, 55, direction);
    }
  }
  permute(key_48bits, key, pc2_table, SUBKEY_SIZE);
  if(type == OP_DECRYPT) {
    rotate(key, 0, 27, direction);
    rotate(key, 28, 55, direction);
    if(!(round == 16 || round == 15 || round == 8 || round == 1)) {
      rotate(key, 0, 27, direction);
      rotate(key, 28, 55, direction);
    }
  }
  xor_array(expand_r, key_48bits, EXPANSION_BLOCK_SIZE);
  // Step 3 : S box
  memset(new_right, 0, DES_BLOCK_SIZE / 2 * sizeof(byte));
  for(int i = 0; i < 8; i ++) {
    int index = get_bits(expand_r, i * SBOX_BITS_SIZE, SBOX_BITS_SIZE);
    new_right[i / 2] |= (byte)(sbox[i][index] << (i % 2 ? 0 : 4));
  }
  // Step 4 : P table
  byte permuted_new_right[DES_BLOCK_SIZE / 2];
  permute(permuted_new_right, new_right, p_table, DES_BLOCK_SIZE / 2);
  // Step 5 : xor the above result with left part. Swap
  xor_array(permuted_new_right, left, DES_BLOCK_SIZE / 2);
  memcpy(left, right, 4 * sizeof(byte));
  memcpy(right, permuted_new_right, 4 * sizeof(byte));
}


void des_block_operate(byte *output, const byte *input, const byte *key, op_type type) {
  byte key_56bit[7], temp_output[DES_BLOCK_SIZE];
  // Step 1 : initial permutation, actually add no security, prepare 56bits key
  permute(temp_output, input, initial_permutation_table, DES_BLOCK_SIZE);
  permute(key_56bit, key, pc1_table, 7);
  // Step 2 : 16 rounds of feistel function, XOR and swap
  for(int i = 1; i <= 16; i ++) {
    des_round_operate(temp_output, key_56bit, i, type);
  }
  // Step 5 : swap one more time
  byte temp[DES_BLOCK_SIZE / 2];
  memcpy(temp, temp_output, DES_BLOCK_SIZE / 2);
  memcpy(temp_output, temp_output + DES_BLOCK_SIZE / 2, DES_BLOCK_SIZE / 2);
  memcpy(temp_output + DES_BLOCK_SIZE / 2, temp, DES_BLOCK_SIZE / 2);
  // Step 4 : final permutation, just inverts the initial permutation
  permute(output, temp_output, final_permutation_table, DES_BLOCK_SIZE);
}

void des_operate(const byte* input, byte* output, int len_in_byte, const byte* key, const byte* iv, op_type type, bool triple) {
  assert(!(len_in_byte % DES_BLOCK_SIZE));
  byte input_block[DES_BLOCK_SIZE], initial_vector[DES_BLOCK_SIZE];
  memcpy(initial_vector, iv, DES_BLOCK_SIZE);
  while(len_in_byte) {
    memcpy(input_block, input, DES_BLOCK_SIZE);
    if(type == OP_ENCRYPT) {
      xor_array(input_block, initial_vector, DES_BLOCK_SIZE);
      des_block_operate(output, input_block, key, type);
      if(triple) {
        memcpy(input_block, output, DES_BLOCK_SIZE);
        des_block_operate(output, input_block, key + DES_KEY_SIZE, OP_DECRYPT);
        memcpy(input_block, output, DES_BLOCK_SIZE);
        des_block_operate(output, input_block, key + DES_KEY_SIZE * 2, type);
      }
      memcpy((void*)initial_vector, output, DES_BLOCK_SIZE); // CBC
    } else if(type == OP_DECRYPT) {
      if(triple) {
        des_block_operate(output, input_block, key + DES_KEY_SIZE * 2, type);
        memcpy(input_block, output, DES_BLOCK_SIZE);
        des_block_operate(output, input_block, key + DES_KEY_SIZE, OP_ENCRYPT);
        memcpy(input_block, output, DES_BLOCK_SIZE);
      }
      des_block_operate(output, input_block, key, type);
      xor_array(output, initial_vector, DES_BLOCK_SIZE);
      memcpy((void*)initial_vector, input, DES_BLOCK_SIZE); // CBC
    }
    input += DES_BLOCK_SIZE;
    output += DES_BLOCK_SIZE;
    len_in_byte -= DES_BLOCK_SIZE;
  }
}

void des_padding(const byte *input, byte *output, int input_len, op_type type, padding_scheme scheme) {
  int padding_len = DES_BLOCK_SIZE - (input_len % DES_BLOCK_SIZE), padded_len = padding_len + input_len;
  memset(output, 0, padded_len * sizeof(byte));
  memcpy(output, input, input_len * sizeof(byte));
  switch(scheme) {
    case NIST_800_3A :
      output[input_len] = 0x80;
      break;
    case PKCS_5:
      memset(output + input_len, padding_len, padding_len);
      break;
    default:
      break;
  }
}

void des_encrypt(const byte* plain, byte* cipher, int len_in_byte, const byte* key, const byte* iv, padding_scheme scheme) {
  des_operate(plain, cipher, len_in_byte, key, iv, OP_ENCRYPT, false);
}

void des3_encrypt(const byte* plain, byte* cipher, int len_in_byte, const byte* key, const byte* iv, padding_scheme scheme) {
  des_operate(plain, cipher, len_in_byte, key, iv, OP_ENCRYPT, true);
}

void des_decrypt(const byte* cipher, byte* plain, int len_in_byte, const byte* key, const byte* iv, padding_scheme scheme) {
  des_operate(cipher, plain, len_in_byte, key, iv, OP_DECRYPT, false);
}

void des3_decrypt(const byte* cipher, byte* plain, int len_in_byte, const byte* key, const byte* iv, padding_scheme scheme) {
  des_operate(cipher, plain, len_in_byte, key, iv, OP_DECRYPT, true);
}
