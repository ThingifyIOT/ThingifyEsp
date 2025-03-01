#ifndef _THINGIFY_PACKET_TYPE_H
#define _THINGIFY_PACKET_TYPE_H

enum class ThingifyPacketType
{
	Heartbeat = 0,
	UpdateNodes = 1,
	AckToClient = 2,
	UpdateNodesFromClient = 3,
	AckToDevice = 4,
	ClientSessionCreateAck = 5,
	UpdateClientNodeSubscription = 6,
	ThingSessionCreate = 7,
	ClientSessionCreate = 8,
	ThingSessionCreateAck = 9,
	ClientReceivedCreateSessionAck = 10,
	UpdateFirmwareDataAck = 21,
	UpdateFirmwareCommitToThing = 24,
	UpdateFirmwareBeginToThing = 26,
	ZeroConfigurationPacket = 27,
	ZeroConfigurationResponsePacket = 28,
	ZeroConfigurationDeviceInfoRequestPacket = 29,
	ZeroConfigurationDeviceInfoResponsePacket = 30
};
#endif