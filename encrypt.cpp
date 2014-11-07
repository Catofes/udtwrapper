/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

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
