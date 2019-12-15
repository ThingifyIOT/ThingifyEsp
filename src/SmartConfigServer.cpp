
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif 

#include "SmartConfigServer.h"
#include "Serialization/Serializer.h"
#include "Api/ZeroConfigurationPacket.h"
#include "Api/ZeroConfigurationResponsePacket.h"

SmartConfigServer::SmartConfigServer() : _server(8888), _logger(LoggerInstance)
{

}
void SmartConfigServer::Start()
{
    _server.begin();
    Serial.println("Started smart config server");
}

void SmartConfigServer::Loop()
{
    WiFiClient client = _server.available();
    if (!client) 
    {    
        return;  
    }
    _logger.info(F("Got remote connection from: %s"), client.remoteIP().toString().c_str());
    
    uint64_t start = millis();
    while (client.available() < 2)
    {
        if(millis() - start > 2000)
        {
            _logger.err(F("Timout waiting for header"));
            return;
        }
        delay(10);
    }

    char packetLengthBuffer[2];
    if(client.readBytes(packetLengthBuffer, 2) != 2)
    {
        return;
    }

    uint16_t packetLength = ntohs(*(uint16_t*)packetLengthBuffer);
    while (client.available() > packetLength)
    {
        if(millis() - start > 2000)
        {
            _logger.err(F("Timout waiting for packet body"));
            return;
        }
        delay(10);
    }
    char packetBodyBytes[2000];
    if(client.readBytes(packetBodyBytes, packetLength) != packetLength)
    {
        _logger.err(F("Failed to read packet body"));
        return;
    }
    auto payload = new FixedString<1000>;
	payload->append(packetBodyBytes, packetLength);
	const auto packet = Serializer::DeserializePacket(*payload);
    if(packet == nullptr)
    {
        _logger.err(F("Failed to deserialize packet"));
        return;
    }
    if(packet->PacketType() != ThingifyPacketType::ZeroConfigurationPacket)
    {
        _logger.err(F("Wrong packet type"));
        return;
    }
    auto zeroConfiguration = static_cast<ZeroConfigurationPacket*>(packet);  

    _logger.info(F("Got smart config: api = %s, token = %s"), zeroConfiguration->ApiAddress.c_str(), zeroConfiguration->Token.c_str());

    for(int i=0; i < zeroConfiguration->WifiNetworks.size(); i++)
    {
        _logger.info(F(" Network%d: %s, %s"), i+1, zeroConfiguration->WifiNetworks[i]->Name.c_str(), zeroConfiguration->WifiNetworks[i]->Password.c_str());
    }

    auto zeroConfigurationResponsePacket = new ZeroConfigurationResponsePacket();
    FixedString200 responseBytes;
    if(!Serializer::SerializePacket(zeroConfigurationResponsePacket, responseBytes))
    {
        _logger.err(F("Failed to serialize zero config response packet"));
        return;
    }
    char lengthHeader[2];
    *((int16_t*)lengthHeader) = htons(responseBytes.length());
    if(client.write(lengthHeader, 2) != 2)
    {
        _logger.err(F("Failed to write length header"));
        return;
    }
    if(client.write(responseBytes.c_str(), responseBytes.length()) != responseBytes.length())
    {
        _logger.err(F("Failed to write response packet"));
        return;
    }
}