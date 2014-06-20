#include "des.h"
#include "bit.h"
#include "hex.h"
#include <iostream>
using namespace std;

int main() {
  byte key[24] = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c'};
  byte plain[8] = {'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd'};
  byte iv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  byte cipher[8];
  /*
  des3_encrypt(plain, cipher, 8, key, iv, PKCS_5);
  for(int i = 0; i < 8; i ++) {
    cout << hex << (int)cipher[i] << " ";
  }
  cout << endl;
  */
  const char *new_cipher = "0xc782182a883ad57a";
  byte *input;
  cout << hex_decode(new_cipher, (char**)&input) << endl;
  des3_decrypt(input, plain, 8, key, iv, PKCS_5);
  for(int i = 0; i < 8; i ++) {
    cout << (int)plain[i] << " ";
  }
  cout << endl;
}

