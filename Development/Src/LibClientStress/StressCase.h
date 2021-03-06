#ifndef __ZION_STRESS_CASE__
#define __ZION_STRESS_CASE__

namespace Zion
{

	class CClient;
	class CStressClient;
	class CStressManager;

	class CStressCase : public sigslot::has_slots<>
	{
		friend class CStressClient;
		friend class CStressManager;
	public:
		CStressCase(const char* name);
		virtual ~CStressCase();

		CClient* GetClient();
		CStressClient* GetStressClient() { return m_pClient; }
		const Zion::String& GetName() { return m_strName; }

		virtual const DDLReflect::STRUCT_INFO* GetConfigType() { return NULL; }
		virtual const void* GetConfigData() { return NULL; }
		virtual bool SetConfigData(const void* pConfig, _U32 size) { (void)pConfig; (void)size; return false; }

		virtual const DDLReflect::STRUCT_INFO* GetStatusType() { return NULL; }
		virtual const _U8* GetStatusData() { return NULL; }
		const Zion::String GetStatusInfo();

		virtual void OnAttach() {}
		virtual void OnTick()	{}
		virtual void OnDetach() {}

	protected:
		virtual CStressCase* Create() = 0;

		void Attach(CStressClient* pClient);
		void Detach(CStressClient* pClient);

	private:
		Zion::String m_strName;
		CStressClient* m_pClient;
	};

	template <typename T, typename S>
	class TStressCase : public CStressCase
	{
	public:
		TStressCase(const char* Name) : CStressCase(Name)
		{
			memset(&m_Config, 0, sizeof(m_Config));
			memset(&m_Status, 0, sizeof(m_Status));
		}
		virtual ~TStressCase()
		{
		}

		virtual const DDLReflect::STRUCT_INFO* GetConfigType()
		{
			return DDLReflect::GetStruct<T>();
		}

		virtual const void* GetConfigData()
		{
			return &m_Config;
		}

		virtual bool SetConfigData(const void* pConfig, _U32 size)
		{
			if(size!=sizeof(T)) return false;
			memcpy(&m_Config, pConfig, sizeof(T));
			return true;
		}

		const T& GetConfig()
		{
			return m_Config;
		}

		void SetConfig(const T& val)
		{
			m_Config = val;
		}

		virtual const DDLReflect::STRUCT_INFO* GetStatusType()
		{
			return DDLReflect::GetStruct<S>();
		}

		virtual const _U8* GetStatusData()
		{
			return (const _U8*)&m_Status;
		}

		const S& GetStatus()
		{
			return m_Status;
		}

	protected:
		T m_Config;
		S m_Status;
	};

}

#endif 
