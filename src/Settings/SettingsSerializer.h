#ifndef _SETTINGS_SERIALIZER_H
#define _SETTINGS_SERIALIZER_H

#include "ThingSettings.h"
#include <FixedString.h>
#include "Serialization\cmp.h"  

class SettingsSerializer
{
    bool WriteMapPair(cmp_ctx_t& cmp, const char* key, FixedStringBase& value);
    bool WriteMapPair(cmp_ctx_t& cmp, const char* key, int value);
    bool WriteWifiNetwork(cmp_ctx_t& cmp,  WifiCredential *network);
    WifiCredential* ReadWifiNetwork(cmp_ctx_t& cmp);
public:
    bool Serialize(ThingSettings& settings, FixedStringBase& stream);
    bool Deserialize(FixedStringBase& stream, ThingSettings& settings);

};

#endif