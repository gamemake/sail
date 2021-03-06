#ifndef __ZION_LIVE_DATA__
#define __ZION_LIVE_DATA__

namespace Zion
{
	namespace LiveData
	{

		class CMonitor;
		class CObject;
		class CManager;
		class CAccesser;

		class CMonitor : public DDLDataObject::IMonitor
		{
		public:
			CMonitor(CManager* pManager, const DDLReflect::STRUCT_INFO* info);

			bool IsDirty();
			void SetDirty();
			virtual void SetDirty(const char* name, _U32 offset, _U32 size);
			void Clean();

		private:
			CManager* m_pManager;
			const DDLReflect::STRUCT_INFO* m_pInfo;
			bool m_bDirty;
		};

		class CObject : public DDLDataObject::CObject
		{
		public:
			CObject(CManager* pManager, const DDLReflect::STRUCT_INFO* pInfo, A_LIVE_OBJECT* pData);

			bool IsDirty();
			void Clean();

		private:
			CManager* m_pManager;
			CMonitor m_Monitor;
		};

		class CManager
		{
			friend class CMonitor;
		public:
			CManager();

			void InitRandom(_U32 seed);
			_F32 Rand(); // return 0~1.0

			void EnableMonitor(bool bEnable);
			bool IsEnableMointor();

			CObject* Append(const DDLReflect::STRUCT_INFO* pInfo);
			CObject* Append(const DDLReflect::STRUCT_INFO* pInfo, A_LIVE_OBJECT* data);
			CObject* Append(const DDLReflect::STRUCT_INFO* pInfo, const _U8* data, _U32 len);
			CObject* Append(const DDLReflect::STRUCT_INFO* pInfo, const char* data);
			bool Remove(const A_UUID& _uuid);
			void Clear();

			CObject* Get(const A_UUID& _uuid);
			CObject* FindFirst();
			CObject* FindNext(CObject* obj);

			bool IsLiveDataChanged();

		protected:
			void SetLiveDataChanged();
			void ClearLiveDataChanged();

		private:
			_U32 m_LastNum;
			bool m_bEnableMonitor;
			bool m_bIsLiveDataChanged;
			Map<A_UUID, CObject*> m_ObjMap;
		};

	}
}

#endif
