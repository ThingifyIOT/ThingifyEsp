#include "Serializer.h"
#include <ThingifyUtils.h>
#include "Api/PacketBase.h"
#include "Node/Node.h"
#include "Node/NodeId.h"
#include "Api/HeartbeatPacket.h"
#include "Api/AckToDevicePacket.h"
#include "SerializationHelpers.h"
#include "Api/ThingifyPacketType.h"
#include "Api/UpdateFirmwareBeginToThingPacket.h"
#include "Api/UpdateFirmwareCommitToThingPacket.h"
#include "Api/UpdateFirmwareDataAck.h"
#include "Api/ZeroConfigurationPacket.h"


bool Serializer::SerializePacket(PacketBase* packet, FixedStringBase& outputBuffer)
{
	switch (packet->PacketType())
	{
		case ThingifyPacketType::ThingSessionCreate:
		{
			auto thingSessionCreatePacket = static_cast<ThingSessionCreatePacket*>(packet);
			return SerializeThingSessionCreate(thingSessionCreatePacket, outputBuffer);
			break;
		}
		case ThingifyPacketType::UpdateNodes:
		{
			auto updateNodesPacket = static_cast<UpdateNodesPacket*>(packet);
			return SerializeUpdateNodesPacket(updateNodesPacket, outputBuffer);
			break;
		}
		case ThingifyPacketType::ClientReceivedCreateSessionAck:
		{
			return SerializeSessionCreateAck(outputBuffer);
		}
		case ThingifyPacketType::UpdateFirmwareDataAck:
		{
			auto updateFirmwareAck = static_cast<UpdateFirmwareDataAck*>(packet);
			return SerializeUpdateFirmwareDataAck(updateFirmwareAck, outputBuffer);
		}
		case ThingifyPacketType::ZeroConfigurationResponsePacket:
		{
			return SerializeZeroConfigurationResponse(outputBuffer);
		}
		default:
			_logger.err(L("Failed to serialize, unknown packet type"));
			return false;
	}
	return false;
}

PacketBase* Serializer::DeserializePacket(FixedStringBase& data)
{
	BufferReader bufferReader(data.c_str(), data.length());
	cmp_ctx_t cmp;
	cmp_init(&cmp, &bufferReader, FileReader, FileWriter);

	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		_logger.err(L("Failed to read packet array size #1"));
		return nullptr;
	}

	if (arraySize != 2)
	{
		return nullptr;
	}

	int packetTypeInteger;

	if (!cmp_read_int(&cmp, &packetTypeInteger))
	{
		_logger.err(L("Failed to read packet type"));
		return nullptr;
	}

	const auto packetType = static_cast<ThingifyPacketType>(packetTypeInteger);

	if (packetType == ThingifyPacketType::Heartbeat)
	{
		return DeserializeHeartbeat(cmp);
	}
	if (packetType == ThingifyPacketType::AckToDevice)
	{
		return DeserializeAckToDevice(cmp);
	}
	if (packetType == ThingifyPacketType::UpdateNodesFromClient)
	{
		return DeserializeUpdateNodesFromClient(cmp);
	}
	if (packetType == ThingifyPacketType::ThingSessionCreateAck)
	{
		return DeserializeThingSessionCreateAck(cmp);
	}
	if(packetType == ThingifyPacketType::UpdateFirmwareCommitToThing)
	{
		return DeserializeUpdateFirmwareCommitToThing(cmp);
	}
	if (packetType == ThingifyPacketType::UpdateFirmwareBeginToThing)
	{
		return DeserializeUpdateFirmwareBeginToThing(cmp);
	}
	if(packetType == ThingifyPacketType::ZeroConfigurationPacket)
	{
		return DeserializeZeroConfigurationPacket(cmp);
	}
	return nullptr;
}

