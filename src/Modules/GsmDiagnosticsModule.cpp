#ifdef THINGIFY_GSM
#include "GsmDiagnosticsModule.h"


GsmDiagnosticsModule::GsmDiagnosticsModule(ThingifyGsm& thing) :
	_thing(thing)
{
	ShowFreeHeap = true;
	ShowIp = true;
	ShowNetworkName = true;
	ShowUptime = true;
	ShowCrashNode = false;
	ShowConnectionDuration = true;
	ShowReconnectCount = true;

#if ESP8266
	ShowFreeStack = true;
	ShowStackFragmentation = true;
#endif

	ipNode = nullptr;
	gatewayIpNode = nullptr;
	networkNameNode = nullptr;
	bssidNode = nullptr;
	freeHeapNode = nullptr;
	uptimeNode = nullptr;
	crashNode = nullptr;
}

const char* GsmDiagnosticsModule::GetName()
{
	return "Debug nodes";
}

bool GsmDiagnosticsModule::Init()
{
	if (ShowIp)
	{
		ipNode = _thing.AddString("internal_ip")->SetKind(NodeKind::Statistics);
	}
	
	if (ShowNetworkName)
	{
		networkNameNode = _thing.AddString("network_name")->SetKind(NodeKind::Statistics);
	}
	moduleReconnectCountNode = _thing.AddInt("gsm.connect_count")->SetKind(NodeKind::Statistics);
	timeoutedCommand = _thing.AddString("gsm.to_command")->SetKind(NodeKind::Statistics);
	signalQualityNode = _thing.AddRange("signal_strength", 0, 32, 1, ThingifyUnit::Rssi)->SetKind(NodeKind::Statistics);

	batteryNode = _thing.AddRange("battery_level", 0, 100, 1, ThingifyUnit::Percent)->SetKind(NodeKind::Statistics);
	batteryVoltageNode = _thing.AddFloat("battery_voltage", ThingifyUnit::Volt)->SetKind(NodeKind::Statistics);
	if (ShowFreeHeap)
	{
		freeHeapNode = _thing.AddInt("free_heap", ThingifyUnit::Byte)->SetKind(NodeKind::Statistics);
	}
	if (ShowUptime)
	{
		uptimeNode = _thing.AddTimeSpan("uptime")->SetKind(NodeKind::Statistics);;
	}
	if (ShowCrashNode)
	{
		crashNode = _thing.AddBoolean("crash")->SetKind(NodeKind::Statistics)->OnChanged(this, [](void *ctx, Node *node)
		{
			Node *n = nullptr;
			n->SetValue(NodeValue::Boolean(false));
			return false;
		});
	}
	if (ShowReconnectCount)
	{
		reconnectCountNode = _thing.AddInt("reconnect_count")->SetKind(NodeKind::Statistics);;
	}
	if (ShowConnectionDuration)
	{
		connectionDurationNode = _thing.AddTimeSpan("online_time")->SetKind(NodeKind::Statistics);;
	}
	lacNode = _thing.AddInt("gsm.lac")->SetKind(NodeKind::Statistics);
	cellIdNode = _thing.AddInt("gsm.cell_id")->SetKind(NodeKind::Statistics);
	restartReasonNode = _thing.AddString("restart_reason")->SetKind(NodeKind::Statistics);
	restartReasonNode->SetValue(NodeValue::String(_thing.GetRestartReason()));
#if ESP8266
	if (ShowFreeStack)
	{
		_dev.AddInt("free_stack", ThingifyUnit::Byte);
	}
	if (ShowStackFragmentation)
	{
		_dev.AddInt("heap_frag", ThingifyUnit::Percent);
	}
#endif // ESP8266	
	return true;
}

bool GsmDiagnosticsModule::Tick()
{
	static SoftTimer st(UpdateIntervalInMs);
	st.UpdateDelay(UpdateIntervalInMs);
	if (!st.IsElapsed())
	{
		return true;
	}
	
	auto gsm = _thing.Gsm();

	if (ShowIp)
	{
		ipNode->SetValue(NodeValue::String(gsm.ipAddress.ToString().c_str()));
	}
	
	if (ShowNetworkName)
	{
		networkNameNode->SetValue(NodeValue::String(gsm.operatorName.c_str()));
	}	

	signalQualityNode->SetValue(NodeValue::Int(gsm.signalQuality));
	batteryNode->SetValue(NodeValue::Int(gsm.batteryInfo.Percent));
	batteryVoltageNode->SetValue(NodeValue::Float(gsm.batteryInfo.Voltage));

	moduleReconnectCountNode->SetValue(NodeValue::Int(gsm.ModuleConnectCount));
	timeoutedCommand->SetValue(NodeValue::String(gsm.At().TimeoutedCommand.c_str()));
	if (ShowFreeHeap)
	{
		freeHeapNode->SetValue(NodeValue::Int(static_cast<int>(ESP.getFreeHeap())));
	}
	if (ShowUptime)
	{
		uptimeNode->SetValue(NodeValue::TimeSpan(millis()));
	}

	if (ShowReconnectCount)
	{
		reconnectCountNode->SetValue(NodeValue::Int(_thing.GetReconnectCount()));
	}

	if (ShowConnectionDuration)
	{
		connectionDurationNode->SetValue(NodeValue::TimeSpan(_thing.GetMillisecondsSinceConnect()));
	}
	lacNode->SetValue(NodeValue::Int(_thing.Gsm().Lac));
	cellIdNode->SetValue(NodeValue::Int(_thing.Gsm().CellId));
#if ESP8266
	if (ShowFreeStack)
	{
		_dev["free_stack"]->SetValue(NodeValue::Int(ESP.getFreeContStack()));
	}

	if (ShowStackFragmentation)
	{
		_dev["heap_frag"]->SetValue(NodeValue::Int(ESP.getHeapFragmentation()));
	}
#endif
	return true;
}

#endif
