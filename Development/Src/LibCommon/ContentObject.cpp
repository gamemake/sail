#include <ZionBase.h>
#include "CommonDDL.h"
#include "ContentObject.h"

#include <map>
#include <string>
#include <fstream>
#include "des64.h"

namespace Zion
{

	const char* GetHomeDirectory();

	namespace ContentObject
	{

		class CContentObject
		{
		public:
			CContentObject(const DDLReflect::STRUCT_INFO* info, const A_UUID& uuid)
			{
				m_pInfo = info;
				m_pData = (A_CONTENT_OBJECT*)DDLReflect::CreateObject(info);
				m_pData->_uuid = uuid;
			}
		
			~CContentObject()
			{
				DDLReflect::DestoryObject(m_pInfo, m_pData);
				m_pData = NULL;
			}

			const DDLReflect::STRUCT_INFO* m_pInfo;
			A_CONTENT_OBJECT* m_pData;
		};

		// content object type
		struct STRUCT_INTERNAL_INFO
		{
			_U16							type_id;
			const DDLReflect::STRUCT_INFO*	info;
			bool							bExactMatch;
			Array<String>					keys;
			Map<String, A_CONTENT_OBJECT*>	key_map;
			bool							dirty;
		};

		// content object
		class CContentObjectManager
		{
		public:
			CContentObjectManager()
			{
			}
			~CContentObjectManager()
			{
				Clear();
			}
			void Clear()
			{
				Map<A_UUID, CContentObject*>::iterator i;
				for(i=m_object_map.begin(); i!=m_object_map.end(); i++)
				{
					ZION_DELETE i->second;
				}
				m_object_map.clear();
			}
			Map<A_UUID, CContentObject*> m_object_map;
		};

		const _U16 g_typeid_base = 0x1000;
		static Map<String, _U16>			g_typemap;
		static Array<STRUCT_INTERNAL_INFO>	g_typearray;
		static CContentObjectManager		g_objct_manager;
		static bool LoadContentFromJsonFile(const char* filename, bool ignore);

		void Register(const DDLReflect::STRUCT_INFO* info, bool bExactMatch, const char* keys)
		{
			if(!DDLReflect::IsParent(info, DDLReflect::GetStruct<A_CONTENT_OBJECT>()))
			{
				ZION_ASSERT(0);
				return;
			}

			Array<String> vkeys;
			if(keys)
			{
				StringSplit(keys, ',', vkeys);
				if(vkeys.size()>4 || vkeys.empty())
				{
					ZION_ASSERT(0);
					return;
				}
				for(size_t i=0; i<vkeys.size(); i++)
				{
					if(vkeys[i]=="_uuid")
					{
						if(vkeys.size()!=1)
						{
							ZION_ASSERT(0);
							return;
						}
						else
						{
							vkeys.clear();
							break;
						}
					}

					if(DDLReflect::GetStructFieldOffset(info, vkeys[i].c_str())==(_U32)-1)
					{
						ZION_ASSERT(0);
						return;
					}
				}
			}

			ZION_ASSERT(g_typemap.find(info->name)==g_typemap.end());
			if(g_typemap.find(info->name)!=g_typemap.end()) return;

			STRUCT_INTERNAL_INFO internal_info;
			internal_info.type_id = g_typeid_base + (_U16)g_typearray.size();
			internal_info.info = info;
			internal_info.bExactMatch = bExactMatch;
			internal_info.keys = vkeys;
			internal_info.dirty = true;

			g_typemap[info->name] = internal_info.type_id;
			g_typearray.push_back(internal_info);
		}

		void GetTypeList(Array<const DDLReflect::STRUCT_INFO*>& list)
		{
			list.resize(g_typearray.size());
			for(size_t i=0; i<g_typearray.size(); i++)
			{
				list[i] = g_typearray[i].info;
			}
		}

		_U16 GetTypeId(const char* name)
		{
			Map<String, _U16>::const_iterator i;
			i = g_typemap.find(name);
			if(i==g_typemap.end()) return (_U16)-1;
			return i->second;
		}

		const DDLReflect::STRUCT_INFO* GetType(const char* name)
		{
			Map<String, _U16>::const_iterator i;
			i = g_typemap.find(name);
			if(i==g_typemap.end()) return NULL;
			return g_typearray[i->second-g_typeid_base].info;
		}

		const DDLReflect::STRUCT_INFO* GetType(_U16 id)
		{
			if(id<g_typeid_base) return NULL;
			if(id>=g_typeid_base+(_U16)g_typearray.size()) return NULL;
			return g_typearray[id-g_typeid_base].info;
		}