bool Serializer::SerializeThingSessionCreate(ThingSessionCreatePacket* thingSessionCreate, FixedStringBase& outputBuffer)
{
	cmp_ctx_t cmp;
	cmp_init(&cmp, &outputBuffer, nullptr, FileWriter);
	if(!WriteArrayPacketHeader(cmp, ThingifyPacketType::ThingSessionCreate, 4))
	{
		return false;
	}

	WriteString(cmp, thingSessionCreate->ClientId);
	WriteString(cmp, thingSessionCreate->LoginToken);
	WriteString(cmp, thingSessionCreate->FirmwareVersion);
	cmp_write_array(&cmp, 1);
	cmp_write_array(&cmp, 3);
	WriteString(cmp, thingSessionCreate->ClientId);
	WriteString(cmp, thingSessionCreate->DeviceName);
	SerializeNodeList(cmp, thingSessionCreate->Nodes);
	return cmp.error == 0;
}

ThingSessionCreateAckPacket* Serializer::DeserializeThingSessionCreateAck(cmp_ctx_t& cmp)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		return nullptr;
	}

	if (arraySize != 4)
	{
		return nullptr;
	}

	int responseType = 0;
	if (!cmp_read_int(&cmp, &responseType))
	{
		return nullptr;
	}
	auto thingSessionCreateAckPacket = new ThingSessionCreateAckPacket();
	thingSessionCreateAckPacket->Type = static_cast<LoginResponseType>(responseType);

	if (!SerializationHelpers::ReadCmpString(cmp, thingSessionCreateAckPacket->ClientInServerOutTopic))
	{
		_logger.err(L("Failed to deserialize ClientInServerOutTopic"));
		delete thingSessionCreateAckPacket;
		return nullptr;
	}
	if (!SerializationHelpers::ReadCmpString(cmp, thingSessionCreateAckPacket->ClientOutServerInTopic))
	{
		delete thingSessionCreateAckPacket;
		return nullptr;
	}
	if (!cmp_read_long(&cmp, &thingSessionCreateAckPacket->CurrentTime))
	{
		delete thingSessionCreateAckPacket;
		return nullptr;
	}
	return thingSessionCreateAckPacket;
}


PacketBase* Serializer::DeserializeUpdateFirmwareBeginToThing(cmp_ctx_t& cmp)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		return nullptr;
	}

	if (arraySize != 3)
	{
		return nullptr;
	}
	int correlationId = 0;
	if (!cmp_read_int(&cmp, &correlationId))
	{
		return nullptr;
	}
	uint64_t firmwareSize;
	if (!cmp_read_ulong(&cmp, &firmwareSize))
	{
		return nullptr;
	}
	FixedString50 firmwareMd5;
	if (!SerializationHelpers::ReadCmpString(cmp, firmwareMd5))
	{
		return nullptr;
	}
	auto firmwareUpdateBeginPacket = new UpdateFirmwareBeginToThingPacket();
	firmwareUpdateBeginPacket->CorrelationId = correlationId;
	firmwareUpdateBeginPacket->FirmwareSize = firmwareSize;
	firmwareUpdateBeginPacket->FirmwareMd5 = firmwareMd5;
	return firmwareUpdateBeginPacket;
}

PacketBase* Serializer::DeserializeZeroConfigurationPacket(cmp_ctx_t& cmp)
{
	uint32_t mapSize;
	if(!cmp_read_map(&cmp, &mapSize))
	{
		return nullptr;
	}
	auto packet = new ZeroConfigurationPacket();
	for(uint32_t i =0; i < mapSize; i++)
	{
		FixedString50 key;
		if(!SerializationHelpers::ReadCmpString(cmp, key))
		{
			delete packet;
			return nullptr;
		}
		if(key.equals("token"))
		{
			SerializationHelpers::ReadCmpString(cmp, packet->Token);
		}
		else if(key.equals("api_address"))
		{
			SerializationHelpers::ReadCmpString(cmp, packet->ApiAddress);
		}
		else if(key.equals("wifi_networks"))
		{
			uint32_t wifiArraySize;
			cmp_read_array(&cmp, &wifiArraySize);
			while (wifiArraySize-- > 0)
			{
				auto wifiNetwork = new WifiNetworkPacket();
				if(!ReadWifiNetwork(cmp, *wifiNetwork))
				{
					delete packet;
					return nullptr;
				}
				packet->WifiNetworks.push_back(wifiNetwork);
			}			
		}
	}
	return packet;
}

