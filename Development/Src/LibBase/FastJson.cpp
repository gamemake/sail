#include "ZionDefines.h"
#include "ZionSTL.h"
#include "FastJson.h"
#include <math.h>

#if 0
const char* ErrorReturn(int line)
{
	return NULL;
}
#define ERROR_RETURN return ErrorReturn(__LINE__)
#else
#define ERROR_RETURN return NULL
#endif

namespace Zion
{

	bool JsonReader::Parse(const char* begin, const char* end, JsonInterop& interop)
	{
		m_interop = &interop;
		m_begin = begin;
		m_end = end;
		const char* cur = ParseElement(m_begin);
		if (!cur) return false;
		cur = Skip(cur);
		return cur == m_end;
	}

	const char* JsonReader::Skip(const char* cur)
	{
		if (cur)
		{
			for (; cur < m_end; cur++)
			{
				switch (*cur)
				{
				case '\r':	break;
				case '\n':	break;
				case ' ':	break;
				case '\t':	break;
				default: return cur;
				}
			}
		}
		return cur;
	}

	const char* JsonReader::ParseElement(const char* start)
	{
		const char* cur = Skip(start);
		if (cur == NULL) ERROR_RETURN;
		switch (*cur)
		{
		case '"': return ParseString(cur);
		case '[': return ParseArray(cur);
		case '{': return ParseObject(cur);
		case 't':
			if (m_end - cur < 4) ERROR_RETURN;
			if (memcmp(cur, "true", 4) != 0) ERROR_RETURN;
			if (!m_interop->NewValueBool(true)) ERROR_RETURN;
			return cur + 4;
		case 'f':
			if (m_end - cur < 5) ERROR_RETURN;
			if (memcmp(cur, "false", 5) != 0) ERROR_RETURN;
			if (!m_interop->NewValueBool(false)) ERROR_RETURN;
			return cur + 5;
		case 'n':
			if (m_end - cur < 4) ERROR_RETURN;
			if (memcmp(cur, "null", 4) != 0) ERROR_RETURN;
			if (!m_interop->NewValueNull()) ERROR_RETURN;
			return cur + 4;
		case '+':
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return ParseNumber(cur);
		}
		ERROR_RETURN;
	}

	const char* GetNumber(const char* cur, const char* end, int& value)
	{
		_U8 c = *((const _U8*)cur);
		if (c<'0' || c>'9') return NULL;
		for (;;)
		{
			value = value * 10 + (c - '0');
			if ((++cur) == end) break;
			c = *((const _U8*)cur);
			if (c<'0' || c>'9') break;
		}
		return cur;
	}

	const char* GetNumber(const char* cur, const char* end, _U64& value, int& t)
	{
		_U8 c = *((const _U8*)cur);
		if (c<'0' || c>'9') return NULL;
		for (;;)
		{
			value = value * 10 + (c - '0');
			if ((++cur) == end) break;
			c = *((const _U8*)cur);
			if(c=='.')
			{
				if((++cur) == end) return NULL;
				t = 0;
				c = *((const _U8*)cur);
				if (c<'0' || c>'9') return cur;
				for(;;)
				{
					value = value * 10 + (c - '0');
					t -= 1;
					if ((++cur) == end) break;
					c = *((const _U8*)cur);
					if (c<'0' || c>'9') break;
				}
				return cur;
			}
			if (c<'0' || c>'9') break;
		}
		t = 0;
		return cur;
	}