		bool GetTypePrimaryKey(const char* name, Array<String>& keys)
		{
			_U16 id = GetTypeId(name);
			if(id==(_U16)-1) return false;
			keys = g_typearray[id-g_typeid_base].keys;
			return true;
		}

		bool GetTypePrimaryKey(const char* name, Set<String>& keys)
		{
			_U16 id = GetTypeId(name);
			if(id==(_U16)-1) return false;
			Array<String>& _keys = g_typearray[id-g_typeid_base].keys;
			for(Array<String>::const_iterator i=_keys.begin(); i!=_keys.end(); i++)
			{
				keys.insert(*i);
			}
			return true;
		}

		A_CONTENT_OBJECT* CreateObject(const DDLReflect::STRUCT_INFO* info)
		{
			A_UUID uuid;
			AUuidGenerate(uuid);
			return CreateObject(info, uuid);
		}

		A_CONTENT_OBJECT* CreateObject(const DDLReflect::STRUCT_INFO* info, const A_UUID& uuid)
		{
			if(!info) return NULL;
			Map<String, _U16>::const_iterator i = g_typemap.find(info->name);
			if(i==g_typemap.end()) return NULL;
			_U16 id = i->second;

			CContentObject* object = ZION_NEW CContentObject(info, uuid);
			g_objct_manager.m_object_map[uuid] = object;
			g_typearray[id-g_typeid_base].dirty = true;
			return object->m_pData;
		}

		void DeleteObject(const A_UUID& uuid)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.find(uuid);
			if(i==g_objct_manager.m_object_map.end()) return;
			CContentObject* pObject = i->second;

			ZION_ASSERT(g_typemap.find(pObject->m_pInfo->name)!=g_typemap.end());
			_U16 id = g_typemap[pObject->m_pInfo->name];
			g_typearray[id-g_typeid_base].dirty = true;

			ZION_DELETE pObject;
			g_objct_manager.m_object_map.erase(i);
		}

