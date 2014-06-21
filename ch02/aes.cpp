#include "aes.h"
#include <memory>

void rotate_byte_left(byte *target, int len) {
  byte temp = target[0];
  for(int i = 1; i < len; i ++) target[i-1] = target[i];
  target[len - 1] = temp;
}

void substitution(byte *target, int len) {
  for(int i =  0; i < len; i ++)
    target[i] = sbox[(target[i] & 0xF0) >> 4][target[i] & 0x0F];
}

// compute all scheduled keys
void compute_key_schedule(const byte* key, int key_length, byte w[][AES_WORD_SIZE]) {
  byte rcon = 0x01;
  int key_words = key_length / AES_WORD_SIZE;
  // Step 1 : Copy all key bytes into scheduled keys
  for(int i = 0; i < key_words; i ++)
    memcpy(w[i], key + i * AES_WORD_SIZE, key_length);
  // Step 2 : Iterate to generate all keys
  for(int i = key_words; i < 4 * (key_words + 7); i ++) {
    memcpy(w[i], w[i - 1], AES_WORD_SIZE);
    if(!(i % key_words)) {
      // Rotate
      rotate_byte_left(w[i], AES_WORD_SIZE);
      // Substitution
      substitution(w[i], AES_WORD_SIZE);
      // xor with round constant
      if(!(i % 36)) rcon = 0x1b; // ? why
      w[i][0] ^= rcon;
      rcon <<= 1;
    } else if((key_length == AES_256_KEY_SIZE) && (i % key_words == 4)) {
      substitution(w[i], AES_WORD_SIZE);
    }
    for(int j = 0; j < AES_WORD_SIZE; j ++)
      w[i][j] ^= w[i - key_words][j];
  }
}

void shift_rows(byte state[][4])
{
  int tmp;

  tmp = state[1][0];
  state[1][0] = state[1][1];
  state[1][1] = state[1][2];
  state[1][2] = state[1][3];
  state[1][3] = tmp;

  tmp = state[2][0];
  state[2][0] = state[2][2];
  state[2][2] = tmp;
  tmp = state[2][1];
  state[2][1] = state[2][3];
  state[2][3] = tmp;

  tmp = state[3][3];
  state[3][3] = state[3][2];
  state[3][2] = state[3][1];
  state[3][1] = state[3][0];
  state[3][0] = tmp;
}

void add_round_key(byte state[][AES_WORD_SIZE], byte w[][AES_WORD_SIZE]) {
  for(int r = 0; r < AES_WORD_SIZE; r ++)
    for(int c = 0; c < AES_WORD_SIZE; c ++)
      state[r][c] ^= w[c][r];
}

void substitution_state(byte state[][AES_WORD_SIZE]) {
  for(int r = 0; r < AES_WORD_SIZE; r ++)
    substitution(state[r], AES_WORD_SIZE);
}

byte xtime(byte x) {
  return (x << 1) ^ ((x & 0x80) ? 0x1b : 0x00);
}

byte dot(byte x, byte y) {
  byte product = 0;
  for(byte mask = 0x01; mask; mask <<= 1) {
    if(y & mask) {
      product ^= x;
    }
    x = xtime(x);
  }
  return product;
}

void mix_columns(byte s[][4]) {
  byte t[4];
  for(int c = 0; c < 4; c++) {
    t[0] = dot(2, s[0][c]) ^ dot(3, s[1][c]) ^ s[2][c] ^ s[3][c];
    t[1] = s[0][c] ^ dot(2, s[1][c]) ^ dot(3, s[2][c]) ^ s[3][c];
    t[2] = s[0][c] ^ s[1][c] ^ dot(2, s[2][c]) ^ dot(3, s[3][c]);
    t[3] = dot(3, s[0][c]) ^ s[1][c] ^ s[2][c] ^ dot(2, s[3][c]);
    s[0][c] = t[0];
    s[1][c] = t[1];
    s[2][c] = t[2];
    s[3][c] = t[3];
  }
}

void aes_block_encrypt(const byte *input, byte *output, const byte *key, int key_size) {
  byte state[AES_WORD_SIZE][AES_BLOCK_SIZE / AES_WORD_SIZE];
  // Step 1 : Build the state
  for(int r = 0; r < AES_WORD_SIZE; r ++)
    for(int c = 0; c < AES_BLOCK_SIZE / AES_WORD_SIZE; c ++)
      state[r][c] = input[r + AES_WORD_SIZE * c];

  // Step 2 : Compute scheduled keys
  byte w[AES_WORD_SIZE * (key_size / AES_WORD_SIZE + 7)][AES_WORD_SIZE];
  compute_key_schedule(key, key_size, w);

  // Step 3 : Xor the input with the first four 4bytes
  add_round_key(state, w);

  // Step 4 : process for several rounds
  int round_number = key_size / AES_WORD_SIZE + 6;
  for(int round = 0; round < round_number; round ++) {
    substitution_state(state);
    shift_rows(state);
    if(round < round_number - 1) {
      mix_columns(state);
    }
    add_round_key(state, &w[(round + 1) * 4]);
  }

  // Step 5 : Build the output from the state
  for(int r = 0; r < AES_WORD_SIZE; r ++)
    for(int c = 0; c < AES_BLOCK_SIZE / AES_WORD_SIZE; c ++)
      output[r + (c * AES_WORD_SIZE)] = state[r][c];
}

