#include "PacketLogger.h"

#include "Api/ContiPacketType.h"
#include "Api/LoginResponseType.h"
#include "Api/ThingSessionCreateAckPacket.h"
#include "Api/HeartbeatPacket.h"
#include "Api/AckToDevicePacket.h"
#include "Api/UpdateNodesFromClientPacket.h"
#include "Api/UpdateNodesPacket.h"
#include "Api/ThingSessionCreatePacket.h"
#include "Api/UpdateFirmwareToThingPacket.h"
#include "Api/UpdateFirmwareBeginToThingPacket.h"
#include "Api/UpdateFirmwareDataAck.h"

Logger& PacketLogger::_logger = LoggerInstance;

void PacketLogger::LogPacket(const char* prefix, PacketBase* packet)
{
	switch (packet->PacketType())
	{
		case ThingifyPacketType::ThingSessionCreateAck:
		{
			_logger.infoBegin(LogComponent::Packet, L("%s ThingSessionCreateAck "), prefix);
			const auto thingSessionCreateAck = static_cast<ThingSessionCreateAckPacket*>(packet);
			auto sessionCreateResult = thingSessionCreateAck->Type;

			if (sessionCreateResult == LoginResponseType::Success)
			{
				_logger.add(L("in_topic: %s, out_topic: %s"),
					thingSessionCreateAck->ClientInServerOutTopic.c_str(),
					thingSessionCreateAck->ClientOutServerInTopic.c_str());
			}
			else if (sessionCreateResult == LoginResponseType::InvalidCredentials)
			{
				_logger.add(L("Invalid credentials supplied"));
			}
			else if (sessionCreateResult == LoginResponseType::Error)
			{
				_logger.add(L("Other login error"));
			}
			else if (sessionCreateResult == LoginResponseType::ProtocolViolation)
			{
				_logger.add(L("Protocol Violation"));
			}
			else
			{
				_logger.add(L("Login resoponse not success: %d"), sessionCreateResult);
			}
			_logger.end();
			break;
		}
		case ThingifyPacketType::ClientReceivedCreateSessionAck:
		{
			_logger.info(LogComponent::Packet, L("%s ClientReceivedCreateSessionAck"), prefix);
			break;
		}
		case ThingifyPacketType::Heartbeat:	
		{
			const auto heartbeat = static_cast<HeartbeatPacket*>(packet);
			_logger.info(LogComponent::Packet, L("%s Heartbeat, num = %d"), prefix, heartbeat->HeartbeatNumber);
			break;
		}
		case ThingifyPacketType::AckToDevice:
		{
			const auto ackToDevice = static_cast<AckToDevicePacket*>(packet);
			_logger.info(LogComponent::Packet, L("%s AckToDevice, packetId = %d"), prefix, ackToDevice->PacketId);
			break;
		}
		case ThingifyPacketType::UpdateNodesFromClient:
		{
			const auto updatesFromClient = static_cast<UpdateNodesFromClientPacket*>(packet);

			_logger.infoBegin(LogComponent::Packet, L("%s UpdateNodesFromClient id = %d "), 
				prefix, updatesFromClient->PacketId);

			if (updatesFromClient->Updates.size() > 0)
			{
				_logger.add(L("[ updates: %d ] "), updatesFromClient->Updates.size());
			}
			if (updatesFromClient->FunctionExecutionRequests.size() > 0)
			{
				_logger.add(L("[ function requests: %d ] "), updatesFromClient->FunctionExecutionRequests.size());
			}
			_logger.end();
			break;
		}
		case ThingifyPacketType::UpdateNodes:
		{
			const auto updateNodesPacket = static_cast<UpdateNodesPacket*>(packet);
			_logger.infoBegin(LogComponent::Packet, L("%s UpdateNodesPacket "), prefix);

			if (updateNodesPacket->UpdatedNodes.size())
			{
				_logger.add(L("[ updates: %d ] "), updateNodesPacket->UpdatedNodes.size());
			}

			if (updateNodesPacket->UpdateResults.size())
			{
				_logger.add(L("[ results: %d ] "), updateNodesPacket->UpdateResults.size());
			}
			if (updateNodesPacket->FunctionExecutionResponses.size())
			{
				_logger.add(L("[ function responses: %d ] "), updateNodesPacket->FunctionExecutionResponses.size());
			}

			_logger.end();
			break;
		}
		case ThingifyPacketType::ThingSessionCreate:
		{
			const auto thingSessionCreate = static_cast<ThingSessionCreatePacket*>(packet);
			_logger.info(LogComponent::Packet, L("%s ThingSessionCreate token = '%s', clientId = '%s', nodes = %d"), 
				prefix,
				thingSessionCreate->LoginToken.c_str(),
				thingSessionCreate->ClientId.c_str(),
				thingSessionCreate->Nodes.size());
			break;
		}
		case ThingifyPacketType::UpdateFirmwareBeginToThing:
		{
			const auto updateFirmwareBegin = static_cast<UpdateFirmwareBeginToThingPacket*>(packet);
			_logger.info(LogComponent::Packet, L("%s UpdateFirmwareBeginToThing corrId = %d, FirmwareSize = %lld, FirmwareMd5 = %s"),
				prefix,
				updateFirmwareBegin->CorrelationId, updateFirmwareBegin->FirmwareSize, updateFirmwareBegin->FirmwareMd5.c_str());
		}
		break;
		case ThingifyPacketType::UpdateFirmwareCommitToThing:
		{
		const auto updateFirmwarePacket = static_cast<UpdateFirmwareCommitToThingPacket*>(packet);
		_logger.info(LogComponent::Packet, L("%s UpdateFirmwareCommitToThing corrId = %d"),
			prefix,
			updateFirmwarePacket->CorrelationId);
		}
		case ThingifyPacketType::UpdateFirmwareDataAck:
		{
			const auto updateFirmwareAck = static_cast<UpdateFirmwareDataAck*>(packet);
			_logger.info(LogComponent::Packet, L("%s UpdateFirmwareDataAck corrId = %d, isSuccess = %d, errorString = %s"),
				prefix,
				updateFirmwareAck->CorrelationId, updateFirmwareAck->IsSuccess, updateFirmwareAck->ErrorString.c_str());
		}
		break;
		default:
		{
			_logger.err(L("%s [UNKNOWN PACKET TYPE = %d]"), prefix, packet->PacketType());
			break;
		}
	}
}

