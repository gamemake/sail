////////////////////////////////////////////////
// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //
////////////////////////////////////////////////

#ifndef __DATASYNCDDL_DDL_CODEGEN__
#define __DATASYNCDDL_DDL_CODEGEN__

#pragma pack(push)
#pragma pack(4)

#include "CommonDDL.h"

const _U32 SYNCFLAG_DATAFORMAT_MASK = 1;

const _U32 SYNCFLAG_BINARY = 0;

const _U32 SYNCFLAG_JSON = 1;

const _U32 SYNCFLAG_MODE_MASK = 14;

const _U32 SYNCFLAG_CLIENT = 2;

const _U32 SYNCFLAG_SERVER = 4;

const _U32 SYNCMODE_VERIFY = 8;

class DATASYNC_JSON_C2S;

namespace DDLReflect
{
	template<>
	const CLASS_INFO* GetClass< DATASYNC_JSON_C2S >();
}

class DATASYNC_JSON_S2C;

namespace DDLReflect
{
	template<>
	const CLASS_INFO* GetClass< DATASYNC_JSON_S2C >();
}

namespace DDLStub
{

	template<typename CLASS>
	class DATASYNC_JSON_C2S : public DDLStub<CLASS>
	{
	public:
		DATASYNC_JSON_C2S(CLASS* Class) : DDLStub<CLASS>(Class)
		{
		}

		virtual const DDLReflect::CLASS_INFO* GetClassInfo()
		{
			return DDLReflect::GetClass< ::DATASYNC_JSON_C2S >();
		}
		
		virtual bool Dispatcher(_U16 fid, DDL::BufferReader& Buf)
		{
			if(fid==0)
			{
				_U32 __length;
				char* _prefix_type;
				char* _prefix_data;

				// <string> <type> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_type = (char*)alloca(sizeof(_prefix_type[0])*(__length+1));
				if(!_prefix_type) return false;
				if(!Buf.ReadBuffer(_prefix_type, (unsigned int)sizeof(_prefix_type[0])*__length)) return false;
				_prefix_type[__length] = '\0';
				// <string> <data> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_data = (char*)alloca(sizeof(_prefix_data[0])*(__length+1));
				if(!_prefix_data) return false;
				if(!Buf.ReadBuffer(_prefix_data, (unsigned int)sizeof(_prefix_data[0])*__length)) return false;
				_prefix_data[__length] = '\0';

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObject(_prefix_type, _prefix_data);
				return true;
			}
			if(fid==1)
			{
				_U32 __length;
				A_UUID _prefix__uuid;
				char* _prefix_data;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;
				// <string> <data> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_data = (char*)alloca(sizeof(_prefix_data[0])*(__length+1));
				if(!_prefix_data) return false;
				if(!Buf.ReadBuffer(_prefix_data, (unsigned int)sizeof(_prefix_data[0])*__length)) return false;
				_prefix_data[__length] = '\0';

				// call implement
				DDLStub<CLASS>::GetClass()->DS_UpdateObject(_prefix__uuid, _prefix_data);
				return true;
			}
			if(fid==2)
			{
				_U32 __length;
				A_UUID* _prefix__uuid;
				_U32 _prefix_count;

				// <A_UUID> <_uuid> <> <count>;
				if(!Buf.Read(__length)) return false;
				_prefix__uuid = (A_UUID*)alloca(sizeof(_prefix__uuid[0])*__length);
				if(!_prefix__uuid) return false;
				if(!Buf.ReadPointer(_prefix__uuid, __length)) return false;
				// <_U32> <count> <> <>;
				if(!Buf.Read(_prefix_count)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_DeleteObject(_prefix__uuid, _prefix_count);
				return true;
			}
			return false;
		}
	};

}

namespace DDLProxy
{

	template<_U32 BUF_SIZE>
	class DATASYNC_JSON_C2S : public DDLProxy<BUF_SIZE>
	{
	public:
		DATASYNC_JSON_C2S(IClient* Client) : DDLProxy<BUF_SIZE>(Client, DDLReflect::GetClassID<typename ::DATASYNC_JSON_C2S>())
		{
		}

		bool DS_CreateObject(const char* type, const char* data)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <string> <type> <> <>
			__length = DDL::StringLength(type);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(type, (unsigned int)sizeof(type[0])*__length)) return false;
			// <string> <data> <> <>
			__length = DDL::StringLength(data);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(data, (unsigned int)sizeof(data[0])*__length)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 0, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_UpdateObject(const A_UUID& _uuid, const char* data)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;
			// <string> <data> <> <>
			__length = DDL::StringLength(data);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(data, (unsigned int)sizeof(data[0])*__length)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 1, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_DeleteObject(const A_UUID* _uuid, _U32 count)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <count>
			__length = (_U16)(count);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(_uuid, __length)) return false;
			// <_U32> <count> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(count)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 2, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}
	};

}

