#ifndef  _THING_STATE_H
#define _THING_STATE_H

enum class ThingState: uint8_t
{
	Disabled,
	NotConfigured,
	Configuring,
	SearchingForNetwork,
	ConnectingToNetwork,
	ConnectingToMqtt,
	Online,
	DisconnectedFromMqtt,
	Authenticating,
	ServiceUnavailable,
	Error
};

#endif // ! _THING_STATE_H


