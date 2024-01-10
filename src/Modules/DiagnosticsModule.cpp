#include "DiagnosticsModule.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

DiagnosticsModule::DiagnosticsModule(Thingify& dev): 
_dev(dev)
{
	ShowFreeHeap = true;
	ShowIp = true;
	ShowGatewayIp = true;
	ShowSsid = true;
	ShowBssid = true;
	ShowSignalStrength = true;
	ShowUptime = true;
	ShowCrashNode = false;
	ShowConnectionDuration = true;
	ShowReconnectCount = true;

#if ESP8266
	ShowFreeStack = true;
	ShowStackFragmentation = true;
#endif
	UpdateIntervalInMs = 1000;

	ipNode = nullptr;
	gatewayIpNode = nullptr;
	ssidNode = nullptr;
	bssidNode =  nullptr;
	freeHeapNode = nullptr;
	uptimeNode = nullptr;
	crashNode = nullptr;
	serverAddressNode = nullptr;
}

const char* DiagnosticsModule::GetName()
{
	return "Diagnostic nodes";
}

bool DiagnosticsModule::Init()
{
	if (ShowIp)
	{
		ipNode = _dev.AddString("internal_ip")->SetKind(NodeKind::Statistics);
	}
	if (ShowGatewayIp)
	{
		gatewayIpNode = _dev.AddString("gateway_ip")->SetKind(NodeKind::Statistics);
	}
	if (ShowSsid)
	{
		ssidNode = _dev.AddString("network_name")->SetKind(NodeKind::Statistics);
	}
	if (ShowBssid)
	{
		bssidNode = _dev.AddString("bssid")->SetKind(NodeKind::Statistics);
	}
	if (ShowSignalStrength)
	{
		signalStrengthNode = _dev.AddRange("signal_strength", 0, 100, 1, ThingifyUnit::Percent)->SetKind(NodeKind::Statistics);
	}
	if (ShowFreeHeap)
	{
		freeHeapNode = _dev.AddInt("free_heap", ThingifyUnit::Byte)->SetKind(NodeKind::Statistics);
	}
	if (ShowUptime)
	{
		uptimeNode = _dev.AddTimeSpan("uptime")->SetKind(NodeKind::Statistics);;
	}
	if (ShowCrashNode)
	{
		crashNode = _dev.AddBoolean("crash")->OnChanged(this, [](void *ctx, Node *node) 
		{
			Node *n = nullptr;
			n->SetValue(NodeValue::Boolean(false));
			return false;
		})->SetKind(NodeKind::Statistics);
	}
	if (ShowReconnectCount)
	{
		reconnectCountNode = _dev.AddInt("reconnect_count")->SetKind(NodeKind::Statistics);
	}
	if (ShowConnectionDuration)
	{
		connectionDurationNode = _dev.AddTimeSpan("online_time")->SetKind(NodeKind::Statistics);
	}
	restartReasonNode = _dev.AddString("restart_reason")->SetKind(NodeKind::Statistics);
	restartReasonNode->SetValue(NodeValue::String(_dev.GetRestartReason()));
    resetSettingsCountNode = _dev.AddInt("settings_reset_cnt")->SetKind(NodeKind::Statistics);
    resetSettingsCountNode->SetValue(NodeValue::Int((int)_dev.GetResetSettingsCount()));
	serverAddressNode = _dev.AddString("server")->SetKind(NodeKind::Statistics);

#if ESP8266
	if (ShowFreeStack)
	{
		_dev.AddInt("free_stack", ThingifyUnit::Byte)->SetKind(NodeKind::Statistics);
	}
	if (ShowStackFragmentation)
	{
		_dev.AddInt("heap_frag", ThingifyUnit::Percent)->SetKind(NodeKind::Statistics);
	}
#endif // ESP8266	
	return true;
}

bool DiagnosticsModule::Tick()
{
	static SoftTimer st(UpdateIntervalInMs);
	if (!st.IsElapsed())
	{
		return true;
	}

	if (ShowIp)
	{
		ipNode->SetValue(NodeValue::String(WiFi.localIP().toString().c_str()));
	}
	if (ShowGatewayIp)
	{
		gatewayIpNode->SetValue(NodeValue::String(WiFi.gatewayIP().toString().c_str()));
	}
	if (ShowSsid)
	{
		ssidNode->SetValue(NodeValue::String(WiFi.SSID().c_str()));
	}
	if (ShowBssid)
	{
		bssidNode->SetValue(NodeValue::String(WiFi.BSSIDstr().c_str()));
	}
	if (ShowSignalStrength)
	{
		auto signalStrength = WiFi.RSSI();		
		signalStrengthNode->SetValue(NodeValue::Int(DbmToPercent(signalStrength)));
	}
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
		reconnectCountNode->SetValue(NodeValue::Int(_dev.GetReconnectCount()));
	}

	if (ShowConnectionDuration)
	{
		connectionDurationNode->SetValue(NodeValue::TimeSpan(_dev.GetMillisecondsSinceConnect()));
	}

	char serverName[255];
	memset(serverName, 0, 255);
	snprintf(serverName, 255, "%s:%d", _dev.GetServerName(), _dev.GetServerPort());
	serverAddressNode->SetValue(NodeValue::String(serverName));
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

int DiagnosticsModule::DbmToPercent(int dBm)
{
	if (dBm < -92)
	{
		return 1;
	}
	else if (dBm > -21)
	{
		return 100;
	}
	else
	{
		return round((-0.0154*dBm*dBm) - (0.3794*dBm) + 98.182);
	}
}