bool Serializer::ReadWifiNetwork(cmp_ctx_t& cmp, WifiNetworkPacket& wifiNetwork)
{
	uint32_t mapSize;
	if(!cmp_read_map(&cmp, &mapSize))
	{
		return false;
	}
	for(uint32_t i =0; i < mapSize; i++)
	{
		FixedString50 key;
		if(!SerializationHelpers::ReadCmpString(cmp, key))
		{
			return false;
		}
		if(key.equals("name"))
		{
			SerializationHelpers::ReadCmpString(cmp, wifiNetwork.Name);
		}
		else if(key.equals("password"))
		{
			SerializationHelpers::ReadCmpString(cmp, wifiNetwork.Password);
		}
	}
	return true;
}

PacketBase* Serializer::DeserializeUpdateFirmwareCommitToThing(cmp_ctx_t& cmp)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		return nullptr;
	}

	if (arraySize != 1)
	{
		return nullptr;
	}
	int correlationId = 0;
	if (!cmp_read_int(&cmp, &correlationId))
	{
		return nullptr;
	}	

	auto updateFirmwarePacket = new UpdateFirmwareCommitToThingPacket();
	updateFirmwarePacket->CorrelationId = correlationId;
	return updateFirmwarePacket;
}

bool Serializer::SerializeSessionCreateAck(FixedStringBase &data)
{
	cmp_ctx_t cmp;
	cmp_init(&cmp, &data, nullptr, FileWriter);

	WriteArrayPacketHeader(cmp, ThingifyPacketType::ClientReceivedCreateSessionAck, 0);
	return cmp.error == 0;
}
bool Serializer::SerializeUpdateFirmwareDataAck(UpdateFirmwareDataAck* packet, FixedStringBase& data)
{
	cmp_ctx_t cmp;
	cmp_init(&cmp, &data, nullptr, FileWriter);

	WriteArrayPacketHeader(cmp, ThingifyPacketType::UpdateFirmwareDataAck, 4);
	if (!cmp_write_integer(&cmp, packet->CorrelationId))
	{
		return false;
	}
	if (!cmp_write_bool(&cmp, packet->IsSuccess))
	{
		return false;
	}
	if (!WriteString(cmp, packet->ErrorString))
	{
		return false;
	}
	if (!cmp_write_u32(&cmp, packet->MaxChunkSize))
	{
		return false;
	}
	return cmp.error == 0;
}

bool Serializer::SerializeZeroConfigurationResponse(FixedStringBase &data)
{
	cmp_ctx_t cmp;
	cmp_init(&cmp, &data, nullptr, FileWriter);
	WriteArrayPacketHeader(cmp, ThingifyPacketType::ZeroConfigurationResponsePacket, 0);
	return cmp.error == 0;
}

