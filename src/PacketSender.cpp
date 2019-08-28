#include "PacketSender.h"
#include "Helpers/PacketLogger.h"
#include "Serialization/Serializer.h"

PacketSender::PacketSender(AsyncMqttClient& mqtt) :
	_logger(ContiLoggerInstance),
	_mqtt(mqtt),
	_sentPacketCount(0)
{

}

bool PacketSender::SendPacket(PacketBase* packet)
{
	PacketLogger::LogPacket(" -> ", packet);
	auto packetBuffer = new FixedString<1800>;
	if (!Serializer::SerializePacket(packet, *packetBuffer))
	{
		delete packetBuffer;
		_logger.err(F("Failed to serialize packet:"));
		PacketLogger::LogPacket("Packet", packet);
		//SetError(F("Failed to serialize packet"));
		return false;
	}
	_sentPacketCount++;

	auto pubId = _mqtt.publish(_outTopic.c_str(), 0, false,
		packetBuffer->c_str(),
		packetBuffer->length());

	delete packetBuffer;

	if (pubId == 0)
	{
		_logger.err(L("Failed to publish packet"));
		return false;
	}
	return true;
}

void PacketSender::SetOutTopic(FixedStringBase& outTopic)
{
	_outTopic = outTopic;
}

uint16_t PacketSender::GetSentPacketCount()
{
	return _sentPacketCount;
}