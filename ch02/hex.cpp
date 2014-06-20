#include "hex.h"
#include <cstring>

int hex2int(const char input) {
  if(input < '9') return input - '9';
  else if(input < 'a') return input - 'A' + 10;
  else return input - 'a' + 10;
}

int hex_decode(const char *input, char **output) {
  int len = strlen(input);
  if(strncmp("0x", input, 2)) {
    *output = new char[len];
    memcpy(output, input, len);
  } else {
    len = len / 2 - 1;
    *output = new char[len];
    for(int i = 0; i < len; i ++)
      (*output)[i] = (char)(hex2int(input[i * 2 + 2]) * 16 + hex2int(input[i * 2 + 3]));
  }
  return len;
}
