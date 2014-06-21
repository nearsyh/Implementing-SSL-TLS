#include "des.h"
#include <memory>
#include <cassert>
#include <iostream>
using namespace std;

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