bool Serializer::SerializeUpdateNodesPacket(UpdateNodesPacket* updateNodesPacket, FixedStringBase& data)
{
	cmp_ctx_t cmp;

	cmp_init(&cmp, &data, 0, FileWriter);

	if (!WriteArrayPacketHeader(cmp, ThingifyPacketType::UpdateNodes, 6))
	{
		return false;
	}

	// 0 - packet id
	if (!cmp_write_integer(&cmp, updateNodesPacket->PacketId))
	{
		return false;
	}
	// 1 - new nodes array
	if (!cmp_write_array(&cmp, 0))
	{
		return false;
	}
	// 2 - removed nodes array
	if (!cmp_write_array(&cmp, 0))
	{
		return false;
	}

	// 3 - updated nodes array
	if (!cmp_write_array(&cmp, updateNodesPacket->UpdatedNodes.size()))
	{
		return false;
	}

	for (auto& valueUpdate : updateNodesPacket->UpdatedNodes)
	{
		if (!cmp_write_array(&cmp, 2))
		{
			return false;
		}

		if (!SerializeNodeId(cmp, valueUpdate.nodeId))
		{
			return false;
		}

		if (!SerializeNodeValue(cmp, valueUpdate.Value))
		{
			return false;
		}
	}

	// 4 - update confirms
	if (!cmp_write_array(&cmp, updateNodesPacket->UpdateResults.size()))
	{
		return false;
	}
	for (auto& updateResultItem : updateNodesPacket->UpdateResults)
	{
		if (!cmp_write_array(&cmp, 2))
		{
			return false;
		}

		if (!SerializeNodeId(cmp, updateResultItem.nodeId))
		{
			_logger.warn(L("Failed to serialize node id"));
			return false;
		}
		if (!SerializeUpdateResult(cmp, updateResultItem.UpdateResult))
		{
			_logger.warn(L("Failed to serialize update result"));
			return false;
		}
	}
	//5 - function executions
	if (!cmp_write_array(&cmp, updateNodesPacket->FunctionExecutionResponses.size()))
	{
		return false;
	}
	for (auto& functionResponse : updateNodesPacket->FunctionExecutionResponses)
	{
		if (!cmp_write_array(&cmp, 5))
		{
			return false;
		}

		if (!SerializeNodeId(cmp, functionResponse.nodeId))
		{
			_logger.warn(L("Failed to serialize node id"));
			return false;
		}

		if (!cmp_write_int(&cmp, functionResponse.FunctionId))
		{
			_logger.warn(L("Failed to serialize function id"));
			return false;
		}
		if (!cmp_write_bool(&cmp, functionResponse.IsSuccess))
		{
			_logger.warn(L("Failed to serialize function is success"));
			return false;
		}

		SerializeNodeValue(cmp, functionResponse.ReturnValue);		

		if(!WriteString(cmp, functionResponse.ErrorMessage))
		{
			_logger.warn(L("Failed to serialize function error"));
			return false;
		}
	}

	return true;
}

bool Serializer::SerializeUpdateResult(cmp_ctx_t &cmp, DeviceNodeUpdateResult &updateResult)
{
	if (updateResult.ResultType == DeviceNodeUpdateResultType::Success)
	{
		if (!cmp_write_array(&cmp, 1))
		{
			return false;
		}
		if (!cmp_write_u8(&cmp, static_cast<uint8_t>(updateResult.ResultType)))
		{
			return false;
		}
	}
	if (updateResult.ResultType == DeviceNodeUpdateResultType::Failure)
	{
		if (!cmp_write_array(&cmp, 2))
		{
			return false;
		}
		if (!cmp_write_u8(&cmp, static_cast<uint8_t>(updateResult.ResultType)))
		{
			return false;
		}
		FixedString<10> ErrorMessage;
		WriteString(cmp, ErrorMessage);
	}
	return true;
}


AckToDevicePacket* Serializer::DeserializeAckToDevice(cmp_ctx_t&cmp)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		return nullptr;
	}
	if (arraySize != 1)
	{
		return nullptr;
	}
	int packetId;
	if (!cmp_read_int(&cmp, &packetId))
	{
		return nullptr;
	}
	const auto ackToDevice = new AckToDevicePacket;
	ackToDevice->PacketId = packetId;
	return ackToDevice;
}

