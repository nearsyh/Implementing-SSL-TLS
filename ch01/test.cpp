#include "base64.h"
#include <iostream>
#include <cassert>
using namespace std;

int main() {
  string input = "abcdABCD0123,./+";
  assert(base64_decode(base64_encode(input)) == input);
  return 0;
}
