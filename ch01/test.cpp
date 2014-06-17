#include "base64.h"
#include <iostream>
using namespace std;

int main() {
  string input;
  cin >> input;
  cout << base64_encode(input) << endl;
  cout << base64_decode(base64_encode(input)) << endl;
  return 0;
}
