#ifndef DES_H
#define DES_H

#include "bit.h"

typedef enum { OP_ENCRYPT, OP_DECRYPT } op_type;
typedef enum { NIST_800_3A, PKCS_5 } padding_scheme;

const int DES_BLOCK_SIZE = 8;
const int DES_KEY_SIZE = 8;
const int EXPANSION_BLOCK_SIZE = 6;
const int PC1_KEY_SIZE = 7;
const int SUBKEY_SIZE = 6;
const int SBOX_BITS_SIZE = 6;

void des_encrypt(const byte *plain, byte *cipher, int len, const byte *key, padding_scheme scheme);
void des_decrypt(const byte *cipher, byte *plain, int len, const byte *key, padding_scheme scheme);

#endif
