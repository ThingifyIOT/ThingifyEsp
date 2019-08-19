#ifndef _SERIALIZATION_HELPERS_H
#define _SERIALIZATION_HELPERS_H

#include <FixedString.h>
#include "Logging/ContiLogger.h"

#include "cmp.h"
class SerializationHelpers
{
	static ContiLogger& _logger;
	public:
	 static const char *cmpTypeToStr(uint8_t cmpType);
	 static bool ReadCmpString(cmp_ctx_t& cmp, FixedStringBase &str);
	 static bool ReadCmpBin(cmp_ctx_t& cmp, FixedStringBase &str);
};

#endif

