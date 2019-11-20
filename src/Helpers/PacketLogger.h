#ifndef _PACKET_LOGGER_H
#define _PACKET_LOGGER_H

#include <pgmspace.h>
#include "Api/PacketBase.h"


#include "Logging/Logger.h"

class PacketLogger
{
	static Logger& _logger;
public:
	static void LogPacket(const char* prefix, PacketBase* packet);
};

#endif

