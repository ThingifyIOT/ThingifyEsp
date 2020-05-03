#include "SettingsSerializer.h"
#include "Serialization\SerializationHelpers.h"
#include "Serialization\BufferReader.h"


bool SettingsSerializer::WriteMapPair(cmp_ctx_t& cmp, const char* key, FixedStringBase& value)
{
    if(!cmp_write_str(&cmp, key, strlen(key)))
    {
        return false;
    }
    if(!cmp_write_str(&cmp, value.c_str(), value.length()))
    {
        return false;
    }
    return true;
}
bool SettingsSerializer::WriteMapPair(cmp_ctx_t& cmp, const char* key, int value)
{
    if(!cmp_write_str(&cmp, key, strlen(key)))
    {
        return false;
    }
    if(!cmp_write_int(&cmp, value))
    {
        return false;
    }
    return true;
}
bool SettingsSerializer::Serialize(ThingSettings& settings, FixedStringBase& stream)
{
    cmp_ctx_t cmp;
	cmp_init(&cmp, &stream, 0, SerializationHelpers::FileWriter);
    if(!cmp_write_map(&cmp, 4))
    {
        return false;
    }
    if(!WriteMapPair(cmp, "a", settings.ServerName))
    {
        return false;
    }
	if(!WriteMapPair(cmp, "p", settings.ServerPort))
	{
		return false;
	}
    if(!WriteMapPair(cmp, "t", settings.Token))
    {
        return false;
    } 
    if(!WriteMapPair(cmp, "n", settings.ThingName))
    {
        return false;
    }
    if(!cmp_write_str(&cmp, "w", 1))
    {
        return false;
    }
    if(!cmp_write_array(&cmp, settings.WifiNetworks.size()))
    {
        return false;
    }
    for(int i=0; i < settings.WifiNetworks.size(); i++)
    {
        if(!WriteWifiNetwork(cmp, settings.WifiNetworks[i]));
    }
    return true;
}

bool SettingsSerializer::WriteWifiNetwork(cmp_ctx_t& cmp, WifiNetwork *network)
{
    if(!cmp_write_map(&cmp, 2))
    {
        return false;
    }
    if(!WriteMapPair(cmp, "n", network->Name))
    {
        return false;
    }
    if(!WriteMapPair(cmp, "p", network->Password))
    {
        return false;
    }
    return true;
}

bool SettingsSerializer::Deserialize(FixedStringBase& stream, ThingSettings& settings)
{
    BufferReader bufferReader(stream.c_str(), stream.length());
	cmp_ctx_t cmp;
	cmp_init(&cmp, &bufferReader, SerializationHelpers::FileReader, SerializationHelpers::FileWriter);

    uint32_t mapSize;
	if(!cmp_read_map(&cmp, &mapSize))
	{
		return false;
	}
	for(uint32_t i =0; i < mapSize; i++)
	{
		FixedString32 key;
		if(!SerializationHelpers::ReadCmpString(cmp, key))
		{
			return false;
		}
		if(key.equals("a"))
		{
			if(!SerializationHelpers::ReadCmpString(cmp, settings.ServerName))
			{
				return false;
			}
		}
		if(key.equals("p"))
		{
			if(!cmp_read_int(&cmp, &settings.ServerPort))
			{
				return false;
			}
		}
		else if(key.equals("t"))
		{
			if(!SerializationHelpers::ReadCmpString(cmp, settings.Token))
			{
				return false;
			}
		}
        else if(key.equals("n"))
		{
			if(!SerializationHelpers::ReadCmpString(cmp, settings.ThingName))
			{
				return false;
			}
		}
		else if(key.equals("w"))
		{
			uint32_t wifiArraySize;
			cmp_read_array(&cmp, &wifiArraySize);
			while (wifiArraySize-- > 0)
			{
				auto wifiNetwork = new WifiNetwork();
				if(!ReadWifiNetwork(cmp, *wifiNetwork))
				{
					delete wifiNetwork;
					return false;
				}
				settings.WifiNetworks.push_back(wifiNetwork);
			}			
		}
	}
    return true;
}


bool SettingsSerializer::ReadWifiNetwork(cmp_ctx_t& cmp, WifiNetwork& wifiNetwork)
{
	uint32_t mapSize;
	if(!cmp_read_map(&cmp, &mapSize))
	{
		return false;
	}
	for(uint32_t i =0; i < mapSize; i++)
	{
		FixedString32 key;
		if(!SerializationHelpers::ReadCmpString(cmp, key))
		{
			return false;
		}
		if(key.equals("n"))
		{
			if(!SerializationHelpers::ReadCmpString(cmp, wifiNetwork.Name))
            {
                return false;
            }
		}
		else if(key.equals("p"))
		{
			if(!SerializationHelpers::ReadCmpString(cmp, wifiNetwork.Password))
            {
                return false;
            }
		}
	}
	return true;
}
