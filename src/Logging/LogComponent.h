#ifndef _LOG_COMPONENT_H
#define _LOG_COMPONENT_H

enum class LogComponent
{
	None = 0,
	Mqtt = 1,
	Packet = 2,
	Network = 4,
	Node = 16,
	Sensor = 32,
	Other = 64
};
#endif