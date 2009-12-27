/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "FactoryFactory.h"
#include "Deed.h"
#include "FactoryObject.h"
#include "ResourceContainerFactory.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"
#include <assert.h>

//=============================================================================

bool				FactoryFactory::mInsFlag    = false;
FactoryFactory*		FactoryFactory::mSingleton  = NULL;

//======================================================================================================================

FactoryFactory*	FactoryFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new FactoryFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

FactoryFactory::FactoryFactory(Database* database) : FactoryBase(database)
{

	_setupDatabindings();
}

//=============================================================================

FactoryFactory::~FactoryFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void FactoryFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case FFQuery_ResourceData:
		{

			uint64 count = result->getRowCount();

			FactoryObject* factory = dynamic_cast<FactoryObject*>(asyncContainer->mObject);

			if(count)
			{	
				/*
				DataBinding* binding = mDatabase->CreateDataBinding(2);
				binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
				binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);
				
				HResourceList*	hRList = harvester->getResourceList();
				
				HResourceList::iterator it = hRList->begin();

				HarvesterHopperItem hopperTemp;
				for(uint64 i=0;i <count;i++)
				{
					result->GetNextRow(binding,&hopperTemp);
					hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
				}

				*/
			}

			/*
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);
			asynContainer->mId		= harvester->getId();
			asynContainer->mObject	= harvester;

			mDatabase->ExecuteSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
															 " FROM structure_attributes sa"
															 " INNER JOIN attributes ON (sa.attribute_id = attributes.id)"
															 " WHERE sa.structure_id = %"PRIu64" ORDER BY sa.order",harvester->getId());
			  */
			
		}
		break;

		case FFQuery_AttributeData:
		{
			/*
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);
			//_buildAttributeMap(harvester,result);

			Attribute_QueryContainer	attribute;
			uint64						count = result->getRowCount();
			//int8						str[256];
			//BStringVector				dataElements;

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(mAttributeBinding,(void*)&attribute);				
				harvester->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
			}

			harvester->setLoadState(LoadState_Loaded);

			gLogger->logMsgF("HarvesterFactory: loaded Harvester %I64u", MSG_HIGH, harvester->getId());
			asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);
			
			*/
		}
		break;

		case FFQuery_MainData:
		{
			/*
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_ResourceData,asyncContainer->mClient,asyncContainer->mId);

			HarvesterObject* harvester = new(HarvesterObject);
			_createHarvester(result,harvester);

			asynContainer->mObject = harvester;
			asynContainer->mClient = asyncContainer->mClient;

			//gLogger->logMsgF("HarvesterFactory: loaded Harvester %I64u", MSG_HIGH, harvester->getId());
			//asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);

			//now request the associated resource container count

			int8 sql[250];
			sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());
			mDatabase->ExecuteSqlAsync(this,asynContainer,sql);

			 */
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void FactoryFactory::_createFactory(DatabaseResult* result, FactoryObject* factory)
{

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mFactoryBinding,factory);

	factory->setLoadState(LoadState_Loaded);
	factory->setType(ObjType_Structure);
	factory->mCustomName.convert(BSTRType_Unicode16);
}

//=============================================================================

void FactoryFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	//request the harvesters Data first
	
	int8 hmm[1024];
	sprintf(hmm, "SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,std.type,std.object_string,std.stf_name, std.stf_file, s.name, std.lots_used, std.resource_Category, h.ResourceID, h.active, std.maint_cost_wk, std.power_used, s.condition, std.max_condition FROM structures s INNER JOIN structure_type_data std ON (s.type = std.type) INNER JOIN harvesters h ON (s.id = h.id) WHERE (s.id = %"PRIu64")",id);
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_MainData,client,id);

	mDatabase->ExecuteSqlAsync(this,asynContainer,hmm);
}


//=============================================================================

void FactoryFactory::_setupDatabindings()
{
	mFactoryBinding = mDatabase->CreateDataBinding(22);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mId),8,0);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mOwner),8,1);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mX),4,2);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mY),4,3);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mZ),4,4);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mW),4,5);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mX),4,6);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mY),4,7);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mZ),4,8);
/*
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mFactoryFamily),4,9);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mModel),256,10);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mName),256,11);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mNameFile),256,12);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mCustomName),256,13);
	mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mLotsUsed),1,14);
//	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mResourceCategory),4,15);

	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mCurrentResource),8,15);
	mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mActive),1,16);
	//mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mCurrentExtractionRate),4,18);
	
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,maint_cost_wk),4,17);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mPowerUsed),4,18);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mDamage),4,19);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mMaxCondition),4,20);
	  */
}

//=============================================================================

void FactoryFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mFactoryBinding);

}

//=============================================================================

void FactoryFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	//*ONLY* used to load resource containers
	InLoadingContainer* ilc = _getObject(object->getParentId());
	FactoryObject*		factory = dynamic_cast<FactoryObject*>(ilc->mObject);

	//add res containers
	gWorldManager->addObject(object,true);


	if(factory->decLoadCount() == 0)
	{
		if(!(_removeFromObjectLoadMap(factory->getId())))
			gLogger->logMsg("FactoryFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(factory,ilc->mClient);

		mILCPool.free(ilc);
	}

}

//=============================================================================

void FactoryFactory::releaseAllPoolsMemory()
{
	releaseQueryContainerPoolMemory();
	releaseILCPoolMemory();

}