#include <Thingify.h>    
#include <IModule.h>
#include <ThingifyUtils.h>
#include <GsmModule.h>
#include <ThingifyGsm.h>

class GsmDiagnosticsModule : public IModule
{
	Node
		*ipNode,
		*gatewayIpNode,
		*networkNameNode,
		*batteryNode,
		*signalQualityNode,
		*batteryVoltageNode,
		*moduleReconnectCountNode,
		*bssidNode,
		*freeHeapNode,
		*uptimeNode,
		*crashNode,
		*reconnectCountNode,
		*connectionDurationNode,
		*timeoutedCommand,
		*cellIdNode,
		*lacNode,
		*restartReasonNode;

	ThingifyGsm& _thing;
public:
	GsmDiagnosticsModule(ThingifyGsm& thing);

	bool ShowFreeHeap;
	bool ShowIp;
	bool ShowNetworkName;
	bool ShowUptime;
	bool ShowCrashNode;
	bool ShowConnectionDuration;
	bool ShowReconnectCount;
#if ESP8266
	bool ShowFreeStack;
	bool ShowStackFragmentation;
#endif

	int UpdateIntervalInMs = 1000;

	const char* GetName() override;
	bool Init() override;
	bool Tick() override;
};