UpdateNodesFromClientPacket* Serializer::DeserializeUpdateNodesFromClient(cmp_ctx_t& cmp)
{
	uint32_t updateNodesArraySize;
	if (!cmp_read_array(&cmp, &updateNodesArraySize))
	{
		_logger.err(L("Failed to read packet array size"));
		return nullptr;
	}

	if (updateNodesArraySize < 3)
	{
		return nullptr;
	}

	int packetId;
	if (!cmp_read_int(&cmp, &packetId))
	{
		return nullptr;
	}


	uint32_t updatesCount;
	if (!cmp_read_array(&cmp, &updatesCount))
	{
		_logger.err(L("Failed to read updates count"));
		return nullptr;
	}

	auto* packet = new UpdateNodesFromClientPacket;
	packet->PacketId = packetId;
	for (uint32_t i = 0; i < updatesCount; i++)
	{
		uint32_t updateItemArraySize;
		if (!cmp_read_array(&cmp, &updateItemArraySize))
		{
			_logger.err(L("failed to read item array size"));
			delete packet;
			return nullptr;
		}

		if (updateItemArraySize != 2)
		{
			_logger.err(L("wrong update array size"));
			delete packet;
			return nullptr;
		}
		NodeId nodeId;
		if (!DeserializeNodeId(cmp, nodeId))
		{
			_logger.err(L("Failed to read node id"));
			delete packet;
			return nullptr;
		}

		NodeValue nodeValue(ValueType::Bool);
		if (!DeserializeNodeValue(cmp, nodeValue))
		{
			_logger.err(L("Failed to read node value"));
			delete packet;
			return nullptr;
		}
		NodeUpdate nodeUpdate(nodeId, nodeValue);

		packet->Updates.add(nodeUpdate);
	}

	uint32_t functionExecutionsCount;
	if (!cmp_read_array(&cmp, &functionExecutionsCount))
	{
		_logger.err(L("Failed to read function executions count"));
		return nullptr;
	}
	for (uint32_t i = 0; i < functionExecutionsCount; i++)
	{
		uint32_t functionExecutionArraySize;
		if (!cmp_read_array(&cmp, &functionExecutionArraySize))
		{
			_logger.err(L("failed to read function array size"));
			delete packet;
			return nullptr;
		}

		if (functionExecutionArraySize != 3)
		{
			_logger.err(L("wrong update array size"));
			delete packet;
			return nullptr;
		}

		FunctionExecutionRequestItem functionExecutionRequest;

		if (!DeserializeNodeId(cmp, functionExecutionRequest.nodeId))
		{
			_logger.err(L("Failed to read node id"));
			delete packet;
			return nullptr;
		}
		
		if (!cmp_read_ulong(&cmp, &(functionExecutionRequest.FunctionId)))
		{
			_logger.err(L("Failed to read function id"));
			delete packet;
			return nullptr;
		}

		uint32_t argumentsArraySize;
		if (!cmp_read_array(&cmp, &argumentsArraySize))
		{
			_logger.err(L("Failed to read function arg array size"));
			return nullptr;
		}
		for (uint32_t j = 0; j < argumentsArraySize; j++)
		{
			NodeValue argument(ValueType::Bool);
			if (!Serializer::DeserializeNodeValue(cmp, argument))
			{
				_logger.err(L("Failed to deserialize argument %d"), j);
				delete packet;
				return nullptr;
			}
			functionExecutionRequest.Arguments.add(argument);
		}
		packet->FunctionExecutionRequests.add(functionExecutionRequest);
	}

	return packet;
}

HeartbeatPacket* Serializer::DeserializeHeartbeat(cmp_ctx_t&cmp)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		return nullptr;
	}
	if (arraySize != 1)
	{
		return nullptr;
	}
	int heartbearNumber;
	if (!cmp_read_int(&cmp, &heartbearNumber))
	{
		return nullptr;
	}
	const auto heartbeat = new HeartbeatPacket;
	heartbeat->HeartbeatNumber = heartbearNumber;
	return heartbeat;
}

void Serializer::SerializeNode(cmp_ctx_t& cmp, Node* node)
{
	cmp_write_array(&cmp, 7);

	cmp_write_str(&cmp, node->name(), strlen(node->name()));
	cmp_write_integer(&cmp, static_cast<uint8_t>(node->unit())); // unit type

	cmp_write_u8(&cmp, static_cast<uint8_t>(node->Type));

	SerializeNodeAttributes(cmp, node);

	cmp_write_bool(&cmp, node->isReadOnly());

	SerializeNodeValue(cmp, node->Value);
	cmp_write_u8(&cmp, static_cast<uint8_t>(node->kind()));
}

