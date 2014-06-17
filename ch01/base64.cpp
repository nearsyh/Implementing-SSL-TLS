#include <string>
#include <cassert>
#include <iostream>
#include "base64.h"
using namespace std;

const static string base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const static int unbase64[] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
  53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 0, -1, -1, -1,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
  42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1
}; 

string base64_encode(string input) {
  string ret = "";
  int len = input.length(), index = 0;
  do {
    ret += base64[(input[index] & 0xFC) >> 2];
    if(len - index == 1) {
      ret += base64[(input[index] & 0x03) << 4];
      ret += "==";
      break;
    }
    ret += base64[((input[index] & 0x03) << 4) | ((input[index + 1] & 0xF0) >> 4)];
    if(len - index == 2) {
      ret += base64[(input[index+1] & 0x0F) << 2];
      ret += "=";
      break;
    }
    ret += base64[((input[index+1] & 0x0F) << 2) | ((input[index+2] & 0xC0) >> 6)];
    ret += base64[input[index+2] & 0x3F];
    index += 3;
  } while(index < len);
  return ret;
}

string base64_decode(string input) {
  assert(!(input.length() & 0x03));
  string ret = "";
  for(int i = 0; i < input.length(); i ++) {
    if(static_cast<int>(input[i]) > 128 || unbase64[static_cast<int>(input[i])] == -1) {
      cerr << "Invalid base64 input" << endl;
      return "";
    }
  }
  int len = input.length(), index = 0;
  do {
    ret += (unbase64[input[index]] << 2) | (unbase64[input[index + 1]] & 0x30) >> 4;
    if(input[index + 2] != '=')
      ret += (unbase64[input[index + 1]] & 0x0f) << 4 | (unbase64[input[index + 2]] & 0x3c) >> 2;
    if(input[index + 3] != '=')
      ret += (unbase64[input[index + 2]] & 0x03) << 6 | unbase64[input[index + 3]];
    index += 4;
  } while(index < len);
  return ret;
}
