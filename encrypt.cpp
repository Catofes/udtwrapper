#include <iostream>
using namespace std;

#include "encrypt.h"

Encrypt::Encrypt()
{
  key = "";
}

bool Encrypt::setSecretKey(string inputkey)
{
  key = inputkey;
}

bool Encrypt::encrypt(char *buffer, int &size)
{
  for(int i = 0; i < size; i++){
    buffer[i]++;
  }
}

bool Encrypt::decrypt(char *buffer, int &size)
{
  for(int i = 0; i < size; i++){
    buffer[i]--;
  }
}