	const char* JsonReader::ParseNumber(const char* start)
	{
		const char* cur = start;

		bool is_minus = false;

		if (*cur == '-')
		{
			is_minus = true;
			if ((++cur) == m_end) ERROR_RETURN;
		}
		else if (*cur == '+')
		{
			if ((++cur) == m_end) ERROR_RETURN;
		}

		_U64 val1 = 0;
		int t1;
		cur = GetNumber(cur, m_end, val1, t1);
		if (cur == NULL) ERROR_RETURN;
		if (cur == m_end || (*cur != 'e' && *cur != 'E'))
		{
			if(t1<0)
			{
				if(is_minus)
				{
					if (!m_interop->NewValueF64(val1 * pow((double)10, t1) * -1)) ERROR_RETURN;
				}
				else
				{
					if (!m_interop->NewValueF64(val1 * pow((double)10, t1))) ERROR_RETURN;
				}
			}
			else
			{
				if(is_minus)
				{
					if (!m_interop->NewValueS64((_S64)val1*-1)) ERROR_RETURN;
				}
				else
				{
					if (!m_interop->NewValueU64(val1)) ERROR_RETURN;
				}
			}
			return cur;
		}
		if ((++cur) == m_end) ERROR_RETURN;

		bool is_ext_minus = false;
		if (*cur == '-')
		{
			is_ext_minus = true;
			if ((++cur) == m_end) ERROR_RETURN;
		}
		else if (*cur == '+')
		{
			if ((++cur) == m_end) ERROR_RETURN;
		}

		int t2 = 0;
		cur = GetNumber(cur, m_end, t2);
		if (cur == NULL) ERROR_RETURN;

		int t;
		if(is_ext_minus)
			t = t1 + t2;
		else
			t = t1 - t2;

		if(is_minus)
		{
			if (!m_interop->NewValueF64(val1 * pow((double)10, t) * -1)) ERROR_RETURN;
		}
		else
		{

			if (!m_interop->NewValueF64(val1 * pow((double)10, t))) ERROR_RETURN;
		}

		return cur;
	}

	const char* JsonReader::ParseString(const char* start)
	{
		const char* value;
		_U32 length;
		start = ParseStringValue(start, value, length);
		if (!start) ERROR_RETURN;
		if (!m_interop->NewValueStr(value, length)) ERROR_RETURN;
		return start;
	}

	const char* JsonReader::ParseArray(const char* start)
	{
		const char* cur = start + 1;
		if (cur == m_end) ERROR_RETURN;

		m_interop->StartArray();

		bool first = true;
		for (;;)
		{
			cur = Skip(cur);
			if (*cur == ']') break;

			if (first)
			{
				first = false;
			}
			else
			{
				if (*cur != ',') ERROR_RETURN;
				cur += 1;
			}

			if (!m_interop->NewElement()) ERROR_RETURN;
			cur = ParseElement(cur);
			if (!cur) ERROR_RETURN;
		}

		m_interop->EndArray();
		return cur + 1;
	}

	const char* JsonReader::ParseObject(const char* start)
	{
		const char* cur = start + 1;
		if (cur == m_end) ERROR_RETURN;

		m_interop->StartObject();

		bool first = true;
		for (;;)
		{
			cur = Skip(cur);
			if (cur == m_end) ERROR_RETURN;

			if (*cur == '}') break;
			if (first)
			{
				first = false;
			}
			else
			{
				if (*cur != ',') ERROR_RETURN;
				cur += 1;
			}

			const char* name;
			_U32 name_len;
			cur = ParseStringValue(cur, name, name_len);
			if (!cur) ERROR_RETURN;
			m_interop->NewElement(name, name_len);

			cur = Skip(cur);
			if (!cur || *cur != ':') ERROR_RETURN;

			cur = ParseElement(cur + 1);
			if (!cur) ERROR_RETURN;
		}

		m_interop->EndObject();

		return cur + 1;
	}

