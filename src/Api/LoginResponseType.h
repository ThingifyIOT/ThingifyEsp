#ifndef _LOGIN_RESPONSE_TYPE_H
#define _LOGIN_RESPONSE_TYPE_H

enum class LoginResponseType
{
	Success,
	InvalidCredentials,
	Error,
	ProtocolViolation
};

#endif