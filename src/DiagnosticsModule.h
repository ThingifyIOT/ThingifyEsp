#pragma once

#include <Thingify.h>
#include <IModule.h>
#include <ThingifyUtils.h>

class DiagnosticsModule: public IModule
{
	Node
		*ipNode,
		*gatewayIpNode,
		*ssidNode,
		*bssidNode,
		*signalStrengthNode,
		*freeHeapNode,
		*uptimeNode,
		*crashNode,
		*reconnectCountNode,
		*connectionDurationNode,
		*restartReasonNode;

	Thingify& _dev;
	int DbmToPercent(int dBm);
public:
	DiagnosticsModule(Thingify& dev);

	bool ShowFreeHeap;
	bool ShowIp;
	bool ShowGatewayIp;
	bool ShowSsid;
	bool ShowBssid;
	bool ShowSignalStrength;
	bool ShowUptime;
	bool ShowCrashNode;
	bool ShowConnectionDuration;
	bool ShowReconnectCount;
#if ESP8266
	bool ShowFreeStack;
	bool ShowStackFragmentation;
#endif

	int UpdateIntervalInMs;

	const char* GetName() override;
	bool Init() override;
	bool Tick() override;
};

