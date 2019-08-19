#pragma once
#include "../ESPAsyncTCP/IAsyncClient.h"
#include <GsmModule.h>
#include <FixedString.h>

class AsyncGsmClient: public IAsyncClient 
{
	GsmModule& _gsm;
	GsmAsyncSocket* _socket;
	void* _onConnectHandlerArg;
	AcConnectHandler _onConnectHandler;
	void* _onPollArg;
	AcConnectHandler _onPoll;
	void* _onDisconnectHandlerArg;
	AcConnectHandler _onDisconnectHandler;
	void* _onDataHandlerArg;
	AcDataHandler _onDataHandler;
	void OnGsmSocketEvent(SocketEventType eventType);
	void OnGsmDataReceived(FixedStringBase& data);
	void OnPoll();
public:
	AsyncGsmClient(GsmModule& gsm, uint8_t mux);
	GsmAsyncSocket& GsmSocket();
	size_t space();
	size_t add(const char* data, size_t size, uint8_t apiflags = 0);
	bool send();
	bool connect(IPAddress ip, uint16_t port);
	size_t write(const char* data);
	size_t write(const char* data, size_t size, uint8_t apiflags = 0);
	uint8_t state();
	bool connected();
	void close(bool now = false);
	bool canSend();
	bool disconnecting();
	bool disconnected();
	int8_t abort();
	bool free();
	void setRxTimeout(uint32_t timeout);
	void ackLater();
	size_t ack(size_t len);

	bool connect(const char* host, uint16_t port);
	void onConnect(AcConnectHandler cb, void* arg = 0);   //on successful connect
	void onDisconnect(AcConnectHandler cb, void* arg = 0);   //disconnected
	void onAck(AcAckHandler cb, void* arg = 0);             //ack received
	void onError(AcErrorHandler cb, void* arg = 0);        //unsuccessful connect or error
	void onData(AcDataHandler cb, void* arg = 0);           //data received (called if onPacket is not used)
	void onPacket(AcPacketHandler cb, void* arg = 0);        //data received
	void onTimeout(AcTimeoutHandler cb, void* arg = 0);      //ack timeout
	void onPoll(AcConnectHandler cb, void* arg = 0);        //every 125ms when connected
	
};

