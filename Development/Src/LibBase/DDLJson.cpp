#include "ZionBase.h"
#include "FastJson.h"

namespace DDLReflect
{

	class JsonDDLInterop : public Zion::JsonInterop
	{
	public:
		JsonDDLInterop(FIELD_INFO* finfo, void* data)
		{
			m_Stack.push(*finfo);
			m_Stack.top().offset = 0;
			m_Data = (_U8*)data;
		}

		virtual ~JsonDDLInterop()
		{
		}

		virtual bool StartObject()
		{
			if (m_Stack.top().type != TYPE_STRUCT)
			{
				ZION_ASSERT(!"invalid json value type");
				return false;
			}
			return true;
		}

		virtual bool NewElement(const char* _name, _U32 name_len)
		{
			FIELD_INFO* struct_finfo = &m_Stack.top();
			ZION_ASSERT(struct_finfo->type == TYPE_STRUCT);
			FIELD_INFO finfo;
			Zion::String name;
			name.append(_name, (size_t)name_len);
			_U8* data = (_U8*)GetStructFieldData(struct_finfo->sinfo, name.c_str(), m_Data + struct_finfo->offset, finfo);
			if (!data)
			{
				ZION_ASSERT(!"not found");
				return false;
			}
			finfo.offset = (_U16)(data - m_Data);
			m_Stack.push(finfo);
			return true;
		}

		virtual bool EndObject()
		{
			ZION_ASSERT(m_Stack.top().type == TYPE_STRUCT);
			m_Stack.pop();
			return true;
		}

		virtual bool StartArray()
		{
			if ((m_Stack.top().type&TYPE_ARRAY) == 0)
			{
				ZION_ASSERT(!"not found");
				return false;
			}
			return true;
		}

		virtual bool NewElement()
		{
			FIELD_INFO* array_finfo = &m_Stack.top();
			ZION_ASSERT((array_finfo->type&TYPE_ARRAY) != 0);
			_U32 count = 0;
			if (count == array_finfo->alen)
			{
				ZION_ASSERT(!"AAAA");
				return false;
			}
			FIELD_INFO finfo;
			memcpy(&finfo, array_finfo, sizeof(FIELD_INFO));
			finfo.type &= TYPE_MASK;
			finfo.offset += (_U16)(finfo.elen * count + array_finfo->prefix);
			m_Stack.push(finfo);
			return true;
		}

		virtual bool EndArray()
		{
			ZION_ASSERT((m_Stack.top().type&TYPE_ARRAY) != 0);
			m_Stack.pop();
			return true;
		}

		virtual bool NewValueS64(_S64 value)
		{
			FIELD_INFO* finfo = &m_Stack.top();
			switch (finfo->type)
			{
			case TYPE_S8:
				*((_S8*)(m_Data + finfo->offset)) = (_S8)value;
				break;
			case TYPE_S16:
				*((_S16*)(m_Data + finfo->offset)) = (_S16)value;
				break;
			case TYPE_S32:
				*((_S32*)(m_Data + finfo->offset)) = (_S32)value;
				break;
			case TYPE_S64:
				*((_S64*)(m_Data + finfo->offset)) = (_S64)value;
				break;
			case TYPE_F32:
				*((_F32*)(m_Data + finfo->offset)) = (_F32)value;
				break;
			case TYPE_F64:
				*((_F64*)(m_Data + finfo->offset)) = (_F64)value;
				break;
			default:
				ZION_ASSERT(!"XXXX");
				return false;
			}
			m_Stack.pop();
			return true;
		}

		virtual bool NewValueU64(_U64 value)
		{
			FIELD_INFO* finfo = &m_Stack.top();
			switch (finfo->type)
			{
			case TYPE_U8:
				*((_U8*)(m_Data + finfo->offset)) = (_U8)value;
				break;
			case TYPE_U16:
				*((_U16*)(m_Data + finfo->offset)) = (_U16)value;
				break;
			case TYPE_U32:
				*((_U32*)(m_Data + finfo->offset)) = (_U32)value;
				break;
			case TYPE_U64:
				*((_U64*)(m_Data + finfo->offset)) = (_U64)value;
				break;
			case TYPE_S8:
				*((_S8*)(m_Data + finfo->offset)) = (_S8)value;
				break;
			case TYPE_S16:
				*((_S16*)(m_Data + finfo->offset)) = (_S16)value;
				break;
			case TYPE_S32:
				*((_S32*)(m_Data + finfo->offset)) = (_S32)value;
				break;
			case TYPE_S64:
				*((_S64*)(m_Data + finfo->offset)) = (_S64)value;
				break;
			case TYPE_F32:
				*((_F32*)(m_Data + finfo->offset)) = (_F32)value;
				break;
			case TYPE_F64:
				*((_F64*)(m_Data + finfo->offset)) = (_F64)value;
				break;
			default:
				ZION_ASSERT(!"XXXX");
				return false;
			}

			m_Stack.pop();
			return true;
		}

		virtual bool NewValueF64(_F64 value)
		{
			FIELD_INFO* finfo = &m_Stack.top();
			switch (finfo->type)
			{
			case TYPE_F32:
				*((_F32*)(m_Data + finfo->offset)) = (_F32)value;
				break;
			case TYPE_F64:
				*((_F64*)(m_Data + finfo->offset)) = (_F64)value;
				break;
			default:
				ZION_ASSERT(!"XXXX");
				return false;
			}

			m_Stack.pop();
			return true;
		}

