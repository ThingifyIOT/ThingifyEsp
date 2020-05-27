#ifndef _WIFI_CREDENTIAL_H
#define _WIFI_CREDENTIAL_H

#include <FixedString.h>


class WifiCredential
{    
public:
    WifiCredential(const char* name, const char* password) :Name(name), Password(password)
    {

    }
    WifiCredential(FixedString32 name, FixedString64 password) :Name(name), Password(password)
    {

    }
    FixedString32 Name;
    FixedString64 Password;
};

#endif