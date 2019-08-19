#include "SerializationHelpers.h"

ContiLogger& SerializationHelpers::_logger = ContiLoggerInstance;

const char* SerializationHelpers::cmpTypeToStr(uint8_t cmpType)
{
	switch (cmpType)
	{
		case CMP_TYPE_POSITIVE_FIXNUM: return "CMP_TYPE_POSITIVE_FIXNUM";
		case CMP_TYPE_FIXMAP: return "CMP_TYPE_FIXMAP";
		case CMP_TYPE_FIXARRAY: return "CMP_TYPE_FIXARRAY";
		case CMP_TYPE_FIXSTR: return "CMP_TYPE_FIXSTR";
		case CMP_TYPE_NIL: return "CMP_TYPE_NIL";
		case CMP_TYPE_BOOLEAN: return "CMP_TYPE_BOOLEAN";
		case CMP_TYPE_BIN8: return "CMP_TYPE_BIN8";
		case CMP_TYPE_BIN16: return "CMP_TYPE_BIN16";
		case CMP_TYPE_BIN32: return "CMP_TYPE_BIN32";
		case CMP_TYPE_EXT8: return "CMP_TYPE_EXT8";
		case CMP_TYPE_EXT16: return "CMP_TYPE_EXT16";
		case CMP_TYPE_EXT32: return "CMP_TYPE_EXT32";
		case CMP_TYPE_FLOAT: return "CMP_TYPE_FLOAT";
		case CMP_TYPE_DOUBLE: return "CMP_TYPE_DOUBLE";
		case CMP_TYPE_UINT8: return "CMP_TYPE_UINT8";
		case CMP_TYPE_UINT16: return "CMP_TYPE_UINT16";
		case CMP_TYPE_UINT32: return "CMP_TYPE_UINT32";
		case CMP_TYPE_UINT64: return "CMP_TYPE_UINT64";
		case CMP_TYPE_SINT8: return "CMP_TYPE_SINT8";
		case CMP_TYPE_SINT16: return "CMP_TYPE_SINT16";
		case CMP_TYPE_SINT32: return "CMP_TYPE_SINT32";
		case CMP_TYPE_SINT64: return "CMP_TYPE_SINT64";
		case CMP_TYPE_FIXEXT1: return "CMP_TYPE_FIXEXT1";
		case CMP_TYPE_FIXEXT2: return "CMP_TYPE_FIXEXT2";
		case CMP_TYPE_FIXEXT4: return "CMP_TYPE_FIXEXT4";
		case CMP_TYPE_FIXEXT8: return "CMP_TYPE_FIXEXT8";
		case CMP_TYPE_FIXEXT16: return "CMP_TYPE_FIXEXT16";
		case CMP_TYPE_STR8: return "CMP_TYPE_STR8";
		case CMP_TYPE_STR16: return "CMP_TYPE_STR16";
		case CMP_TYPE_STR32: return "CMP_TYPE_STR32";
		case CMP_TYPE_ARRAY16: return "CMP_TYPE_ARRAY16";
		case CMP_TYPE_ARRAY32: return "CMP_TYPE_ARRAY32";
		case CMP_TYPE_MAP16: return "CMP_TYPE_MAP16";
		case CMP_TYPE_MAP32: return "CMP_TYPE_MAP32";
		case CMP_TYPE_NEGATIVE_FIXNUM: return "CMP_TYPE_NEGATIVE_FIXNUM";
		default: return "UNKNOWN";
	}
}

bool SerializationHelpers::ReadCmpString(cmp_ctx_t& cmp, FixedStringBase &str)
{
	cmp_object_t obj;

	if (!cmp_read_object(&cmp, &obj))
	{
		
		_logger.err(L("Failed to read cmp object in ReadCmpString"));
		return false;
	}

	uint32_t strSize;
	switch (obj.type) 
	{
	case CMP_TYPE_FIXSTR:
	case CMP_TYPE_STR8:
	case CMP_TYPE_STR16:
	case CMP_TYPE_STR32:
		strSize = obj.as.str_size;
		break;
	case CMP_TYPE_NIL:
		strSize = 0;
		break;
	default:
		return false;
	}


	if (strSize > str.capacity())
	{
		_logger.err(L("capacity too low"));
		return false;
	}

	str.clear();

	while (strSize--)
	{
		int8_t c;
		if (!cmp_read_char(&cmp, &c))
		{
			_logger.err(L("cmp_read_char failed"));
			return false;
		}
		if (!str.append(c))
		{
			_logger.err(L("failed to append"));
			return false;
		}
	}
	return true;
}


bool SerializationHelpers::ReadCmpBin(cmp_ctx_t& cmp, FixedStringBase &str)
{
	uint32_t strSize;

	if (!cmp_read_str_size(&cmp, &strSize))
	{
		_logger.err(L("Failed to read bin size"));
		return false;
	}

	if (strSize > str.capacity())
	{
		return false;
	}
	str.clear();

	while (strSize--)
	{
		int8_t c;
		char data[2];
		if (!cmp.read(&cmp, data, 1))
		{
			return false;
		}

		if (!str.append(data[0]))
		{
			return false;
		}
	}
	return true;
}