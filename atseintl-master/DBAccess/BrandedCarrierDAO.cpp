//------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/BrandedCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BrandedCarrier.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAllBrandedCarriers.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
BrandedCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BrandedCarrierDAO"));

BrandedCarrierDAO&
BrandedCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct IsNotBFVCode
{
  IsNotBFVCode(const BFVersion& code) : _code(code) {}

  bool operator()(const BrandedCarrier* info) const { return info->bfvCode() != _code; }

  const BFVersion _code;
};

const std::vector<BrandedCarrier*>&
getBrandedCarriersData(DeleteList& deleteList)
{
  BrandedCarrierDAO& dao = BrandedCarrierDAO::instance();
  return dao.get(deleteList, "BFO");
}

const std::vector<BrandedCarrier*>&
getS8BrandedCarriersData(DeleteList& deleteList)
{
  BrandedCarrierDAO& dao = BrandedCarrierDAO::instance();
  return dao.get(deleteList, "BFN");
}

const std::vector<BrandedCarrier*>&
BrandedCarrierDAO::get(DeleteList& del, const BFVersion& code)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  IsNotBFVCode filter(code);
  return *(applyFilter(del, ptr, filter));
}

std::vector<BrandedCarrier*>*
BrandedCarrierDAO::create(IntKey key)
{
  std::vector<BrandedCarrier*>* ret = new std::vector<BrandedCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllBrandedCarriers bc(dbAdapter->getAdapter());
    bc.findAllBrandedCarriers(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandedCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

IntKey
BrandedCarrierDAO::createKey(BrandedCarrier* info)
{
  return IntKey(0);
}

void
BrandedCarrierDAO::load()
{
  // Pre-load this table only if Brand Service is enabled
  std::string configVal("N");
  (Global::config()).getValue("BRAND_SERVICE", configVal, "FAREDISPLAY_SVC");

  if (configVal != "Y" && configVal != "y" && configVal != "1")
    return;

  configVal = "N";
  (Global::config()).getValue("BRANDED_CARRIER", configVal, "ABCC_SVC");

  // Pre-load this table only if Branded Carrier flag is enabled

  if (configVal != "Y" && configVal != "y" && configVal != "1")
    return;

  StartupLoader<QueryGetAllBrandedCarriers, BrandedCarrier, BrandedCarrierDAO>();
}

bool
BrandedCarrierDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  BrandedCarrier* info(new BrandedCarrier);
  BrandedCarrier::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  bool alreadyExists(false);

  for (std::vector<BrandedCarrier*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const BrandedCarrier* thisCarrier((*bit));
    if (thisCarrier->carrier() == info->carrier())
    {
      alreadyExists = true;
      break;
    }
  }

  if (alreadyExists)
  {
    delete info;
  }
  else
  {
    ptr->push_back(info);
    cache().getCacheImpl()->queueDiskPut(IntKey(0), true);
  }

  return true;
}

void
BrandedCarrierDAO::destroy(IntKey key, std::vector<BrandedCarrier*>* recs)
{
  destroyContainer(recs);
}

std::string
BrandedCarrierDAO::_name("BrandedCarrier");
std::string
BrandedCarrierDAO::_cacheClass("FareDisplay");

DAOHelper<BrandedCarrierDAO>
BrandedCarrierDAO::_helper(_name);
BrandedCarrierDAO* BrandedCarrierDAO::_instance = nullptr;

} // namespace tse
