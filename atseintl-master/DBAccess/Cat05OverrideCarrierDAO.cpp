
//-----------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/Cat05OverrideCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/Cat05OverrideCarrier.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCat05OverrideCarrier.h"

namespace tse
{
log4cxx::LoggerPtr
Cat05OverrideCarrierDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.Cat05OverrideCarrierDAO"));

Cat05OverrideCarrierDAO&
Cat05OverrideCarrierDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}
const std::vector<Cat05OverrideCarrier*>&
getInfiniCat05OverrideData(const PseudoCityCode& pcc, DeleteList& deleteList)
{
  Cat05OverrideCarrierDAO& dao(Cat05OverrideCarrierDAO::instance());
  const std::vector<Cat05OverrideCarrier*>& ret(dao.get(deleteList, pcc));

  return ret;
}

const std::vector<Cat05OverrideCarrier*>&
Cat05OverrideCarrierDAO::get(DeleteList& del, const PseudoCityCode& pseudo)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  Cat05OverrideKey key(pseudo);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  return *ptr;
}
std::vector<Cat05OverrideCarrier*>*
Cat05OverrideCarrierDAO::create(Cat05OverrideKey key)
{
  std::vector<Cat05OverrideCarrier*>* ret(new std::vector<Cat05OverrideCarrier*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetCat05OverrideCarrier ovcc(dbAdapter->getAdapter());
    ovcc.findCat05OverrideCarriers(*ret, key._a);
    // std::cerr << "!QueryGetCat05OverrideCxr:" << key._a<<" " << ret->size() << " !!!!" <<
    // std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in Cat05OverrideCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}
bool
Cat05OverrideCarrierDAO::translateKey(const ObjectKey& objKey, Cat05OverrideKey& key) const
{
  return key.initialized = objKey.getValue("PSEUDOCITY", key._a);
}
void
Cat05OverrideCarrierDAO::translateKey(const Cat05OverrideKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("PSEUDOCITY", key._a);
  return;
}

bool
Cat05OverrideCarrierDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<Cat05OverrideCarrier, Cat05OverrideCarrierDAO>(flatKey, objectKey)
      .success();
}

Cat05OverrideKey
Cat05OverrideCarrierDAO::createKey(const Cat05OverrideCarrier* info)
{
  return Cat05OverrideKey(info->pseudoCity());
}
void
Cat05OverrideCarrierDAO::destroy(Cat05OverrideKey, std::vector<Cat05OverrideCarrier*>* recs)
{
  destroyContainer(recs);
}
void
Cat05OverrideCarrierDAO::load()
{

  StartupLoaderNoDB<Cat05OverrideCarrier, Cat05OverrideCarrierDAO>();
}

size_t
Cat05OverrideCarrierDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "Cat05OverrideCarrier cache cleared");
  return result;
}
std::string
Cat05OverrideCarrierDAO::_name("Cat05OverrideCarrier");
std::string
Cat05OverrideCarrierDAO::_cacheClass("Fares");
DAOHelper<Cat05OverrideCarrierDAO>
Cat05OverrideCarrierDAO::_helper(_name);
Cat05OverrideCarrierDAO* Cat05OverrideCarrierDAO::_instance = nullptr;

} // tse