namespace DDLStub
{

	template<typename CLASS>
	class DATASYNC_JSON_S2C : public DDLStub<CLASS>
	{
	public:
		DATASYNC_JSON_S2C(CLASS* Class) : DDLStub<CLASS>(Class)
		{
		}

		virtual const DDLReflect::CLASS_INFO* GetClassInfo()
		{
			return DDLReflect::GetClass< ::DATASYNC_JSON_S2C >();
		}
		
		virtual bool Dispatcher(_U16 fid, DDL::BufferReader& Buf)
		{
			if(fid==0)
			{
				_U32 _prefix_flag;

				// <_U32> <flag> <> <>;
				if(!Buf.Read(_prefix_flag)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncOpen(_prefix_flag);
				return true;
			}
			if(fid==1)
			{
				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncReady();
				return true;
			}
			if(fid==2)
			{
				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncClose();
				return true;
			}
			if(fid==3)
			{
				A_UUID _prefix__uuid;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObjectDone(_prefix__uuid);
				return true;
			}
			if(fid==4)
			{
				_U32 __length;
				char* _prefix_type;
				char* _prefix_json;

				// <string> <type> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_type = (char*)alloca(sizeof(_prefix_type[0])*(__length+1));
				if(!_prefix_type) return false;
				if(!Buf.ReadBuffer(_prefix_type, (unsigned int)sizeof(_prefix_type[0])*__length)) return false;
				_prefix_type[__length] = '\0';
				// <string> <json> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_json = (char*)alloca(sizeof(_prefix_json[0])*(__length+1));
				if(!_prefix_json) return false;
				if(!Buf.ReadBuffer(_prefix_json, (unsigned int)sizeof(_prefix_json[0])*__length)) return false;
				_prefix_json[__length] = '\0';

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObject(_prefix_type, _prefix_json);
				return true;
			}
			if(fid==5)
			{
				_U32 __length;
				A_UUID _prefix__uuid;
				char* _prefix_json;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;
				// <string> <json> <> <>;
				if(!Buf.Read(__length)) return false;
				_prefix_json = (char*)alloca(sizeof(_prefix_json[0])*(__length+1));
				if(!_prefix_json) return false;
				if(!Buf.ReadBuffer(_prefix_json, (unsigned int)sizeof(_prefix_json[0])*__length)) return false;
				_prefix_json[__length] = '\0';

				// call implement
				DDLStub<CLASS>::GetClass()->DS_UpdateObject(_prefix__uuid, _prefix_json);
				return true;
			}
			if(fid==6)
			{
				_U32 __length;
				A_UUID* _prefix__uuid;
				_U32 _prefix_count;

				// <A_UUID> <_uuid> <> <count>;
				if(!Buf.Read(__length)) return false;
				_prefix__uuid = (A_UUID*)alloca(sizeof(_prefix__uuid[0])*__length);
				if(!_prefix__uuid) return false;
				if(!Buf.ReadPointer(_prefix__uuid, __length)) return false;
				// <_U32> <count> <> <>;
				if(!Buf.Read(_prefix_count)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_DeleteObject(_prefix__uuid, _prefix_count);
				return true;
			}
			return false;
		}
	};

}

namespace DDLProxy
{

	template<_U32 BUF_SIZE>
	class DATASYNC_JSON_S2C : public DDLProxy<BUF_SIZE>
	{
	public:
		DATASYNC_JSON_S2C(IClient* Client) : DDLProxy<BUF_SIZE>(Client, DDLReflect::GetClassID<typename ::DATASYNC_JSON_S2C>())
		{
		}

		bool DS_SyncOpen(_U32 flag)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// <_U32> <flag> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(flag)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 0, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_SyncReady()
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 1, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_SyncClose()
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 2, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_CreateObjectDone(const A_UUID& _uuid)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 3, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_CreateObject(const char* type, const char* json)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <string> <type> <> <>
			__length = DDL::StringLength(type);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(type, (unsigned int)sizeof(type[0])*__length)) return false;
			// <string> <json> <> <>
			__length = DDL::StringLength(json);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(json, (unsigned int)sizeof(json[0])*__length)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 4, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_UpdateObject(const A_UUID& _uuid, const char* json)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;
			// <string> <json> <> <>
			__length = DDL::StringLength(json);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WriteData(json, (unsigned int)sizeof(json[0])*__length)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 5, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_DeleteObject(const A_UUID* _uuid, _U32 count)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <count>
			__length = (_U16)(count);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(_uuid, __length)) return false;
			// <_U32> <count> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(count)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 6, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}
	};

}

class DATASYNC_BINARY_C2S;