		virtual bool NewValueStr(const char* _value, _U32 len)
		{
			Zion::String value;
			value.append(_value, len);

			FIELD_INFO* finfo = &m_Stack.top();
			switch (finfo->type)
			{
			case TYPE_STRING:
				if (finfo->slen < (_U32)value.size())
				{
					ZION_ASSERT(!"");
					return false;
				}
				memcpy(m_Data + finfo->offset, value.c_str(), value.size() + 1);
				break;
			case TYPE_UUID:
			case TYPE_UUID_REF:
				if (!AUuidFromString(value.c_str(), *((A_UUID*)(m_Data + finfo->offset))))
				{
					ZION_ASSERT(!"");
					return false;
				}
				return true;
			default:
				ZION_ASSERT(!"XXXX");
				return false;
			}

			m_Stack.pop();
			return true;
		}

		virtual bool NewValueBool(bool value)
		{
			FIELD_INFO* finfo = &m_Stack.top();
			switch (finfo->type)
			{
			case TYPE_U8:
				*((_U8*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_U16:
				*((_U16*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_U32:
				*((_U32*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_U64:
				*((_U64*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_S8:
				*((_S8*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_S16:
				*((_S16*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_S32:
				*((_S32*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_S64:
				*((_S64*)(m_Data + finfo->offset)) = value ? 1 : 0;
				break;
			case TYPE_F32:
				*((_F32*)(m_Data + finfo->offset)) = value ? (_F32)1 : (_F32)0;
				break;
			case TYPE_F64:
				*((_F64*)(m_Data + finfo->offset)) = value ? (_F64)1 : (_F64)0;
				break;
			case TYPE_STRING:
				if (value)
				{
					if (finfo->slen < 4)
					{
						ZION_ASSERT(!"");
						return false;
					}
					memcpy(m_Data + finfo->offset, "true", 5);
				}
				else
				{
					if (finfo->slen < 5)
					{
						ZION_ASSERT(!"");
						return false;
					}
					memcpy(m_Data + finfo->offset, "false", false);
				}
				break;
			default:
				ZION_ASSERT(!"XXXX");
				return false;
			}

			m_Stack.pop();
			return true;
		}

		virtual bool NewValueNull()
		{
			ZION_ASSERT(0);
			return false;
		}

	private:
		Zion::Stack<FIELD_INFO> m_Stack;
		_U8* m_Data;
	};

	static bool ProcessElement(Zion::JsonInterop* interop, const Zion::JsonValue& elm)
	{
		switch (elm.GetType())
		{
		case Zion::JsonValue::TYPE_NULL:	if (!interop->NewValueNull()) return false; break;
		case Zion::JsonValue::TYPE_BOOL:	if (!interop->NewValueBool(elm.AsBool())) return false; break;
		case Zion::JsonValue::TYPE_U64:		if (!interop->NewValueU64(elm.AsU64())) return false; break;
		case Zion::JsonValue::TYPE_S64:		if (!interop->NewValueS64(elm.AsS64())) return false; break;
		case Zion::JsonValue::TYPE_F64:		if (!interop->NewValueF64(elm.AsF64())) return false; break;
		case Zion::JsonValue::TYPE_STR:		if (!interop->NewValueStr(elm.AsSTR(), elm.GetStringLength())) return false; break;
		case Zion::JsonValue::TYPE_OBJECT:
			if (!interop->StartObject()) return false;
			for (_U32 i = 0; i < elm.GetMemberCount(); i++) {
				const Zion::JsonValue* name = elm.GetMemberNames() + i;
				const Zion::JsonValue* value = elm.GetMemberValues() + i;
				if (!interop->NewElement(name->AsSTR(), name->GetStringLength())) return false;
				if (!ProcessElement(interop, *value)) return false;
			}
			if (!interop->EndObject()) return false;
			break;
		case Zion::JsonValue::TYPE_ARRAY:
			if (!interop->StartArray()) return false;
			for (_U32 i = 0; i < elm.GetMemberCount(); i++) {
				const Zion::JsonValue* value = elm.GetArray() + i;
				if (!interop->NewElement()) return false;
				if (!ProcessElement(interop, *value)) return false;
			}
			if (!interop->EndArray()) return false;
			break;
		}
		return true;
	}

	static bool ProcessElement(const STRUCT_INFO* def, const void* data, Zion::JsonInterop* interop)
	{
		(void)def;
		(void)data;
		(void)interop;
		return false;
	}

	bool FastJson2Struct(const STRUCT_INFO* def, const Zion::JsonValue& Value, void* data)
	{
		FIELD_INFO finfo;
		finfo.type = TYPE_STRUCT;
		finfo.sinfo = def;
		JsonDDLInterop interop(&finfo, data);
		return ProcessElement(&interop, Value);
	}

	bool FastJson2Struct(const STRUCT_INFO* def, const Zion::String& json, void* data)
	{
		FIELD_INFO finfo;
		finfo.type = TYPE_STRUCT;
		finfo.sinfo = def;
		JsonDDLInterop interop(&finfo, data);
		Zion::JsonReader reader;
		return reader.Parse(json.c_str(), json.c_str() + json.size(), interop);
	}

	bool Struct2FastJson(const STRUCT_INFO* def, const void* data, Zion::JsonValue& Value)
	{
		Zion::JsonNodeBuilder op(Value);
		return ProcessElement(def, data, &op);
	}

	bool Struct2FastJson(const STRUCT_INFO* def, const void* data, Zion::String& json)
	{
		Zion::OutputStringStream sstream;
		Zion::JsonStringBuilder op(&sstream);
		if (!ProcessElement(def, data, &op)) return false;
		json = sstream.str();
		return true;
	}

}
