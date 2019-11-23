
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif 

#include "SmartConfigServer.h"
#include "Serialization/Serializer.h"
#include "Api/ZeroConfigurationPacket.h"

SmartConfigServer::SmartConfigServer() : _server(8888)
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
    Serial.printf("Got remote connection from: %s\n", client.remoteIP().toString().c_str());
    
    uint64_t start = millis();
    while (client.available() < 2)
    {
        if(millis() - start > 2000)
        {
            Serial.printf("Timout waiting for header\n");
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
            Serial.printf("Timout waiting for packet body\n");
            return;
        }
        delay(10);
    }
    char packetBodyBytes[2000];
    if(client.readBytes(packetBodyBytes, packetLength) != packetLength)
    {
        Serial.println("Failed to read packet body");
        return;
    }
    auto payload = new FixedString<1000>;
	payload->append(packetBodyBytes, packetLength);
	const auto packet = Serializer::DeserializePacket(*payload);
    if(packet == nullptr)
    {
        Serial.println("Failed to deserialize packet");
        return;
    }
    if(packet->PacketType() != ThingifyPacketType::ZeroConfigurationPacket)
    {
        Serial.println("Wrong packet type");
        return;
    }
    auto zeroConfiguration = static_cast<ZeroConfigurationPacket*>(packet);  

    Serial.printf("Got smart config: api = %s, token = %s\n", zeroConfiguration->ApiAddress.c_str(), zeroConfiguration->Token.c_str());

    for(int i=0; i < zeroConfiguration->WifiNetworks.size(); i++)
    {
        Serial.printf(" Network%d: %s, %s\n", i+1, zeroConfiguration->WifiNetworks[i]->Name.c_str(), zeroConfiguration->WifiNetworks[i]->Password.c_str());
    }
}