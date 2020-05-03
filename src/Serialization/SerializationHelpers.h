#ifndef _SERIALIZATION_HELPERS_H
#define _SERIALIZATION_HELPERS_H

#include <FixedString.h>
#include "Logging/Logger.h"

#include "cmp.h"
class SerializationHelpers
{
	static Logger& _logger;
public:
	static const char *cmpTypeToStr(uint8_t cmpType);
	static bool ReadCmpString(cmp_ctx_t& cmp, FixedStringBase &str);
	static bool ReadCmpBin(cmp_ctx_t& cmp, FixedStringBase &str);
	static size_t FileWriter(cmp_ctx_t *ctx, const void *data, size_t count);
	static bool FileReader(cmp_ctx_t *ctx, void *data, size_t count);
};

#endif