		const DDLReflect::STRUCT_INFO* GetObjectType(const A_UUID& uuid)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.find(uuid);
			if(i==g_objct_manager.m_object_map.end()) return NULL;
			return i->second->m_pInfo;
		}

		A_CONTENT_OBJECT* Modify(const A_UUID& uuid, const DDLReflect::STRUCT_INFO* info)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.find(uuid);
			if(i==g_objct_manager.m_object_map.end()) return NULL;
			if(info!=NULL && i->second->m_pInfo!=info) return NULL;
			ZION_ASSERT(g_typemap.find(info->name)!=g_typemap.end());
			_U16 id = g_typemap[info->name];
			g_typearray[id-g_typeid_base].dirty = true;
			return i->second->m_pData;
		}

		const A_CONTENT_OBJECT* QueryByUUID(const A_UUID& uuid, const DDLReflect::STRUCT_INFO* info)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.find(uuid);
			if(i==g_objct_manager.m_object_map.end()) return NULL;
			if(info!=NULL && i->second->m_pInfo!=info) return NULL;
			return i->second->m_pData;
		}

		const A_CONTENT_OBJECT* QueryByName(const char* name, const DDLReflect::STRUCT_INFO* info)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			for(i=g_objct_manager.m_object_map.begin(); i!=g_objct_manager.m_object_map.end(); i++)
			{
				if((info!=NULL || i->second->m_pInfo==info) && strcmp(name, i->second->m_pData->_name._Value)==0)
				{
					return i->second->m_pData;					
				}
			}
			return NULL;
		}

		const A_CONTENT_OBJECT* QueryByKey(const DDLReflect::STRUCT_INFO* info, const char* v1, const char* v2, const char* v3, const char* v4)
		{
			ZION_ASSERT(info && v1);
			if(!info || !v1) return NULL;

			_U16 type_id = GetTypeId(info->name);
			if(type_id==(_U16)-1) return NULL;

			STRUCT_INTERNAL_INFO& internal_info = g_typearray[type_id-g_typeid_base];

			if(internal_info.keys.empty())
			{
				ZION_ASSERT(v2==NULL && v3==NULL && v4==NULL);
				if(v2!=NULL || v3!=NULL || v4!=NULL) return NULL;
				A_UUID uuid;
				
				if(!AUuidFromString(v1, uuid)) return NULL;
				return QueryByUUID(uuid, info);
			}

			String keys_value;
			size_t keys_count = 1;
			keys_value = v1;
			if(v2)
			{
				keys_value.append(".$$.");
				keys_value.append(v2);
				keys_count = 2;
				if(v3)
				{
					keys_value.append(".$$.");
					keys_value.append(v3);
					keys_count = 3;
					if(v4)
					{
						keys_value.append(".$$.");
						keys_value.append(v4);
						keys_count = 4;
					}
				}
			}

			ZION_ASSERT(internal_info.keys.size()==keys_count);
			if(internal_info.keys.size()!=keys_count) return NULL;

			Map<String, A_CONTENT_OBJECT*>::iterator i;
			i = internal_info.key_map.find(keys_value);
			if(i==internal_info.key_map.end()) return NULL;
			return i->second;
		}

		const A_CONTENT_OBJECT* QueryByUniqueId(const DDLReflect::STRUCT_INFO* info, const char* value1)
		{
			ZION_ASSERT(info);
			if(!info) return NULL;

			_U16 type_id = GetTypeId(info->name);
			if(type_id==(_U16)-1) return NULL;

			STRUCT_INTERNAL_INFO& internal_info = g_typearray[type_id-g_typeid_base];
			Map<String, A_CONTENT_OBJECT*>::iterator i;
			i = internal_info.key_map.find(value1);
			if(i==internal_info.key_map.end()) return NULL;
			return i->second;
		}

		bool GenContentObjectUniqueId(_U16 id, const A_CONTENT_OBJECT* obj, String& uid)
		{
			if(id<g_typeid_base) return false;
			if(id>=g_typeid_base+(_U16)g_typearray.size()) return false;
			STRUCT_INTERNAL_INFO& info = g_typearray[id-g_typeid_base];

			uid.clear();
			for(size_t f=0; f<info.keys.size(); f++)
			{
				String value;
				if(!DDLReflect::StructParamToString(info.info, info.keys[f].c_str(), obj, value))
				{
					return false;
				}
				if(f>0) uid.append(".$$.");
				uid.append(value);
			}

			return true;
		}

		static String g_buildindex_errmsg;

		bool BuildIndex(const DDLReflect::STRUCT_INFO* info)
		{
			if(info==NULL)
			{
				for(size_t i=0; i<g_typearray.size(); i++)
				{
					if(g_typearray[i].keys.size()==0) continue;
					if(!BuildIndex(g_typearray[i].info)) return false;
				}
				return true;
			}

			_U16 type_id = GetTypeId(info->name);
			if(type_id==(_U16)-1)
			{
				g_buildindex_errmsg = StringFormat("invalid struct name : %s", info->name);
				return false;
			}

			STRUCT_INTERNAL_INFO& internal_info = g_typearray[type_id-g_typeid_base];
			internal_info.key_map.clear();
			if(internal_info.keys.size()==0) return true;

			Map<A_UUID, CContentObject*>::iterator i;
			for(i=g_objct_manager.m_object_map.begin(); i!=g_objct_manager.m_object_map.end(); i++)
			{
				if(info!=i->second->m_pInfo && (internal_info.bExactMatch || !IsParent(i->second->m_pInfo, info))) continue;

				String keys_value;
				if(!GenContentObjectUniqueId(type_id, i->second->m_pData, keys_value))
				{
					g_buildindex_errmsg = StringFormat("error in GenContentObjectUniqueId");
					return false;
				}

				if(internal_info.key_map.find(keys_value)!=internal_info.key_map.end())
				{
					char o1[60], o2[60];
					AUuidToString(internal_info.key_map[keys_value]->_uuid, o1);
					AUuidToString(i->first, o2);
					g_buildindex_errmsg = StringFormat("reduplicate %s vs %s", o1, o2);
					return false;
				}
				internal_info.key_map[keys_value] = i->second->m_pData;
			}

			return true;
		}

		const String& BuildIndexGetErrorMsg()
		{
			return g_buildindex_errmsg;
		}

		bool GetList(const DDLReflect::STRUCT_INFO* info, Array<A_UUID>& list, bool bExactMatch)
		{
			list.clear();

			Map<A_UUID, CContentObject*>::iterator i;
			for(i=g_objct_manager.m_object_map.begin(); i!=g_objct_manager.m_object_map.end(); i++)
			{
				if(i->second->m_pInfo==info || (!bExactMatch && IsParent(i->second->m_pInfo, info)))
				{
					list.push_back(i->first);
				}
			}

			return true;
		}

		const A_CONTENT_OBJECT* FindFirst(const DDLReflect::STRUCT_INFO* info, bool bExactMatch)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.begin();
			while(i!=g_objct_manager.m_object_map.end())
			{
				if(i->second->m_pInfo==info || (!bExactMatch && IsParent(i->second->m_pInfo, info))) return i->second->m_pData;
				i++;
			}
			return NULL;
		};

		const A_CONTENT_OBJECT* FindNext(const DDLReflect::STRUCT_INFO* info, bool bExactMatch, const A_CONTENT_OBJECT* object)
		{
			Map<A_UUID, CContentObject*>::iterator i;
			i = g_objct_manager.m_object_map.find(object->_uuid);
			if(i==g_objct_manager.m_object_map.end()) return NULL;
			i++;
			while(i!=g_objct_manager.m_object_map.end())
			{
				if(i->second->m_pInfo==info || (!bExactMatch && IsParent(i->second->m_pInfo, info))) return i->second->m_pData;
				i++;
			}
			return NULL;
		}

		bool LoadContent(const char* path, bool ignore)
		{
			for(size_t i=0; i<g_typearray.size(); i++)
			{
				String file = StringFormat("%s%s%s.json", path?path:GetHomeDirectory(), path?"":"Content/Json/", g_typearray[i].info->name);
				if(LoadContentFromJsonFile(file.c_str(), ignore))
				{
					g_typearray[i].dirty = false;
				}
				else
				{
					if(!ignore) return false;
				}
			}
			return true;
		}

		static bool CreateContentObject(Json::Value elem, bool ignore)
		{
			if(!elem.isObject())
			{
				return false;
			}
			Json::Value type = elem.get("type", Json::Value());
			Json::Value data = elem.get("data", Json::Value());
			if(!type.isString() || !data.isObject())
			{
				return false;
			}
			Json::Value vuuid = data.get("_uuid", Json::Value());
			if(!vuuid.isString())
			{
				return false;
			}
			A_UUID uuid;
			if(!AUuidFromString(vuuid.asString().c_str(), uuid))
			{
				return false;
			}
			const DDLReflect::STRUCT_INFO* info = GetType(type.asString().c_str());
			if(!info)
			{
				return false;
			}
			A_CONTENT_OBJECT* object = CreateObject(info, uuid);
			if(!object)
			{
				return false;
			}
			if(!DDLReflect::Json2Struct(info, data, (_U8*)object, ignore))
			{
				return false;
			}
			return true;
		}

		bool LoadContentFromJsonFile(const char* filename, bool ignore)
		{
			Json::Value root;
			std::ifstream f(filename, std::ifstream::binary);
			if(!f.is_open()) return false;
			Json::Reader reader;
			if(!reader.parse(f, root, false)) return false;
			if(!root.isArray()) return false;

			for(Json::Value::UInt index=0; index<root.size(); index++)
			{
				if(!CreateContentObject(root.get(index, Json::Value()), ignore)) return false;
			}
			return true;
		}

		bool LoadContentFromBinaryFile(const char* filename, const char* deskey)
		{
			DES_KEY key;
			DES_KEY_S keys;
			if(!DES_KeyFromString(key, deskey)) return false;
			DES_SetKey(key, keys);

			FILE* fp = fopen(filename, "rb");
			String line;
			_U8 rawdata[300*1024];

			if(fread(rawdata, 1, 4, fp)!=4 || memcmp(rawdata, "DBNN", 4)!=0)
			{
				fclose(fp);
				return false;
			}

			_U32 object_count;
			if(fread(&object_count, 1, sizeof(object_count), fp)!=4)
			{
				fclose(fp);
				return false;
			}

			for(;;)
			{
				if((object_count--)==0)
				{
					fclose(fp);
					return true;
				}

				_U16 tid;
				_U32 size, i, count;

				if(fread(&tid, 1, sizeof(tid), fp)!=sizeof(tid)) break;
				if(fread(&size, 1, sizeof(size), fp)!=sizeof(size)) break;
				if(size>sizeof(rawdata)) break;

				const DDLReflect::STRUCT_INFO* info = GetType(tid);
				if(!info) break;

				count = (size==0?0:(size-1)/7+1);
				for(i=0; i<count; i++)
				{
					char buf[8], sbuf[8];
					if(fread(buf, 1, sizeof(buf), fp)!=sizeof(buf)) break;
					DES_Decrypt(keys, sbuf, buf);
					for(size_t x=0; x<7; x++)
					{
						rawdata[i*7+x] = (_U8)(sbuf[x]&0x7f);
						if(sbuf[7]&(1<<x)) rawdata[i*7+x] |= 0x80;
					}
					/*
					char buf[8];
					if(fread(buf, 1, sizeof(buf), fp)!=sizeof(buf)) break;
					memcpy(&rawdata[i*7], buf, 7);
					*/
				}
				if(i!=count) break;

				A_CONTENT_OBJECT* obj = CreateObject(info, *((A_UUID*)rawdata));
				if(!obj) break;

				DDL::MemoryReader reader(rawdata, size);
				if(!info->read_proc(reader, obj)) break;
			}

			fclose(fp);
			return false;
		}

		bool SaveContent(const char* path, bool force)
		{
			String realpath;
			if(path)
			{
				realpath = path;
			}
			else
			{
				realpath = StringFormat("%sContent/Json/", GetHomeDirectory());
			}

			for(size_t i=0; i<g_typearray.size(); i++)
			{
				if(!force && !g_typearray[i].dirty) continue;

				char filepath[1000];
				sprintf(filepath, "%s%s.json", realpath.c_str(), g_typearray[i].info->name);
				std::ofstream f;
				f.open(filepath, std::ifstream::binary);
				if(!f.is_open()) return false;
				f << "[";

				Map<A_UUID, CContentObject*>::const_iterator it;
				for(it=g_objct_manager.m_object_map.begin(); it!=g_objct_manager.m_object_map.end(); it++)
				{
					if(g_typearray[i].info!=it->second->m_pInfo) continue;

					if(it==g_objct_manager.m_object_map.begin())
					{
						f << std::endl;
					}
					else
					{
						f << "," << std::endl;
					}

					String va;
					if(!DDLReflect::Struct2Json(it->second->m_pInfo, (const _U8*)(it->second->m_pData), va))
					{
						ZION_ASSERT(0);
					}

					f << "\t" << va;
				}

				f << std::endl << "]" << std::endl;
				f.close();
				g_typearray[i].dirty = false;
			}

			return true;
		}

		bool SaveContentToBinaryFile(const char* file, const char* deskey)
		{
			DES_KEY key;
			DES_KEY_S keys;
			if(!DES_KeyFromString(key, deskey)) return false;
			DES_SetKey(key, keys);

			FILE* fp = fopen(file, "wb");
			if(!fp) return false;

			fwrite("DBNN", 1, 4, fp);
			_U32 object_count = (_U32)g_objct_manager.m_object_map.size();
			fwrite(&object_count, 1, sizeof(object_count), fp);

			Map<A_UUID, CContentObject*>::iterator i;
			for(i=g_objct_manager.m_object_map.begin(); i!=g_objct_manager.m_object_map.end(); i++)
			{
				_U8 rawdata[300*1024];
				DDL::MemoryWriter writer(rawdata, sizeof(rawdata));
				if(!i->second->m_pInfo->write_proc(writer, i->second->m_pData))
				{
					fclose(fp);
					return false;
				}

				_U8* src = rawdata;
				char buf[8], sbuf[8];
				_U32 size = writer.GetSize();
				_U16 tid = ContentObject::GetTypeId(i->second->m_pInfo->name);
				fwrite(&tid, 1, sizeof(tid), fp);
				fwrite(&size, 1, sizeof(size), fp);
				while(size>7)
				{
					sbuf[7] = 0;
					for(size_t i=0; i<7; i++)
					{
						sbuf[i] = (char)(src[i]&0x7f);
						if(src[i]&0x80) sbuf[7] |= 1 << i;
					}
					DES_Encrypt(keys, buf, sbuf);
					/*
					memcpy(buf, src, 7);
					buf[7] = 0;
					*/
					fwrite(buf, 1, 8, fp);
					src += 7;
					size -= 7;
				}
				if(size>0)
				{
					memset(sbuf, 0, sizeof(sbuf));
					for(size_t i=0; i<size; i++)
					{
						sbuf[i] = (char)(src[i]&0x7f);
						if(src[i]&0x80) sbuf[7] |= 1 << i;
					}
					DES_Encrypt(keys, buf, sbuf);
					/*
					memset(buf, 0, sizeof(buf));
					memcpy(buf, src, size);
					*/
					fwrite(buf, 1, 8, fp);
				}
			}

			fclose(fp);
			return true;
		}

		void ClearContents()
		{
			g_objct_manager.Clear();
			BuildIndex();
			for(size_t i=0; i<g_typearray.size(); i++)
			{
				g_typearray[i].dirty = true;
			}
		}

		bool IsContentDirty()
		{
			for(size_t i=0; i<g_typearray.size(); i++)
			{
				if(g_typearray[i].dirty) return true;
			}
			return false;
		}

	}
}