void Serializer::SerializeNodeAttributes(cmp_ctx_t& cmp, Node* node)
{
	if(node->Type == NodeType::Range)
	{
		cmp_write_array(&cmp, 2);
		cmp_write_int(&cmp, 0);
		cmp_write_array(&cmp, 3);

		NodeValue valueMin = NodeValue::Int(node->_rangeMin);
		NodeValue valueMax = NodeValue::Int(node->_rangeMax);
		NodeValue valueStep = NodeValue::Int(node->_rangeStep);

		SerializeNodeValue(cmp, valueMin);
		SerializeNodeValue(cmp, valueMax);
		SerializeNodeValue(cmp, valueStep);
	}
	else
	{
		cmp_write_nil(&cmp);
	}
}
bool Serializer::SerializeNodeValue(cmp_ctx_t& cmp, NodeValue &nodeValue)
{
	cmp_write_array(&cmp, 2);
	cmp_write_integer(&cmp, static_cast<int>(nodeValue.Type));

	if (nodeValue.IsNull())
	{
		cmp_write_array(&cmp, 0);
		return true;
	}

	switch (nodeValue.Type)
	{
	case ValueType::Bool:
		cmp_write_bool(&cmp, nodeValue.AsBool());
		break;
	case ValueType::Int:
		cmp_write_int(&cmp, nodeValue.AsInt());
		break;
	case ValueType::String:
		if (nodeValue.AsString() == nullptr)
		{
			cmp_write_str(&cmp, "", 0);
			break;
		}
		cmp_write_str(&cmp, nodeValue.AsString(), strlen(nodeValue.AsString()));
		break;
	case ValueType::Float:
		cmp_write_double(&cmp, nodeValue.AsFloat());
		break;
	case ValueType::Color:
		cmp_write_array(&cmp, 4);
		cmp_write_u8(&cmp, nodeValue._colorValue.R);
		cmp_write_u8(&cmp, nodeValue._colorValue.G);
		cmp_write_u8(&cmp, nodeValue._colorValue.B);
		cmp_write_u8(&cmp, nodeValue._colorValue.A);
		break;
	case ValueType::TimeSpan:
		cmp_write_u64(&cmp, nodeValue._timespanValue);
		break;
	case ValueType::DateTime:
	case ValueType::Location:
		_logger.err(L("Not implemented value type"));
			break;
	default:
		_logger.err(L("Unknown value type"));
		break;
	}
	return true;
}

void Serializer::SerializeNodeList(cmp_ctx_t& cmp, std::vector<Node*>& nodeList)
{
	const int nodeCount = nodeList.size();
	cmp_write_array(&cmp, nodeCount);
	for (int i = 0; i < nodeCount; i++)
	{
		SerializeNode(cmp, nodeList[i]);
	}
}

bool Serializer::WriteString(cmp_ctx_t& cmp, FixedStringBase &str)
{
	return cmp_write_str(&cmp, str.c_str(), str.length());
}

bool Serializer::DeserializeNodeId(cmp_ctx_t &cmp, NodeId &nodeId)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		_logger.err(L("Failed to read packet array size"));
		return false;
	}

	if (arraySize != 2)
	{
		return false;
	}
	if(!SerializationHelpers::ReadCmpString(cmp, nodeId.DeviceId))
	{
		return false;
	}
	if(!SerializationHelpers::ReadCmpString(cmp, nodeId.NodeName))
	{
		return false;
	}

	return true;
}