namespace DDLReflect
{
	template<>
	const CLASS_INFO* GetClass< DATASYNC_BINARY_C2S >();
}

class DATASYNC_BINARY_S2C;

namespace DDLReflect
{
	template<>
	const CLASS_INFO* GetClass< DATASYNC_BINARY_S2C >();
}

namespace DDLStub
{

	template<typename CLASS>
	class DATASYNC_BINARY_C2S : public DDLStub<CLASS>
	{
	public:
		DATASYNC_BINARY_C2S(CLASS* Class) : DDLStub<CLASS>(Class)
		{
		}

		virtual const DDLReflect::CLASS_INFO* GetClassInfo()
		{
			return DDLReflect::GetClass< ::DATASYNC_BINARY_C2S >();
		}
		
		virtual bool Dispatcher(_U16 fid, DDL::BufferReader& Buf)
		{
			if(fid==0)
			{
				_U32 __length;
				_U16 _prefix_type;
				_U8* _prefix_buf;
				_U32 _prefix_len;

				// <_U16> <type> <> <>;
				if(!Buf.Read(_prefix_type)) return false;
				// <_U8> <buf> <> <len>;
				if(!Buf.Read(__length)) return false;
				_prefix_buf = (_U8*)alloca(sizeof(_prefix_buf[0])*__length);
				if(!_prefix_buf) return false;
				if(!Buf.ReadPointer(_prefix_buf, __length)) return false;
				// <_U32> <len> <> <>;
				if(!Buf.Read(_prefix_len)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObject(_prefix_type, _prefix_buf, _prefix_len);
				return true;
			}
			if(fid==1)
			{
				_U32 __length;
				A_UUID _prefix__uuid;
				_U8* _prefix_buf;
				_U32 _prefix_len;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;
				// <_U8> <buf> <> <len>;
				if(!Buf.Read(__length)) return false;
				_prefix_buf = (_U8*)alloca(sizeof(_prefix_buf[0])*__length);
				if(!_prefix_buf) return false;
				if(!Buf.ReadPointer(_prefix_buf, __length)) return false;
				// <_U32> <len> <> <>;
				if(!Buf.Read(_prefix_len)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_UpdateObject(_prefix__uuid, _prefix_buf, _prefix_len);
				return true;
			}
			if(fid==2)
			{
				_U32 __length;
				A_UUID* _prefix__uuid;
				_U32 _prefix_count;

				// <A_UUID> <_uuid> <> <count>;
				if(!Buf.Read(__length)) return false;
				_prefix__uuid = (A_UUID*)alloca(sizeof(_prefix__uuid[0])*__length);
				if(!_prefix__uuid) return false;
				if(!Buf.ReadPointer(_prefix__uuid, __length)) return false;
				// <_U32> <count> <> <>;
				if(!Buf.Read(_prefix_count)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_DeleteObject(_prefix__uuid, _prefix_count);
				return true;
			}
			return false;
		}
	};

}

namespace DDLProxy
{

	template<_U32 BUF_SIZE>
	class DATASYNC_BINARY_C2S : public DDLProxy<BUF_SIZE>
	{
	public:
		DATASYNC_BINARY_C2S(IClient* Client) : DDLProxy<BUF_SIZE>(Client, DDLReflect::GetClassID<typename ::DATASYNC_BINARY_C2S>())
		{
		}

		bool DS_CreateObject(_U16 type, const _U8* buf, _U32 len)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <_U16> <type> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(type)) return false;
			// <_U8> <buf> <> <len>
			__length = (_U16)(len);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(buf, __length)) return false;
			// <_U32> <len> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(len)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 0, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_UpdateObject(const A_UUID& _uuid, const _U8* buf, _U32 len)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;
			// <_U8> <buf> <> <len>
			__length = (_U16)(len);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(buf, __length)) return false;
			// <_U32> <len> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(len)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 1, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_DeleteObject(const A_UUID* _uuid, _U32 count)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <count>
			__length = (_U16)(count);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(_uuid, __length)) return false;
			// <_U32> <count> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(count)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 2, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}
	};

}

namespace DDLStub
{

	template<typename CLASS>
	class DATASYNC_BINARY_S2C : public DDLStub<CLASS>
	{
	public:
		DATASYNC_BINARY_S2C(CLASS* Class) : DDLStub<CLASS>(Class)
		{
		}

		virtual const DDLReflect::CLASS_INFO* GetClassInfo()
		{
			return DDLReflect::GetClass< ::DATASYNC_BINARY_S2C >();
		}
		
