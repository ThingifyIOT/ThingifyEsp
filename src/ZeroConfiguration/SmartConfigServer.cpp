
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

    _logger.info(F("Started smart config server"));
}
void SmartConfigServer::Stop()
{
    _server.stop();
    _logger.info(F("Stopped smart config server"));
}

ClientRequestContext* SmartConfigServer::GetRequest()
{
    WiFiClient client = _server.available();
    if (!client) 
    {    
        return nullptr;  
    }
    _logger.info(F("Got remote connection from: %s"), client.remoteIP().toString().c_str());
    
    uint64_t start = millis();
    while (client.available() < 2)
    {
        if(millis() - start > 2000)
        {
            _logger.err(F("Timout waiting for header"));
            return nullptr;  
        }
        delay(10);
    }

    char packetLengthBuffer[2];
    if(client.readBytes(packetLengthBuffer, 2) != 2)
    {
        return nullptr;  
    }

    uint16_t packetLength = ntohs(*(uint16_t*)packetLengthBuffer);
    while (client.available() > packetLength)
    {
        if(millis() - start > 2000)
        {
            _logger.err(F("Timout waiting for packet body"));
            return nullptr;  
        }
        delay(10);
    }
    char packetBodyBytes[2000];
    if(client.readBytes(packetBodyBytes, packetLength) != packetLength)
    {
        _logger.err(F("Failed to read packet body"));
        return nullptr;  
    }
    auto payload = new FixedString1024;
	payload->append(packetBodyBytes, packetLength);
	const auto packet = Serializer::DeserializePacket(*payload);
    if(packet == nullptr)
    {
        _logger.err(F("Failed to deserialize packet"));
        return nullptr;  
    }


    if(packet->PacketType() != ThingifyPacketType::ZeroConfigurationPacket && packet->PacketType() != ThingifyPacketType::ZeroConfigurationDeviceInfoRequestPacket)
    {
        _logger.err(F("Wrong packet type"));
        return nullptr;  
    }

    auto requestContext = new ClientRequestContext();
    requestContext->Client = client;
    requestContext->Packet = packet;
    return requestContext;
}

void SmartConfigServer::SendPacket(WiFiClient client, PacketBase* packet)
{
    FixedString128 responseBytes;
    if(!Serializer::SerializePacket(packet, responseBytes))
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