#ifndef __FAST_DDL_JSON__
#define __FAST_DDL_JSON__

namespace DDL
{

	bool FastJson2Struct(const STRUCT_INFO* def, const Zion::JsonValue& Value, void* data);
	bool FastJson2Struct(const STRUCT_INFO* def, const Zion::String& json, void* data);

	bool Struct2FastJson(const STRUCT_INFO* def, const void* data, Zion::JsonValue& Value);
	bool Struct2FastJson(const STRUCT_INFO* def, const void* data, Zion::String& json);

}

#endif