	const char* JsonReader::ParseStringValue(const char* start, const char*& value, _U32& length)
	{
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		static const char escape[256] = {
			Z16, Z16, 0, 0, '\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/',
			Z16, Z16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,
			0, 0, '\b', 0, 0, 0, '\f', 0, 0, 0, 0, 0, 0, 0, '\n', 0,
			0, 0, '\r', 0, '\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16
		};
#undef Z16

#define Z16 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
		static const _U16 hex[256] = {
			Z16,
			Z16,
			Z16,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
			0xffff, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
			Z16,
			0xffff, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
			Z16,
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16
		};
#undef Z16

		start = Skip(start);
		if (!start || *start != '"') ERROR_RETURN;
		const char* cur = (++start);
		if (cur == m_end) ERROR_RETURN;

		char* buffer = m_buffer;
		const char* dcopy = NULL;

		while (*cur != '"')
		{
			if (*cur == '\\')
			{
				if (dcopy)
				{
					size_t len = cur - dcopy;
					memcpy(buffer, dcopy, len);
					dcopy = NULL;
					buffer += len;
				}

				char c = escape[*((_U8*)(++cur))];
				if (c)
				{
					*(buffer++) = c;
					if ((++cur) == m_end) ERROR_RETURN;
				}
				else if (*cur == '"') break;
				else if (*cur == 'u')
				{
					if (m_end - cur < 6) ERROR_RETURN;
					_U16 cp = 0;
					for (_U32 i = 0; i < 4; i++)
					{
						_U16 v = hex[(_U8)cur[i + 1]];
						if (v == 0xffff) ERROR_RETURN;
						cp = (cp << 4) | v;
					}
					if (cp <= 0x7f)
					{
						*(buffer++) = *(char*)&cp;
					}
					else if (cp <= 0x7FF)
					{
						*(buffer++) = (char)(0xC0 | (0x1f & (cp >> 6)));
						*(buffer++) = (char)(0x80 | (0x3f & cp));
					}
					else if (cp <= 0xFFFF)
					{
						*(buffer++) = 0xE0 | (char)((0xf & (cp >> 12)));
						*(buffer++) = 0x80 | (char)((0x3f & (cp >> 6)));
						*(buffer++) = (char)(0x80 | (0x3f & cp));
					}
					cur += 5;
					continue;
				}
				else
				{
					ERROR_RETURN;
				}
			}
			else
			{
				if (!dcopy) dcopy = cur;
				for (;;)
				{
					if ((++cur) == m_end) ERROR_RETURN;
					if (*cur == '"') break;
					if (*cur == '\\') break;
				}
			}
		}

		if (dcopy)
		{
			if (dcopy == start)
			{
				value = start;
				length = (_U32)(cur - start);
				return cur + 1;
			}
			size_t len = cur - dcopy;
			memcpy(buffer, dcopy, len);
			buffer += len;
		}

		value = m_buffer;
		length = (_U32)(buffer - m_buffer);
		return cur + 1;
	}

	static JsonValue g_NullValue;

	JsonValue::JsonValue() : m_type(TYPE_NULL)
	{
	}

	JsonValue::JsonValue(const JsonValue& val)
	{
		switch (val.m_type)
		{
		case TYPE_BOOL:		new (this) JsonValue(val.m_bool); break;
		case TYPE_U64:		new (this) JsonValue(val.m_u64); break;
		case TYPE_S64:		new (this) JsonValue(val.m_s64); break;
		case TYPE_F64:		new (this) JsonValue(val.m_f64); break;
		case TYPE_STR:		new (this) JsonValue(val.m_str.data, val.m_str.length); break;
		case TYPE_OBJECT:	new (this) JsonValue(val.m_object.names, val.m_object.values, val.m_object.count);
		case TYPE_ARRAY:	new (this) JsonValue(val.m_array.data, val.m_object.count);
		default:			m_type = val.m_type;
		}
	}

	JsonValue::JsonValue(TYPE type)
	{
		switch (type)
		{
		case TYPE_STR:		new (this) JsonValue((const char*)NULL, 0); break;
		case TYPE_OBJECT:	new (this) JsonValue((JsonValue*)NULL, (JsonValue*)NULL, 0); break;
		case TYPE_ARRAY:	new (this) JsonValue((JsonValue*)NULL, 0); break;
		default:			m_type = type;
		}
	}

	JsonValue::JsonValue(bool val) : m_type(TYPE_BOOL)
	{
		m_bool = val;
	}

	JsonValue::JsonValue(const char* val, _U32 len) : m_type(TYPE_STR)
	{
		if (len == (_U32)-1) len = (_U32)strlen(val);
		if (len > 0)
		{
			m_str.data = (char*)ZION_ALLOC(len + 1);
			m_str.length = len;
			memcpy(m_str.data, val, (size_t)len);
			m_str.data[len] = '\0';
		}
		else
		{
			m_str.data = NULL;
			m_str.length = 0;
		}
	}

	JsonValue::JsonValue(const Zion::String& val)
	{
		new (this) JsonValue(val.c_str(), (_U32)val.size());
	}

