#ifdef GSM

#include "AsyncGsmClient.h"


AsyncGsmClient::AsyncGsmClient(GsmModule& gsm, uint8_t mux):
	_gsm(gsm),
	_onConnectHandlerArg(nullptr),
	_onDisconnectHandlerArg(nullptr),
	_onDataHandlerArg(nullptr)
{
	_socket = _gsm.CreateSocket(mux, ProtocolType::Tcp);
	_socket->OnSocketEvent(this, [](void* ctx, SocketEventType eventType)
	{
		reinterpret_cast<AsyncGsmClient*>(ctx)->OnGsmSocketEvent(eventType);
	});
	_socket->OnDataRecieved(this, [](void* ctx, FixedStringBase& data)
	{
		reinterpret_cast<AsyncGsmClient*>(ctx)->OnGsmDataReceived(data);
	});
	_socket->OnPoll(this, [](void* ctx) {
		reinterpret_cast<AsyncGsmClient*>(ctx)->OnPoll();
	});
}
GsmAsyncSocket& AsyncGsmClient::GsmSocket()
{
	return *_socket;
}
void AsyncGsmClient::OnGsmSocketEvent(SocketEventType eventType)
{
	Serial.println("OnGsmSocketEvent");


	if (eventType == SocketEventType::ConnectSuccess)
	{
		if (_onConnectHandler)
		{
			_onConnectHandler(_onConnectHandlerArg, this);
		}
	}
	if (eventType == SocketEventType::Disconnected || eventType == SocketEventType::ConnectFailed)
	{
		if(_onDisconnectHandler)
		{
			_onDisconnectHandler(_onDisconnectHandlerArg, this);
		}		
	}
}

void AsyncGsmClient::OnGsmDataReceived(FixedStringBase& data)
{
	if (_onDataHandler)
	{
		_onDataHandler(_onDataHandlerArg, this, (void*)data.c_str(), data.length());
	}
}

void AsyncGsmClient::OnPoll()
{
	if (_onPoll)
	{
		_onPoll(_onPollArg, this);
	}
}


size_t AsyncGsmClient::space()
{
	return _socket->space();
}

size_t AsyncGsmClient::add(const char * data, size_t size, uint8_t apiflags)
{
	return _socket->Send(data, size);
}

bool AsyncGsmClient::send()
{
	return true;
}

bool AsyncGsmClient::connect(IPAddress ip, uint16_t port)
{
	return false;
}

size_t AsyncGsmClient::write(const char * data)
{
	return _socket->Send(data);
}

size_t AsyncGsmClient::write(const char * data, size_t size, uint8_t apiflags)
{
	return _socket->Send(data, size);
}

uint8_t AsyncGsmClient::state()
{
	return 1;
}

bool AsyncGsmClient::connected()
{
	return _socket->IsConnected();
}

void AsyncGsmClient::close(bool now)
{
	_socket->Close();
}

bool AsyncGsmClient::canSend()
{
	return _socket->space() > 0;
}

bool AsyncGsmClient::disconnecting()
{
	return _socket->GetState() == SocketStateType::Closing;
}

bool AsyncGsmClient::disconnected()
{
	return _socket->GetState() == SocketStateType::Closed;
}

int8_t AsyncGsmClient::abort()
{
	return 0;
}

bool AsyncGsmClient::free()
{
	return true;
}

void AsyncGsmClient::setRxTimeout(uint32_t timeout)
{
}

void AsyncGsmClient::ackLater()
{
}

size_t AsyncGsmClient::ack(size_t len)
{
	return size_t();
}

bool AsyncGsmClient::connect(const char * host, uint16_t port)
{
	return _socket->BeginConnect(host, port);
}

void AsyncGsmClient::onConnect(AcConnectHandler cb, void* arg)
{
	_onConnectHandlerArg = arg;
	_onConnectHandler = cb;
}

void AsyncGsmClient::onDisconnect(AcConnectHandler cb, void* arg)
{
	_onDisconnectHandlerArg = arg;
	_onDisconnectHandler = cb;
}

void AsyncGsmClient::onAck(AcAckHandler cb, void* arg)
{
}

void AsyncGsmClient::onError(AcErrorHandler cb, void* arg)
{
}

void AsyncGsmClient::onData(AcDataHandler cb, void* arg)
{
	_onDataHandlerArg = arg;
	_onDataHandler = cb;
}

void AsyncGsmClient::onPacket(AcPacketHandler cb, void* arg)
{
}

void AsyncGsmClient::onTimeout(AcTimeoutHandler cb, void* arg)
{
}

void AsyncGsmClient::onPoll(AcConnectHandler cb, void* arg)
{
	_onPoll = cb;
	_onPollArg = arg;
}

#endif

