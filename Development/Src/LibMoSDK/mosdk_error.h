#ifndef __MOSDK_ERROR_INCLUDE__
#define __MOSDK_ERROR_INCLUDE__

#define MOERROR_NOERROR						0
#define MOERROR_UNKNOWN						1
#define MOERROR_INVALID_PARAMETER			2
#define MOERROR_AUTH_FAILED					3
#define MOERROR_SYSTEM_ERROR				4
#define MOERROR_LOCKED						5
#define MOERROR_INVALID_SESSION				6
#define MOERROR_ALREADY_EXISTED				7
#define MOERROR_NOT_EXISTED					8
#define MOERROR_UNDEFINE_METHOD				9
#define MOERROR_TRY_AGAIN					10
#define MOERROR_INVALID_DATA				11
#define MOERROR_NETWORK						12
#define MOERROR_PENDING						13

int MOErrorCode(const char* str);
const char * MOErrorString(int code);

#endif // __MOSDK_ERROR_INCLUDE__
