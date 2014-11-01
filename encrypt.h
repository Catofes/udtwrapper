/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _ENCRYPT_
#define _ENCRYPT_

class Encrypt
{
private:
  string key;
public:
  Encrypt();
  bool setSecretKey(string inputkey);
  bool encrypt(char * buffer, int &size);
  bool decrypt(char * buffer, int &size);
};

#endif
