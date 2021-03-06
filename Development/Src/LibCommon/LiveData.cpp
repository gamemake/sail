#include <ZionBase.h>
#include "CommonDDL.h"
#include "LiveData.h"

namespace Zion
{
	namespace LiveData
	{

		CMonitor::CMonitor(CManager* pManager, const DDLReflect::STRUCT_INFO* info)
		{
			m_pManager = pManager;
			m_pInfo = info;
			m_bDirty = false;
		}

		bool CMonitor::IsDirty()
		{
			return m_bDirty;
		}

		void CMonitor::SetDirty()
		{
			m_bDirty = true;
			m_pManager->SetLiveDataChanged();
		}

		void CMonitor::SetDirty(const char* name, _U32 offset, _U32 size)
		{
			(void)name;
			(void)offset;
			(void)size;
			m_bDirty = true;
			m_pManager->SetLiveDataChanged();
		}

		void CMonitor::Clean()
		{
			m_bDirty = false;
		}

		CObject::CObject(CManager* pManager, const DDLReflect::STRUCT_INFO* pInfo, A_LIVE_OBJECT* pData) : m_Monitor(pManager, pInfo), DDLDataObject::CObject(&m_Monitor, pInfo, pData, "", 0)
		{
		}

		bool CObject::IsDirty()
		{
			return m_Monitor.IsDirty();
		}

		void CObject::Clean()
		{
			return m_Monitor.Clean();
		}

		CManager::CManager()
		{
			m_LastNum = 0;
			m_bEnableMonitor = true;
			m_bIsLiveDataChanged = false;
		}

		void CManager::InitRandom(_U32 seed)
		{
			m_LastNum = seed;
		}

		_F32 CManager::Rand()
		{
			String val = CHashMD5::CalcToString(&m_LastNum, sizeof(m_LastNum));
			_U32 m_LastNum = CalcCRC32(val.c_str(), (_U32)val.size());
			return ((_F32)(m_LastNum & 0xffffff)) / 0xffffff;
		}

		void CManager::EnableMonitor(bool bEnable)
		{
			m_bEnableMonitor = bEnable;
		}

		bool CManager::IsEnableMointor()
		{
			return m_bEnableMonitor;
		}

		CObject* CManager::Append(const DDLReflect::STRUCT_INFO* pInfo)
		{
			A_UUID _uuid;
			AUuidGenerate(_uuid);
			if(m_ObjMap.find(_uuid)==m_ObjMap.end())
			{
				ZION_ASSERT(!"live object generate duplicate uuid");
				return NULL;
			}

			A_LIVE_OBJECT* data = (A_LIVE_OBJECT*)DDLReflect::CreateObject(pInfo);
			data->_uuid = _uuid;
			CObject* obj = ZION_NEW CObject(this, pInfo, data);
			m_ObjMap[data->_uuid] = obj;
			return NULL;
		}

		CObject* CManager::Append(const DDLReflect::STRUCT_INFO* pInfo, A_LIVE_OBJECT* data)
		{
			if(m_ObjMap.find(data->_uuid)!=m_ObjMap.end())
			{
				ZION_ASSERT(!"live object alread existed");
				return NULL;
			}

			CObject* obj = ZION_NEW CObject(this, pInfo, data);
			obj->Clean();
			m_ObjMap[data->_uuid] = obj;
			return obj;
		}

		CObject* CManager::Append(const DDLReflect::STRUCT_INFO* pInfo, const _U8* buf, _U32 len)
		{
			A_LIVE_OBJECT* data = (A_LIVE_OBJECT*)DDLReflect::CreateObject(pInfo);
			DDL::MemoryReader reader(buf, len);
			if(!pInfo->read_proc(reader, data))
			{
				ZION_ASSERT(!"live object serialize binary error");
				DDLReflect::DestoryObject(pInfo, data);
				return NULL;
			}

			if(m_ObjMap.find(data->_uuid)==m_ObjMap.end())
			{
				ZION_ASSERT(!"live object alread existed");
				DDLReflect::DestoryObject(pInfo, data);
				return NULL;
			}

			CObject* obj = ZION_NEW CObject(this, pInfo, data);
			obj->Clean();
			m_ObjMap[data->_uuid] = obj;
			return obj;
		}

		CObject* CManager::Append(const DDLReflect::STRUCT_INFO* pInfo, const char* str)
		{
			A_LIVE_OBJECT* data = (A_LIVE_OBJECT*)DDLReflect::CreateObject(pInfo);
			String val(str);
			if(!DDLReflect::Json2Struct(pInfo, Zion::String(val), (_U8*)data))
			{
				ZION_ASSERT(!"live object serialize json error");
				DDLReflect::DestoryObject(pInfo, data);
				return NULL;
			}

			if(m_ObjMap.find(data->_uuid)!=m_ObjMap.end())
			{
				ZION_ASSERT(!"live object alread existed");
				DDLReflect::DestoryObject(pInfo, data);
				return NULL;
			}

			CObject* obj = ZION_NEW CObject(this, pInfo, data);
			obj->Clean();
			m_ObjMap[data->_uuid] = obj;
			return obj;
		}

		bool CManager::Remove(const A_UUID& _uuid)
		{
			Map<A_UUID, CObject*>::iterator o = m_ObjMap.find(_uuid);
			if(o==m_ObjMap.end())
			{
				ZION_ASSERT(!"object not found");
				return false;
			}
			else
			{
				m_ObjMap.erase(o);
				return true;
			}
		}

		void CManager::Clear()
		{
			Map<A_UUID, CObject*>::iterator i;
			for(i=m_ObjMap.begin(); i!=m_ObjMap.end(); i++)
			{
				ZION_DELETE i->second;
			}
			m_ObjMap.clear();
			m_LastNum = 0;
			m_bEnableMonitor = true;
			m_bIsLiveDataChanged = false;
		}

		CObject* CManager::Get(const A_UUID& _uuid)
		{
			Map<A_UUID, CObject*>::iterator o = m_ObjMap.find(_uuid);
			if(o==m_ObjMap.end()) return NULL;
			return o->second;
		}

		CObject* CManager::FindFirst()
		{
			return m_ObjMap.begin()==m_ObjMap.end()?NULL:m_ObjMap.begin()->second;
		}

		CObject* CManager::FindNext(CObject* obj)
		{
			A_LIVE_OBJECT* data = (A_LIVE_OBJECT*)obj->GetData();
			Map<A_UUID, CObject*>::iterator i = m_ObjMap.find(data->_uuid);
			if(i==m_ObjMap.end())
			{
				ZION_ASSERT(!"error");
				return NULL;
			}
			 if((++i)==m_ObjMap.end()) return NULL;
			return i->second;
		}

		bool CManager::IsLiveDataChanged()
		{
			return m_bIsLiveDataChanged;
		}

		void CManager::SetLiveDataChanged()
		{
			m_bIsLiveDataChanged = true;
		}

		void CManager::ClearLiveDataChanged()
		{
			m_bIsLiveDataChanged = false;
		}

	}
}
