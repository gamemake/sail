////////////////////////////////////////////////
// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //
////////////////////////////////////////////////

#ifndef __CARDSTRESSCASECONFIG_DDL_CODEGEN__
#define __CARDSTRESSCASECONFIG_DDL_CODEGEN__

#pragma pack(push)
#pragma pack(4)

struct STRESSCASE_ENTERGAME_CONFIG
{
	_U32 server_id;
	DDL::String<100> avatar_name;
};

namespace DDL
{
	template<>
	bool BufferReader::Read<STRESSCASE_ENTERGAME_CONFIG>(STRESSCASE_ENTERGAME_CONFIG& Value);
	template<>
	bool BufferWriter::Write<STRESSCASE_ENTERGAME_CONFIG>(const STRESSCASE_ENTERGAME_CONFIG& Value);
}

namespace DDLReflect
{
	template<>
	const STRUCT_INFO* GetStruct<STRESSCASE_ENTERGAME_CONFIG>();
	extern STRUCT_INFO _rfl_struct_STRESSCASE_ENTERGAME_CONFIG_info;
}

struct STRESSCASE_ENTERGAME_STATUS
{
	_U32 failed_times;
	_U32 sucess_times;
};

namespace DDL
{
	template<>
	bool BufferReader::Read<STRESSCASE_ENTERGAME_STATUS>(STRESSCASE_ENTERGAME_STATUS& Value);
	template<>
	bool BufferWriter::Write<STRESSCASE_ENTERGAME_STATUS>(const STRESSCASE_ENTERGAME_STATUS& Value);
}

namespace DDLReflect
{
	template<>
	const STRUCT_INFO* GetStruct<STRESSCASE_ENTERGAME_STATUS>();
	extern STRUCT_INFO _rfl_struct_STRESSCASE_ENTERGAME_STATUS_info;
}


#pragma pack(pop)

#endif
