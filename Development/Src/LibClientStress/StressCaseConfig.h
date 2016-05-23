////////////////////////////////////////////////
// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //
////////////////////////////////////////////////

#ifndef __STRESSCASECONFIG_DDL_CODEGEN__
#define __STRESSCASECONFIG_DDL_CODEGEN__

#pragma pack(push)
#pragma pack(4)

struct STRESSCASE_LOGIN_CONFIG
{
	_S32 reset_time;
	_U32 retry_time;
};

namespace DDL
{
	template<>
	bool BufferReader::Read<STRESSCASE_LOGIN_CONFIG>(STRESSCASE_LOGIN_CONFIG& Value);
	template<>
	bool BufferWriter::Write<STRESSCASE_LOGIN_CONFIG>(const STRESSCASE_LOGIN_CONFIG& Value);
}

namespace DDLReflect
{
	template<>
	const STRUCT_INFO* GetStruct<STRESSCASE_LOGIN_CONFIG>();
	extern STRUCT_INFO _rfl_struct_STRESSCASE_LOGIN_CONFIG_info;
}

struct STRESSCASE_LOGIN_STATUS
{
	_U32 failed_times;
	_U32 sucess_times;
};

namespace DDL
{
	template<>
	bool BufferReader::Read<STRESSCASE_LOGIN_STATUS>(STRESSCASE_LOGIN_STATUS& Value);
	template<>
	bool BufferWriter::Write<STRESSCASE_LOGIN_STATUS>(const STRESSCASE_LOGIN_STATUS& Value);
}

namespace DDLReflect
{
	template<>
	const STRUCT_INFO* GetStruct<STRESSCASE_LOGIN_STATUS>();
	extern STRUCT_INFO _rfl_struct_STRESSCASE_LOGIN_STATUS_info;
}


#pragma pack(pop)

#endif