void inv_shift_rows(byte state[][AES_WORD_SIZE]) {
  int tmp;
  tmp = state[1][2];
  state[1][2] = state[1][1];
  state[1][1] = state[1][0];
  state[1][0] = state[1][3];
  state[1][3] = tmp;

  tmp = state[2][0];
  state[2][0] = state[2][2];
  state[2][2] = tmp;
  tmp = state[2][1];
  state[2][1] = state[2][3];
  state[2][3] = tmp;

  tmp = state[3][0];
  state[3][0] = state[3][1];
  state[3][1] = state[3][2];
  state[3][2] = state[3][3];
  state[3][3] = tmp;
}

void inv_sub_bytes(byte state[][AES_WORD_SIZE]) {
  for(int r = 0; r < AES_WORD_SIZE; r++)
    for(int c = 0; c < AES_WORD_SIZE; c++)
      state[r][c] = inv_sbox[(state[r][c] & 0xF0) >> 4][state[r][c] & 0x0F];
}

void inv_mix_columns(byte s[][AES_WORD_SIZE]) {
  byte t[AES_WORD_SIZE];
  for(int c = 0; c < AES_WORD_SIZE; c++) {
    t[0] = dot(0x0e, s[0][c]) ^ dot(0x0b, s[1][c]) ^ dot(0x0d, s[2][c]) ^ dot(0x09, s[3][c]);
    t[1] = dot(0x09, s[0][c]) ^ dot(0x0e, s[1][c]) ^ dot(0x0b, s[2][c]) ^ dot(0x0d, s[3][c]);
    t[2] = dot(0x0d, s[0][c]) ^ dot(0x09, s[1][c]) ^ dot(0x0e, s[2][c]) ^ dot(0x0b, s[3][c]);
    t[3] = dot(0x0b, s[0][c]) ^ dot(0x0d, s[1][c]) ^ dot(0x09, s[2][c]) ^ dot(0x0e, s[3][c]);
    s[0][c] = t[0];
    s[1][c] = t[1];
    s[2][c] = t[2];
    s[3][c] = t[3];
  }
}

void aes_block_decrypt(const byte *input, byte *output, const byte *key, int key_size) {
  byte state[AES_WORD_SIZE][AES_BLOCK_SIZE / AES_WORD_SIZE];

  // Step 1 : Build the state
  for(int r = 0; r < 4; r++)
    for(int c = 0; c < 4; c++)
      state[r][c] = input[r + (4 * c)];

  // Step 2 : Compute the scheduled key
  byte w[AES_WORD_SIZE * (key_size / AES_WORD_SIZE + 7)][AES_WORD_SIZE];
  compute_key_schedule(key, key_size, w);

  // Step 3 : Xor the state with last four keys
  int num_round = (key_size >> 2) + 6;
  add_round_key(state, &w[num_round * 4]);

  // Step 4 : process for several rounds
  for(int round = num_round; round > 0; round--) {
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, &w[(round - 1) * 4]);
    if (round > 1) inv_mix_columns(state);
  }

  for(int r = 0; r < AES_WORD_SIZE; r++)
    for(int c = 0; c < AES_BLOCK_SIZE / AES_WORD_SIZE; c++)
      output[r + (4 * c)] = state[r][c];
}


void aes_encrypt(const byte *input, int input_len, byte *output, const byte *iv, const byte *key, int key_length) {
  byte input_block[ AES_BLOCK_SIZE ];
  while (input_len >= AES_BLOCK_SIZE) {
    memcpy(input_block, input, AES_BLOCK_SIZE);
    xor_array(input_block, iv, AES_BLOCK_SIZE); // implement CBC
    aes_block_encrypt(input_block, output, key, key_length);
    memcpy((void *) iv, (void *) output, AES_BLOCK_SIZE); // CBC
    input += AES_BLOCK_SIZE;
    output += AES_BLOCK_SIZE;
    input_len -= AES_BLOCK_SIZE;
  }
}

void aes_decrypt(const byte *input, int input_len, byte *output, const byte *iv, const byte *key, int key_length) {
  while (input_len >= AES_BLOCK_SIZE) {
    aes_block_decrypt(input, output, key, key_length);
    xor_array(output, iv, AES_BLOCK_SIZE);
    memcpy((void *) iv, (void *) input, AES_BLOCK_SIZE); // CBC
    input += AES_BLOCK_SIZE;
    output += AES_BLOCK_SIZE;
    input_len -= AES_BLOCK_SIZE;
  }
}

void aes_128_encrypt(const byte *plain, const int plain_len, byte cipher[], void *iv, const byte *key) {
  aes_encrypt(plain, plain_len, cipher, (const byte *) iv, key, 16);
}

void aes_128_decrypt(const byte *cipher, const int cipher_len, byte plain[], void *iv, const byte *key) {
  aes_decrypt(cipher, cipher_len, plain, (const byte *) iv, key, 16);
}

void aes_256_encrypt(const byte *plain, const int plain_len, byte cipher[], void *iv, const byte *key) {
  aes_encrypt(plain, plain_len, cipher, (const byte *) iv, key, 32);
}

void aes_256_decrypt(const byte *cipher, const int cipher_len, byte plain[], void *iv, const byte *key) {
  aes_decrypt(cipher, cipher_len, plain,  (const byte *) iv, key, 32);
}


