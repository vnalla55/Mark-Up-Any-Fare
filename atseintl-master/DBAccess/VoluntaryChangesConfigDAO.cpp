//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/VoluntaryChangesConfigDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVoluntaryChangesConfig.h"
#include "DBAccess/VoluntaryChangesConfig.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
VoluntaryChangesConfigDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryChangesConfigDAO"));

VoluntaryChangesConfigDAO&
VoluntaryChangesConfigDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const DateTime&
getVoluntaryChangesConfigData(const CarrierCode& carrierCode,
                              const DateTime& currentTktDate,
                              const DateTime& originalTktIssueDate,
                              DeleteList& deleteList)
{
  VoluntaryChangesConfigDAO& dao = VoluntaryChangesConfigDAO::instance();
  return dao.get(deleteList, carrierCode, currentTktDate, originalTktIssueDate);
}

const DateTime&
VoluntaryChangesConfigDAO::get(DeleteList& del,
                               const CarrierCode& key,
                               const DateTime& currentDate,
                               const DateTime& originalTktIssueDate)
{
  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  std::vector<VoluntaryChangesConfig*>* ret = new std::vector<VoluntaryChangesConfig*>;
  del.adopt(ret);
  std::vector<VoluntaryChangesConfig*>::iterator i;
  for (i = ptr->begin(); i != ptr->end(); i++)
  {
    const VoluntaryChangesConfig* vcc = (*i);
    if (vcc->expireDate().date() >= currentDate.date() && vcc->applDate() > originalTktIssueDate)
      return vcc->applDate();
  }

  return DateTime::emptyDate();
}

/*
struct VoluntaryChangesConfigDAO::groupByKey
{
public:
    CarrierKey prevKey;

    DAOCache& cache;

    groupByKey()
        :   cache(VoluntaryChangesConfigDAO::instance().cache()),
            ptr(0)
    {
    }

    std::vector<VoluntaryChangesConfig*>* ptr;

    void operator() (VoluntaryChangesConfig* info)
    {
        CarrierKey key(info->carrier());
        if (!(key == prevKey))
        {
            ptr = new std::vector<VoluntaryChangesConfig*>;
            cache.put(key, ptr);
            prevKey = key;
        }

        ptr->push_back(info);
     }
};

void VoluntaryChangesConfigDAO::load()
{
    std::vector<VoluntaryChangesConfig*> recs;

    DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    try
    {
        QueryGetAllVoluntaryChangesConfig tcr(dbAdapter->getAdapter());
        tcr.findAllVoluntaryChangesConfigs(recs);
    }
    catch (...)
    {
        LOG4CXX_WARN(_logger,"DB exception in VoluntaryChangesConfigDAO::load");
        deleteVectorOfPointers(recs);
        throw;
    }

    std::for_each(recs.begin(), recs.end(), groupByKey());
}
*/

std::vector<VoluntaryChangesConfig*>*
VoluntaryChangesConfigDAO::create(CarrierKey carrier)
{
  std::vector<VoluntaryChangesConfig*>* ret = new std::vector<VoluntaryChangesConfig*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVoluntaryChangesConfig tc(dbAdapter->getAdapter());
    tc.findVoluntaryChangesConfig(*ret, carrier._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryChangesConfigDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryChangesConfigDAO::destroy(CarrierKey key, std::vector<VoluntaryChangesConfig*>* recs)
{
  std::vector<VoluntaryChangesConfig*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
VoluntaryChangesConfigDAO::_name("VoluntaryChangesConfig");
std::string
VoluntaryChangesConfigDAO::_cacheClass("Rules");
DAOHelper<VoluntaryChangesConfigDAO>
VoluntaryChangesConfigDAO::_helper(_name);
VoluntaryChangesConfigDAO* VoluntaryChangesConfigDAO::_instance = nullptr;

} // namespace tse
