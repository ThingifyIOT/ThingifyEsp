#ifndef _PACKET_LOGGER_H
#define _PACKET_LOGGER_H

#include <pgmspace.h>
#include "Api/PacketBase.h"


#include "Logging/ContiLogger.h"

class PacketLogger
{
	static ContiLogger& _logger;
public:
	static void LogPacket(const char* prefix, PacketBase* packet);
};

#endif

