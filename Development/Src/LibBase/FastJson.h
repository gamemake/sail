#ifndef __FAST_JSON__
#define __FAST_JSON__

namespace Zion
{

	class JsonValue;

	class JsonInterop
	{
	public:
		virtual ~JsonInterop() {}

		virtual bool StartObject() = 0;
		virtual bool NewElement(const char* name, _U32 name_len) = 0;
		virtual bool EndObject() = 0;
		virtual bool StartArray() = 0;
		virtual bool NewElement() = 0;
		virtual bool EndArray() = 0;
		virtual bool NewValueS64(_S64 value) = 0;
		virtual bool NewValueU64(_U64 value) = 0;
		virtual bool NewValueF64(_F64 value) = 0;
		virtual bool NewValueStr(const char* value, _U32 len) = 0;
		virtual bool NewValueBool(bool value) = 0;
		virtual bool NewValueNull() = 0;
	};

	class JsonReader
	{
	public:
		bool Parse(const char* begin, const char* end, JsonInterop& interop);

	protected:
		const char* Skip(const char* cur);
		const char* ParseElement(const char* start);
		const char* ParseNumber(const char* start);
		const char* ParseString(const char* start);
		const char* ParseArray(const char* start);
		const char* ParseObject(const char* start);
		const char* ParseStringValue(const char* start, const char*& value, _U32& length);

	private:
		JsonInterop* m_interop;
		const char* m_begin;
		const char* m_end;
		char m_buffer[10 * 1024];
	};

	class JsonNodeBuilder : public JsonInterop
	{
	public:
		JsonNodeBuilder(JsonValue& root);
		virtual ~JsonNodeBuilder();

		virtual bool StartObject();
		virtual bool NewElement(const char* name, _U32 name_len);
		virtual bool EndObject();
		virtual bool StartArray();
		virtual bool NewElement();
		virtual bool EndArray();
		virtual bool NewValueS64(_S64 value);
		virtual bool NewValueU64(_U64 value);
		virtual bool NewValueF64(_F64 value);
		virtual bool NewValueStr(const char* value, _U32 len);
		virtual bool NewValueBool(bool value);
		virtual bool NewValueNull();

	private:
		JsonValue*	m_Stack[1024];
		_U32 m_StackDepth;
	};

	class JsonStringBuilder : public JsonInterop
	{
	public:
		JsonStringBuilder(OutputStringStream* sstream);
		virtual ~JsonStringBuilder();

		virtual bool StartObject();
		virtual bool NewElement(const char* name, _U32 name_len);
		virtual bool EndObject();
		virtual bool StartArray();
		virtual bool NewElement();
		virtual bool EndArray();
		virtual bool NewValueS64(_S64 value);
		virtual bool NewValueU64(_U64 value);
		virtual bool NewValueF64(_F64 value);
		virtual bool NewValueStr(const char* value, _U32 len);
		virtual bool NewValueBool(bool value);
		virtual bool NewValueNull();

	private:
		_U32 m_MemberCount;
		OutputStringStream* m_sstream;
	};

	class JsonValue
	{
	public:

		enum TYPE
		{
			TYPE_NULL,
			TYPE_BOOL,
			TYPE_U64,
			TYPE_S64,
			TYPE_F64,
			TYPE_STR,
			TYPE_OBJECT,
			TYPE_ARRAY,
		};

		struct String
		{
			char* data;
			_U32 length;
		};

		struct Array
		{
			JsonValue* data;
			_U32 count;
			_U32 capacity;
		};

		struct Object
		{
			JsonValue* names;
			JsonValue* values;
			_U32 count;
			_U32 capacity;
		};

		JsonValue();
		JsonValue(const JsonValue& val);
		JsonValue(TYPE type);
		JsonValue(bool val);
		JsonValue(const char* val, _U32 len = (_U32)-1);
		JsonValue(const Zion::String& val);
		JsonValue(_U64 val);
		JsonValue(_S64 val);
		JsonValue(_F64 val);
		JsonValue(_U32 val);
		JsonValue(_S32 val);
		JsonValue(_F32 val);
		JsonValue(JsonValue* data, _U32 count, bool dcopy = false);
		JsonValue(JsonValue* names, JsonValue* values, _U32 count, bool dcopy = false);
		~JsonValue();

		bool Parse(const char* begin, const char* end);
		void Stringify(OutputStringStream& sstream) const;
		Zion::String Stringify() const;

		TYPE GetType() const;
		void SetType(TYPE type);

		bool IsNull() const;
		bool IsBool() const;
		bool IsU64() const;
		bool IsS64() const;
		bool IsF64() const;
		bool IsU32() const;
		bool IsS32() const;
		bool IsF32() const;
		bool IsSTR() const;
		bool IsObject() const;
		bool IsArray() const;

		bool AsBool() const;
		_U64 AsU64() const;
		_S64 AsS64() const;
		_F64 AsF64() const;
		_U32 AsU32() const;
		_S32 AsS32() const;
		_F32 AsF32() const;
		const char* AsSTR() const;
		_U32 GetStringLength() const;

		const JsonValue* GetArray() const;
		JsonValue& Get(_U32 index);
		const JsonValue& Get(_U32 index) const;
		_U32 GetArrayLength() const;

		const JsonValue* GetMemberNames() const;
		const JsonValue* GetMemberValues() const;
		_U32 GetMemberCount() const;

		bool HasMember(const char* name, _U32 length = (_U32)-1) const;
		bool HasMember(const Zion::String& name) const;
		JsonValue* GetMember(const char* name, _U32 length = (_U32)-1);
		const JsonValue* GetMember(const char* name, _U32 length = (_U32)-1) const;
		JsonValue* GetMember(const Zion::String& name);
		const JsonValue* GetMember(const Zion::String& name) const;

		void Set(const JsonValue& val);
		void Set(bool val);
		void Set(const char* val, _U32 len = (_U32)-1);
		void Set(_U64 val);
		void Set(_S64 val);
		void Set(_F64 val);
		void Set(_U32 val);
		void Set(_S32 val);
		void Set(_F32 val);

		inline JsonValue& operator=(const JsonValue& val)	{ Set(val); return *this; }
		inline JsonValue& operator=(bool val)				{ Set(val); return *this; }
		inline JsonValue& operator=(const char* val)		{ Set(val); return *this; }
		inline JsonValue& operator=(_U64 val)				{ Set(val); return *this; }
		inline JsonValue& operator=(_S64 val)				{ Set(val); return *this; }
		inline JsonValue& operator=(_F64 val)				{ Set(val); return *this; }
		inline JsonValue& operator=(_U32 val)				{ Set(val); return *this; }
		inline JsonValue& operator=(_S32 val)				{ Set(val); return *this; }
		inline JsonValue& operator=(_F32 val)				{ Set(val); return *this; }

		JsonValue& operator[](_U32 index);
		const JsonValue& operator[](_U32 index) const;
		JsonValue& operator[](const char* name);
		const JsonValue& operator[](const char* name) const;

		JsonValue* Append(const JsonValue& val);
		JsonValue* Append(const char* name, const JsonValue& val, bool safety = true);
		JsonValue* Append(const char* name, _U32 name_len, const JsonValue& val, bool safety = true);

	private:
		TYPE	m_type;
		union
		{
			bool		m_bool;
			String		m_str;
			_U64		m_u64;
			_S64		m_s64;
			_F64		m_f64;
			Object		m_object;
			Array		m_array;
		};
	};

}

#endif