	JsonValue::JsonValue(_U64 val) : m_type(TYPE_U64)
	{
		m_u64 = val;
	}

	JsonValue::JsonValue(_S64 val) : m_type(TYPE_S64)
	{
		m_s64 = val;
	}

	JsonValue::JsonValue(_F64 val) : m_type(TYPE_F64)
	{
		m_f64 = val;
	}

	JsonValue::JsonValue(_U32 val) : m_type(TYPE_U64)
	{
		m_u64 = (_U64)val;
	}

	JsonValue::JsonValue(_S32 val) : m_type(TYPE_S64)
	{
		m_s64 = (_S32)val;
	}

	JsonValue::JsonValue(_F32 val) : m_type(TYPE_F64)
	{
		m_f64 = (_F64)val;
	}

	JsonValue::JsonValue(JsonValue* data, _U32 count, bool dcopy) : m_type(TYPE_ARRAY)
	{
		if (count > 0)
		{
			m_array.data = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*count);
			m_array.count = count;
			m_array.capacity = count;
			if (dcopy)
			{
				memcpy(m_array.data, data, sizeof(JsonValue)*count);
			}
			else
			{
				for (_U32 i = 0; i < count; i++)
				{
					new (m_array.data + i) JsonValue(data[i]);
				}
			}
		}
		else
		{
			m_array.data = NULL;
			m_array.count = 0;
			m_array.capacity = 0;
		}
	}

	JsonValue::JsonValue(JsonValue* names, JsonValue* values, _U32 count, bool dcopy) : m_type(TYPE_OBJECT)
	{
		if (count > 0)
		{
			m_object.names = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*count);
			m_object.values = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*count);
			m_object.count = count;
			m_object.capacity = count;
			if (dcopy)
			{
				memcpy(m_object.names, names, sizeof(JsonValue)*count);
				memcpy(m_object.values, values, sizeof(JsonValue)*count);
			}
			else
			{
				for (_U32 i = 0; i < count; i++)
				{
					new (m_object.names + i) JsonValue(names[i]);
					new (m_object.values + i) JsonValue(values[i]);
				}
			}
		}
		else
		{
			m_object.names = NULL;
			m_object.values = NULL;
			m_object.count = 0;
			m_object.capacity = 0;
		}
	}

	JsonValue::~JsonValue()
	{
		if (m_type == TYPE_ARRAY)
		{
			if (m_array.data)
			{
				for (_U32 i = 0; i < m_array.count; i++)
				{
					m_array.data[i].~JsonValue();
				}
				ZION_FREE(m_array.data);
			}
		}
		else if (m_type == TYPE_STR)
		{
			if (m_str.data)
			{
				ZION_FREE(m_str.data);
			}
		}
		else if (m_type == TYPE_OBJECT)
		{
			if (m_object.capacity > 0)
			{
				for (_U32 i = 0; i < m_object.count; i++)
				{
					m_object.names[i].~JsonValue();
					m_object.values[i].~JsonValue();
				}
				ZION_FREE(m_object.names);
				ZION_FREE(m_object.values);
			}
		}
	}

	bool JsonValue::Parse(const char* begin, const char* end)
	{
		if (!end) end = begin + strlen(begin);
		JsonNodeBuilder interop(*this);
		JsonReader reader;
		return reader.Parse(begin, end, interop);
	}

	static void StringifyString(OutputStringStream& sstream, const char* str, _U32 length)
	{
		sstream << "\"";

		for (size_t i = 0; i < length; i++)
		{
			char chr = str[i];

			switch (chr)
			{
			case '"':
				sstream << "\\\"";
				break;
			case '\\':
				sstream << "\\\\";
				break;
			case '/':
				sstream << "\\/";
				break;
			case '\b':
				sstream << "\\b";
				break;
			case '\f':
				sstream << "\\f";
				break;
			case '\n':
				sstream << "\\n";
				break;
			case '\r':
				sstream << "\\r";
				break;
			case '\t':
				sstream << "\\t";
				break;
			default:
				if ((_U8)chr >= 0x20)
				{
					sstream << chr;
				}
				else
				{
					char val[10];
					sprintf(val, "\\u%04x", (_U32)(_U8)chr);
					sstream << val;
				}
			}
		}

		sstream << "\"";
	}

	static void StringifyArray(OutputStringStream& sstream, const JsonValue* data, _U32 count)
	{
		sstream << "[";
		for (_U32 i = 0; i < count; i++)
		{
			if (i>0) sstream << ",";
			data[i].Stringify(sstream);
		}
		sstream << "]";
	}

	static void StringifyObject(OutputStringStream& sstream, const JsonValue* names, const JsonValue* values, _U32 count)
	{
		sstream << "{";
		for (_U32 i = 0; i < count; i++)
		{
			if (i>0) sstream << ",";
			names[i].Stringify(sstream);
			sstream << ":";
			values[i].Stringify(sstream);
		}
		sstream << "}";
	}

	void JsonValue::Stringify(OutputStringStream& sstream) const
	{
		switch (m_type)
		{
		case TYPE_NULL:		sstream << "null"; break;
		case TYPE_BOOL:		sstream << (m_bool ? "true" : "false"); break;
		case TYPE_U64:		sstream << m_u64; break;
		case TYPE_S64:		sstream << m_s64; break;
		case TYPE_F64:		sstream << m_f64; break;
		case TYPE_STR:		StringifyString(sstream, m_str.data, m_str.length); break;
		case TYPE_OBJECT:	StringifyObject(sstream, m_object.names, m_object.values, m_object.count); break;
		case TYPE_ARRAY:	StringifyArray(sstream, m_array.data, m_array.count); break;
		}
	}

	Zion::String JsonValue::Stringify() const
	{
		OutputStringStream sstream;
		Stringify(sstream);
		return sstream.str();
	}

	JsonValue::TYPE JsonValue::GetType() const
	{
		return m_type;
	}

	void JsonValue::SetType(TYPE type)
	{
		this->~JsonValue();
		new (this) JsonValue(type);
	}

	bool JsonValue::IsNull() const
	{
		return m_type == TYPE_NULL;
	}

	bool JsonValue::IsBool() const
	{
		return m_type == TYPE_BOOL;
	}

	bool JsonValue::IsSTR() const
	{
		return m_type == TYPE_STR;
	}

	bool JsonValue::IsU64() const
	{
		return m_type == TYPE_U64 || (m_type == TYPE_S64 && m_s64 >= 0);
	}

	bool JsonValue::IsS64() const
	{
		return m_type == TYPE_S64 || (m_type == TYPE_U64 && (m_u64 & 0x8000000000000000l) != 0);
	}

	bool JsonValue::IsF64() const
	{
		return m_type == TYPE_F64 || m_type == TYPE_U64 || m_type == TYPE_S64;
	}

	bool JsonValue::IsU32() const
	{
		switch (m_type)
		{
		case TYPE_U64:		return m_u64 <= 0xffffffff;
		case TYPE_S64:		return m_s64 >= 0 && m_s64 <= 0xffffffff;
		default:			return false;
		}
	}

	bool JsonValue::IsS32() const
	{
		switch (m_type)
		{
		case TYPE_U64:		return m_u64 <= 0x7fffffff;
		case TYPE_S64:		return m_s64 >= (0x7fffffff * -1) && m_s64 <= 0x7fffffff;
		default:			return false;
		}
	}

	bool JsonValue::IsF32() const
	{
		return m_type == TYPE_F64 || m_type == TYPE_U64 || m_type == TYPE_S64;
	}

	bool JsonValue::IsObject() const
	{
		return m_type == TYPE_OBJECT;
	}

	bool JsonValue::IsArray() const
	{
		return m_type == TYPE_ARRAY;
	}

	bool JsonValue::AsBool() const
	{
		if (m_type != TYPE_BOOL) return false;
		return m_bool;
	}

	const char* JsonValue::AsSTR() const
	{
		if (m_type == TYPE_STR && m_str.data)
		{
			return m_str.data;
		}
		else
		{
			return "";
		}
	}

	_U32 JsonValue::GetStringLength() const
	{
		if (m_type == TYPE_STR)
		{
			return m_str.length;			
		}
		else
		{
			return 0;
		}
	}

	_U64 JsonValue::AsU64() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return m_u64;
		case TYPE_S64:
			return (_U64)m_s64;
		default:
			return 0;
		}
	}

	_S64 JsonValue::AsS64() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return (_S64)m_u64;
		case TYPE_S64:
			return m_s64;
		default:
			return 0;
		}
	}

	_F64 JsonValue::AsF64() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return (_F64)m_u64;
		case TYPE_S64:
			return (_F64)m_s64;
		case TYPE_F64:
			return m_f64;
		default:
			return 0;
		}
	}

	_U32 JsonValue::AsU32() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return (_U32)m_u64;
		case TYPE_S64:
			return (_U32)m_s64;
		default:
			return 0;
		}
	}

	_S32 JsonValue::AsS32() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return (_S32)m_u64;
		case TYPE_S64:
			return (_S32)m_s64;
		default:
			return 0;
		}
	}

	_F32 JsonValue::AsF32() const
	{
		switch (m_type)
		{
		case TYPE_U64:
			return (_F32)m_u64;
		case TYPE_S64:
			return (_F32)m_s64;
		case TYPE_F64:
			return (_F32)m_f64;
		default:
			return 0;
		}
	}

	const JsonValue* JsonValue::GetArray() const
	{
		ZION_ASSERT(m_type == TYPE_ARRAY);
		if (m_type != TYPE_ARRAY) return NULL;
		return m_array.data;
	}

	JsonValue& JsonValue::Get(_U32 index)
	{
		if (m_type != TYPE_ARRAY || index >= m_array.count) return g_NullValue;
		return m_array.data[index];
	}

	const JsonValue& JsonValue::Get(_U32 index) const
	{
		if (m_type != TYPE_ARRAY || index >= m_array.count) return g_NullValue;
		return m_array.data[index];
	}

	_U32 JsonValue::GetArrayLength() const
	{
		if (m_type != TYPE_ARRAY) return 0;
		return m_array.count;
	}

	const JsonValue* JsonValue::GetMemberNames() const
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return NULL;
		return m_object.names;
	}

	const JsonValue* JsonValue::GetMemberValues() const
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return 0;
		return m_object.values;
	}

	_U32 JsonValue::GetMemberCount() const
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return 0;
		return m_object.count;
	}

	bool JsonValue::HasMember(const char* name, _U32 length) const
	{
		return GetMember(name, length) != NULL;
	}

	bool JsonValue::HasMember(const Zion::String& name) const
	{
		return GetMember(name.c_str(), (_U32)name.size()) != NULL;
	}

	JsonValue* JsonValue::GetMember(const char* name, _U32 length)
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return NULL;
		if (length == (_U32)-1) length = (_U32)strlen(name);
		for (_U32 i = 0; i < m_object.count; i++)
		{
			JsonValue& n = m_object.names[i];
			if (n.m_str.length == length)
			{
				if (memcmp(n.m_str.data, name, (size_t)length) == 0)
				{
					return m_object.values + i;
				}
			}
		}
		return NULL;
	}

	const JsonValue* JsonValue::GetMember(const char* name, _U32 length) const
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return NULL;
		if (length == (_U32)-1) length = (_U32)strlen(name);
		for (_U32 i = 0; i < m_object.count; i++)
		{
			const JsonValue& n = m_object.names[i];
			if (n.m_str.length == length)
			{
				if (memcmp(n.m_str.data, name, (size_t)length) == 0)
				{
					return m_object.values + i;
				}
			}
		}
		return NULL;
	}

	JsonValue* JsonValue::GetMember(const Zion::String& name)
	{
		return GetMember(name.c_str(), (_U32)name.size());
	}

	const JsonValue* JsonValue::GetMember(const Zion::String& name) const
	{
		return GetMember(name.c_str(), (_U32)name.size());
	}

	void JsonValue::Set(const JsonValue& val)
	{
		this->~JsonValue();
		new (this) JsonValue(val);
	}

	void JsonValue::Set(bool val)
	{
		SetType(TYPE_BOOL);
		m_bool = val;
	}

	void JsonValue::Set(const char* val, _U32 len)
	{
		if (len == (_U32)-1) len = (_U32)strlen(val);
		this->~JsonValue();
		new (this) JsonValue(val, len);
	}

	void JsonValue::Set(const _U64 val)
	{
		this->~JsonValue();
		m_type = TYPE_U64;
		m_u64 = val;
	}

	void JsonValue::Set(const _S64 val)
	{
		this->~JsonValue();
		m_type = TYPE_S64;
		m_s64 = val;
	}

	void JsonValue::Set(const _F64 val)
	{
		this->~JsonValue();
		m_type = TYPE_F64;
		m_f64 = val;
	}

	void JsonValue::Set(const _U32 val)
	{
		this->~JsonValue();
		m_type = TYPE_U64;
		m_u64 = (_U64)val;
	}

	void JsonValue::Set(const _S32 val)
	{
		this->~JsonValue();
		m_type = TYPE_S64;
		m_s64 = (_S64)val;
	}

	void JsonValue::Set(const _F32 val)
	{
		this->~JsonValue();
		m_type = TYPE_F64;
		m_f64 = (_F64)val;
	}

	JsonValue& JsonValue::operator[](_U32 index)
	{
		ZION_ASSERT(m_type == TYPE_ARRAY);
		if (m_type != TYPE_ARRAY || index >= m_array.count) return g_NullValue;
		return m_array.data[index];
	}

	const JsonValue& JsonValue::operator[](_U32 index) const
	{
		ZION_ASSERT(m_type == TYPE_ARRAY);
		if (m_type != TYPE_ARRAY || index >= m_array.count) return g_NullValue;
		return m_array.data[index];
	}

	JsonValue& JsonValue::operator[](const char* name)
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return g_NullValue;
		JsonValue* object = GetMember(name);
		if (!object) return g_NullValue;
		return *object;
	}

	const JsonValue& JsonValue::operator[](const char* name) const
	{
		ZION_ASSERT(m_type == TYPE_OBJECT);
		if (m_type != TYPE_OBJECT) return g_NullValue;
		const JsonValue* object = GetMember(name);
		if (!object) return g_NullValue;
		return *object;
	}

	JsonValue* JsonValue::Append(const JsonValue& val)
	{
		if (m_type != TYPE_ARRAY) return NULL;

		if (m_array.capacity == m_array.count)
		{
			_U32 capacity = m_array.capacity + 2000;
			JsonValue* data = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*capacity);
			if (m_array.capacity > 0)
			{
				if (m_array.count > 0)
				{
					memcpy(data, m_array.data, sizeof(JsonValue)*m_array.count);
				}
				ZION_FREE(m_array.data);
			}
			m_array.data = data;
			m_array.capacity = capacity;
		}

		JsonValue* retval = m_array.data + m_array.count;
		new (retval)JsonValue(val);
		m_array.count += 1;
		return retval;
	}

	JsonValue* JsonValue::Append(const char* name, const JsonValue& val, bool safety)
	{
		return Append(name, (_U32)strlen(name), val, safety);
	}

	JsonValue* JsonValue::Append(const char* name, _U32 name_len, const JsonValue& val, bool safety)
	{
		if (m_type != TYPE_OBJECT) return NULL;
		if (safety && HasMember(name, name_len)) return NULL;

		if (m_object.capacity == m_object.count)
		{
			_U32 capacity = m_object.capacity + 100;
			JsonValue* names = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*capacity);
			JsonValue* values = (JsonValue*)ZION_ALLOC(sizeof(JsonValue)*capacity);
			if (m_object.capacity > 0)
			{
				if (m_object.count > 0)
				{
					memcpy(names, m_object.names, sizeof(JsonValue)*m_object.count);
					memcpy(values, m_object.values, sizeof(JsonValue)*m_object.count);
				}
				ZION_FREE(m_object.names);
				ZION_FREE(m_object.values);
			}
			m_object.names = names;
			m_object.values = values;
			m_object.capacity = capacity;
		}

		JsonValue* retval = m_object.values + m_object.count;
		new (m_object.names + m_object.count) JsonValue(name, name_len);
		new (retval)JsonValue(val);
		m_object.count += 1;
		return retval;
	}

	JsonNodeBuilder::JsonNodeBuilder(JsonValue& root)
	{
		m_StackDepth = 1;
		m_Stack[0] = &root;
	}

	JsonNodeBuilder::~JsonNodeBuilder()
	{
	}

	bool JsonNodeBuilder::StartObject()
	{
		m_Stack[m_StackDepth - 1]->SetType(JsonValue::TYPE_OBJECT);
		return true;
	}

	bool JsonNodeBuilder::NewElement(const char* name, _U32 name_len)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[m_StackDepth - 1];
		ZION_ASSERT(node->IsObject());
		m_Stack[m_StackDepth++] = node->Append(name, name_len, g_NullValue, false);
		return true;
	}

	bool JsonNodeBuilder::EndObject()
	{
		ZION_ASSERT(m_StackDepth > 0);
		ZION_ASSERT(m_Stack[--m_StackDepth]->IsObject());
		return true;
	}

	bool JsonNodeBuilder::StartArray()
	{
		m_Stack[m_StackDepth - 1]->SetType(JsonValue::TYPE_ARRAY);
		return true;
	}

	bool JsonNodeBuilder::NewElement()
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[m_StackDepth - 1];
		ZION_ASSERT(node->IsArray());
		m_Stack[m_StackDepth++] = node->Append(g_NullValue);
		return true;
	}

	bool JsonNodeBuilder::EndArray()
	{
		ZION_ASSERT(m_StackDepth > 0);
		ZION_ASSERT(m_Stack[--m_StackDepth]->IsArray());
		return true;
	}

	bool JsonNodeBuilder::NewValueS64(_S64 value)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->Set(value);
		return true;
	}

	bool JsonNodeBuilder::NewValueU64(_U64 value)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->Set(value);
		return true;
	}

	bool JsonNodeBuilder::NewValueF64(_F64 value)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->Set(value);
		return true;
	}

	bool JsonNodeBuilder::NewValueStr(const char* value, _U32 len)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->Set(value, len);
		return true;
	}

	bool JsonNodeBuilder::NewValueBool(bool value)
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->Set(value);
		return true;
	}

	bool JsonNodeBuilder::NewValueNull()
	{
		ZION_ASSERT(m_StackDepth > 0);
		JsonValue* node = m_Stack[--m_StackDepth];
		node->SetType(JsonValue::TYPE_NULL);
		return true;
	}

	JsonStringBuilder::JsonStringBuilder(OutputStringStream* sstream) : m_sstream(sstream)
	{

	}

	JsonStringBuilder::~JsonStringBuilder()
	{

	}

	bool JsonStringBuilder::StartObject()
	{
		*m_sstream << "{";
		m_MemberCount = 0;
		return true;
	}

	bool JsonStringBuilder::NewElement(const char* name, _U32 name_len)
	{
		if ((m_MemberCount++) == 0) *m_sstream << ",";
		StringifyString(*m_sstream, name, name_len);
		*m_sstream << ":";
		return true;
	}

	bool JsonStringBuilder::EndObject()
	{
		*m_sstream << "}";
		return true;
	}

	bool JsonStringBuilder::StartArray()
	{
		*m_sstream << "[";
		m_MemberCount = 0;
		return true;
	}

	bool JsonStringBuilder::NewElement()
	{
		if ((m_MemberCount++) == 0) *m_sstream << ",";
		return true;
	}

	bool JsonStringBuilder::EndArray()
	{
		*m_sstream << "]";
		return true;
	}

	bool JsonStringBuilder::NewValueS64(_S64 value)
	{
		*m_sstream << value;
		return true;
	}

	bool JsonStringBuilder::NewValueU64(_U64 value)
	{
		*m_sstream << value;
		return true;
	}

	bool JsonStringBuilder::NewValueF64(_F64 value)
	{
		*m_sstream << value;
		return true;
	}

	bool JsonStringBuilder::NewValueStr(const char* value, _U32 len)
	{
		StringifyString(*m_sstream, value, len);
		return true;
	}

	bool JsonStringBuilder::NewValueBool(bool value)
	{
		*m_sstream << (value ? "true" : "false");
		return true;
	}

	bool JsonStringBuilder::NewValueNull()
	{
		*m_sstream << "null";
		return true;
	}

}
