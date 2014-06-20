#include "des.h"
#include "bit.h"
#include <iostream>
using namespace std;

int main() {
  byte key[8] = {'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd'};
  byte plain[8] = {'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd'};
  byte cipher[8];
  //byte_swap(plain, 8);
  des_encrypt(plain, cipher, 8, key, PKCS_5);
  //byte_swap(cipher, 8);
  for(int i = 0; i < 8; i ++) {
    cout << hex << (int)cipher[i] << " ";
  }
  cout << endl;
}