		virtual bool Dispatcher(_U16 fid, DDL::BufferReader& Buf)
		{
			if(fid==0)
			{
				_U32 _prefix_flag;

				// <_U32> <flag> <> <>;
				if(!Buf.Read(_prefix_flag)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncOpen(_prefix_flag);
				return true;
			}
			if(fid==1)
			{
				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncReady();
				return true;
			}
			if(fid==2)
			{
				// call implement
				DDLStub<CLASS>::GetClass()->DS_SyncClose();
				return true;
			}
			if(fid==3)
			{
				A_UUID _prefix__uuid;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObjectDone(_prefix__uuid);
				return true;
			}
			if(fid==4)
			{
				_U32 __length;
				_U16 _prefix_type;
				_U8* _prefix_buf;
				_U32 _prefix_len;

				// <_U16> <type> <> <>;
				if(!Buf.Read(_prefix_type)) return false;
				// <_U8> <buf> <> <len>;
				if(!Buf.Read(__length)) return false;
				_prefix_buf = (_U8*)alloca(sizeof(_prefix_buf[0])*__length);
				if(!_prefix_buf) return false;
				if(!Buf.ReadPointer(_prefix_buf, __length)) return false;
				// <_U32> <len> <> <>;
				if(!Buf.Read(_prefix_len)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_CreateObject(_prefix_type, _prefix_buf, _prefix_len);
				return true;
			}
			if(fid==5)
			{
				_U32 __length;
				A_UUID _prefix__uuid;
				_U8* _prefix_buf;
				_U32 _prefix_len;

				// <A_UUID> <_uuid> <> <>;
				if(!Buf.Read(_prefix__uuid)) return false;
				// <_U8> <buf> <> <len>;
				if(!Buf.Read(__length)) return false;
				_prefix_buf = (_U8*)alloca(sizeof(_prefix_buf[0])*__length);
				if(!_prefix_buf) return false;
				if(!Buf.ReadPointer(_prefix_buf, __length)) return false;
				// <_U32> <len> <> <>;
				if(!Buf.Read(_prefix_len)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_UpdateObject(_prefix__uuid, _prefix_buf, _prefix_len);
				return true;
			}
			if(fid==6)
			{
				_U32 __length;
				A_UUID* _prefix__uuid;
				_U32 _prefix_count;

				// <A_UUID> <_uuid> <> <count>;
				if(!Buf.Read(__length)) return false;
				_prefix__uuid = (A_UUID*)alloca(sizeof(_prefix__uuid[0])*__length);
				if(!_prefix__uuid) return false;
				if(!Buf.ReadPointer(_prefix__uuid, __length)) return false;
				// <_U32> <count> <> <>;
				if(!Buf.Read(_prefix_count)) return false;

				// call implement
				DDLStub<CLASS>::GetClass()->DS_DeleteObject(_prefix__uuid, _prefix_count);
				return true;
			}
			return false;
		}
	};

}

namespace DDLProxy
{

	template<_U32 BUF_SIZE>
	class DATASYNC_BINARY_S2C : public DDLProxy<BUF_SIZE>
	{
	public:
		DATASYNC_BINARY_S2C(IClient* Client) : DDLProxy<BUF_SIZE>(Client, DDLReflect::GetClassID<typename ::DATASYNC_BINARY_S2C>())
		{
		}

		bool DS_SyncOpen(_U32 flag)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// <_U32> <flag> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(flag)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 0, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_SyncReady()
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 1, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_SyncClose()
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 2, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_CreateObjectDone(const A_UUID& _uuid)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 3, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_CreateObject(_U16 type, const _U8* buf, _U32 len)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <_U16> <type> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(type)) return false;
			// <_U8> <buf> <> <len>
			__length = (_U16)(len);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(buf, __length)) return false;
			// <_U32> <len> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(len)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 4, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_UpdateObject(const A_UUID& _uuid, const _U8* buf, _U32 len)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(_uuid)) return false;
			// <_U8> <buf> <> <len>
			__length = (_U16)(len);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(buf, __length)) return false;
			// <_U32> <len> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(len)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 5, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}

		bool DS_DeleteObject(const A_UUID* _uuid, _U32 count)
		{
			DDLProxy<BUF_SIZE>::_Buf.Reset();

			_U32 __length;
			// <A_UUID> <_uuid> <> <count>
			__length = (_U16)(count);
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(__length)) return false;
			if(!DDLProxy<BUF_SIZE>::_Buf.WritePointer(_uuid, __length)) return false;
			// <_U32> <count> <> <>
			if(!DDLProxy<BUF_SIZE>::_Buf.Write(count)) return false;

			// send
			return DDLProxy<BUF_SIZE>::GetClient()->SendData(this->GetClassID(), 6, DDLProxy<BUF_SIZE>::_Buf.GetSize(), DDLProxy<BUF_SIZE>::_Buf.GetBuf());
		}
	};

}


#pragma pack(pop)

#endif
