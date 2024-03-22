#include "LogUtils.h"

const char* LogUtils::LogLevelToString(LogLevel level)
{
	switch (level)
	{
		case LogLevel::Debug: return "D";
		case LogLevel::Info: return "I";
		case LogLevel::Error: return "E";
		case LogLevel::Warning: return "W";
		default: return nullptr;
	}
}

const char* LogUtils::ComponentToString(LogComponent component)
{
	switch (component)
	{
		case LogComponent::Mqtt: return "Mqtt";
		case LogComponent::Packet: return "Packet";
		case LogComponent::Network: return "Network";
		case LogComponent::Node: return "Node";
		case LogComponent::Sensor: return "Sensor";

		default: return nullptr;
	}
}
