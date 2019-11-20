#ifndef _SERIALIZER_H
#define _SERIALIZER_H

#include <vector>
#include <Arduino.h>

#include "Node/Node.h"
#include "cmp.h"
#include "Api/UpdateNodesFromClientPacket.h"
#include "Api/UpdateNodesPacket.h"
#include "Api/PacketBase.h"
#include "Api/HeartbeatPacket.h"
#include "Api/AckToDevicePacket.h"
#include "Api/ThingSessionCreatePacket.h"
#include "Api/ThingSessionCreateAckPacket.h"
#include "Api/UpdateFirmwareDataAck.h"
#include "Logging/Logger.h"
#include "Api/ZeroConfigurationPacket.h"


class UpdateFirmwareDataAck;
class FixedStringBase;

class BufferReader
{
	const char* _buffer;
	int _length;
	int _postion;
public:
	BufferReader(const char *buffer, int length)
	{
		this->_buffer = buffer;
		this->_length = length;
		this->_postion = 0;
	}

	const char *ReadBuffer(int length)
	{
		if (_postion + length > _length)
		{
			return nullptr;
		}
		const char* ret = _buffer + _postion;
		_postion += length;
		return ret;
	}

};

class Serializer
{
public:
	static Logger& _logger;
	static size_t FileWriter(cmp_ctx_t *ctx, const void *data, size_t count);
	static bool FileReader(cmp_ctx_t *ctx, void *data, size_t count);
	static bool WritePacketHeader(cmp_ctx_t &cmp, ContiPacketType packetType);
	static bool WriteArrayPacketHeader(cmp_ctx_t &cmp, ContiPacketType packetType, int arraySize);

	static bool SerializeUpdateFirmwareDataAck(UpdateFirmwareDataAck* packet, FixedStringBase& data);
	static bool SerializePacket(PacketBase *packet, FixedStringBase & data);

	static bool SerializeThingSessionCreate(ThingSessionCreatePacket *thingSessionCreate, FixedStringBase &outputBuffer);
	static bool SerializeUpdateNodesPacket(UpdateNodesPacket* updateNodesPacket, FixedStringBase &data);

	static void SerializeNodeList(cmp_ctx_t& cmp, std::vector<Node*>& nodeList);
	static void SerializeNodeAttributes(cmp_ctx_t & cmp, Node * node);
	static void SerializeNode(cmp_ctx_t & cmp, Node *node);
	static bool SerializeSessionCreateAck(FixedStringBase &data);
	static bool DeserializeNodeId(cmp_ctx_t & cmp, NodeId & nodeId);
	static bool SerializeNodeId(cmp_ctx_t& cmp, NodeId& nodeId);
	static bool SerializeNodeValue(cmp_ctx_t& cmp, NodeValue &nodeValue);
	static bool DeserializeNodeValue(cmp_ctx_t &cmp, NodeValue &nodeValue);
	static bool SerializeUpdateResult(cmp_ctx_t& cmp, DeviceNodeUpdateResult& updateResult);
    static bool ReadWifiNetwork(cmp_ctx_t& cmp, WifiNetworkPacket& wifiNetwork);

	static bool WriteString(cmp_ctx_t & cmp, FixedStringBase & str);

	static ThingSessionCreateAckPacket* DeserializeThingSessionCreateAck(cmp_ctx_t& cmp);
	static PacketBase* DeserializeUpdateFirmwareCommitToThing(cmp_ctx_t& cmp);
	static PacketBase* DeserializeUpdateFirmwareBeginToThing(cmp_ctx_t& cmp);
	static PacketBase* DeserializeZeroConfigurationPacket(cmp_ctx_t& cmp);

	static PacketBase* DeserializePacket(FixedStringBase &data);
	static HeartbeatPacket* DeserializeHeartbeat(cmp_ctx_t& cmp);
	static AckToDevicePacket* DeserializeAckToDevice(cmp_ctx_t& cmp);
	static UpdateNodesFromClientPacket* DeserializeUpdateNodesFromClient(cmp_ctx_t& cmp);
};

#endif

