#include <ZionBase.h>
#include <ZionCommon.h>

#include "CardGameDDL.h"

namespace Zion
{

	const char* GetAppName()
	{
		return "CardGame";
	}

	void InitDDLStub()
	{
		Zion::Engine::InitDDLStub();

		RegisterClientStub<CGCALLBACK_BASE>();
		RegisterClientStub<CGCALLBACK_GAME>();
		RegisterServerStub<CGSERVER_BASE>();
		RegisterServerStub<CGSERVER_GAME>();
	}

	void InitContentObjects()
	{
		Zion::Engine::InitContentObjects();

		ContentObject::Register(DDLReflect::GetStruct<CG_SHOPITEM>(), true, "shopitem_id");
		ContentObject::Register(DDLReflect::GetStruct<CG_CARD_CONFIG>(), true, "_uuid");
	}

	void InitLiveObjects()
	{
		Zion::Engine::InitLiveObjects();

		Zion::LiveObject::Register(DDLReflect::GetStruct<CG_AVATAR>());
		Zion::LiveObject::Register(DDLReflect::GetStruct<CG_CARD>());
	}

};