bool Serializer::SerializeNodeId(cmp_ctx_t &cmp, NodeId &nodeId)
{
	if (!cmp_write_array(&cmp, 2))
	{
		_logger.err(L("Failed to read packet array size"));
		return false;
	}

	if(!cmp_write_str(&cmp, nodeId.DeviceId.c_str(), nodeId.DeviceId.length()))
	{
		return false;
	}
	if (!cmp_write_str(&cmp, nodeId.NodeName.c_str(), nodeId.NodeName.length()))
	{
		return false;
	}	

	return true;
}
bool Serializer::DeserializeNodeValue(cmp_ctx_t &cmp, NodeValue &nodeValue)
{
	uint32_t arraySize;
	if (!cmp_read_array(&cmp, &arraySize))
	{
		_logger.err(L("Failed to read value array size"));
		return false;
	}

	if (arraySize != 2)
	{
		_logger.err(L("Wrong value array size"));
		return false;
	}

	int valueTypeInt;

	if(!cmp_read_int(&cmp, &valueTypeInt))
	{
		_logger.err(L("failed to read value type"));
		return false;
	}

	const auto type = static_cast<ValueType>(valueTypeInt);

	unsigned int stringSize = 100;
	int nodeValueInteger;
	nodeValue.Type = type;
	switch (type)
	{
	case ValueType::Bool:
		bool boolValue;
		
		if (!cmp_read_bool(&cmp, &boolValue))
		{
			_logger.err(L("Failed to read bool value"));
			return false;
		}
		nodeValue._boolValue = boolValue;
		return true;
	case ValueType::String:
		char nodeValueString[100];
		if(!cmp_read_str(&cmp, nodeValueString, &stringSize))
		{
			_logger.err(L("Failed to read string value"));
			return false;
		}
		nodeValue.stringValue = FixedString<ThingifyConstants::MaxStringValueSize>(nodeValueString);
		return true;
	case ValueType::Int:

		if(!cmp_read_int(&cmp, &nodeValueInteger))
		{
			_logger.err(L("Failed to read int value"));

			return false;
		}		
		nodeValue._intValue = nodeValueInteger;
		return true;
	case ValueType::Float:
		float nodeValueFloat;
		int nodeValueInt;
		if (!cmp_read_float(&cmp, &nodeValueFloat))
		{
			if (!cmp_read_int(&cmp, &nodeValueInt))
			{
				_logger.err(L("Failed to deserialize float node "));
				return false;
			}
			nodeValueFloat = nodeValueInt;
		}
		nodeValue._floatValue = nodeValueFloat;
		return true;
	case ValueType::Color:
	{
		uint32_t colorArraySize = 0;
		if (!cmp_read_array(&cmp, &colorArraySize))
		{
			Serial.println(" # 1");
			return false;
		}
		if (colorArraySize != 4)
		{
			Serial.println(" # 2");

			return false;
		}
		int r, g, b, a;
		if (cmp_read_int(&cmp, &r) &&
			cmp_read_int(&cmp, &g) &&
			cmp_read_int(&cmp, &b) &&
			cmp_read_int(&cmp, &a))
		{
			nodeValue._colorValue.R = r;
			nodeValue._colorValue.G = g;
			nodeValue._colorValue.B = b;
			nodeValue._colorValue.A = a;

			return true;
		}
		Serial.println(" # 3");

		return false;
	}
	default:
		_logger.err(L("Unknown node type"));
		return false;
	}
}

bool Serializer::WritePacketHeader(cmp_ctx_t &cmp, ThingifyPacketType packetType)
{
	if (!cmp_write_array(&cmp, 2))
	{
		return false;
	}

	if (!cmp_write_integer(&cmp, static_cast<uint8_t>(packetType))) // 4 - UpdateNodesPacket
	{
		return false;
	}

	return true;
}

bool Serializer::WriteArrayPacketHeader(cmp_ctx_t &cmp, ThingifyPacketType packetType, int arraySize)
{
	if(!WritePacketHeader(cmp, packetType))
	{
		return false;
	}
	if (!cmp_write_array(&cmp, arraySize))
	{
		return false;
	}
	return true;
}

bool Serializer::FileReader(cmp_ctx_t *ctx, void *data, size_t count)
{
	BufferReader *bufferReader = reinterpret_cast<BufferReader*>(ctx->buf);

	const char* readBytes = bufferReader->ReadBuffer(count);
	if (readBytes == nullptr)
	{
		_logger.err(L("Read bytes returned null"));
		return false;
	}

	memcpy(data, readBytes, count);

	return true;
}


size_t Serializer::FileWriter(cmp_ctx_t *ctx, const void *data, size_t count)
{
	FixedStringBase *str = reinterpret_cast<FixedStringBase*>(ctx->buf);
	str->append(reinterpret_cast<const char*>(data), count);
	return count;
}


Logger& Serializer::_logger = LoggerInstance;



