#ifndef  _I_ASYNC_CLIENT_H
#define _I_ASYNC_CLIENT_H

#include <Arduino.h>
#include <functional>
#include <IPAddress.h>

class IAsyncClient;

typedef std::function<void(void*, IAsyncClient*)> AcConnectHandler;
typedef std::function<void(void*, IAsyncClient*, size_t len, uint32_t time)> AcAckHandler;
typedef std::function<void(void*, IAsyncClient*, int8_t error)> AcErrorHandler;
typedef std::function<void(void*, IAsyncClient*, void *data, size_t len)> AcDataHandler;
typedef std::function<void(void*, IAsyncClient*, struct pbuf *pb)> AcPacketHandler;
typedef std::function<void(void*, IAsyncClient*, uint32_t time)> AcTimeoutHandler;

class IAsyncClient
{
public:
	virtual size_t space() = 0;
	virtual size_t add(const char* data, size_t size, uint8_t apiflags = 0) =0;//add for sending
	virtual bool send() = 0;//send all data added with the method above
	virtual	bool connect(IPAddress ip, uint16_t port) = 0;
	virtual size_t write(const char* data) = 0;
	virtual size_t write(const char* data, size_t size, uint8_t apiflags = 0) =0; //only when canSend() == true
	virtual uint8_t state() = 0;
	virtual bool connected() = 0;
	virtual void close(bool now = false) = 0;
	virtual bool canSend() = 0;//ack is not pending
	virtual bool disconnecting() = 0;
	virtual bool disconnected() = 0;
	virtual int8_t abort() = 0;
	virtual bool free() = 0;
	virtual void setRxTimeout(uint32_t timeout) = 0;//no RX data timeout for the connection in seconds
	virtual void ackLater() = 0;
	virtual size_t ack(size_t len) = 0; //ack data that you have not acked using the method below

	virtual bool connect(const char* host, uint16_t port) = 0;
	virtual void onConnect(AcConnectHandler cb, void* arg = 0) = 0;   //on successful connect
	virtual void onDisconnect(AcConnectHandler cb, void* arg = 0) = 0;   //disconnected
	virtual void onAck(AcAckHandler cb, void* arg = 0) =0;             //ack received
	virtual void onError(AcErrorHandler cb, void* arg = 0) = 0;        //unsuccessful connect or error
	virtual void onData(AcDataHandler cb, void* arg = 0) = 0;           //data received (called if onPacket is not used)
	virtual void onPacket(AcPacketHandler cb, void* arg = 0) = 0;        //data received
	virtual void onTimeout(AcTimeoutHandler cb, void* arg = 0) = 0;      //ack timeout
	virtual void onPoll(AcConnectHandler cb, void* arg = 0) = 0;        //every 125ms when connected
};

#endif //
