//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/VoluntaryRefundsConfigDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsConfig.h"
#include "DBAccess/VoluntaryRefundsConfig.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
VoluntaryRefundsConfigDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryRefundsConfigDAO"));

VoluntaryRefundsConfigDAO&
VoluntaryRefundsConfigDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const DateTime&
getVoluntaryRefundsConfigData(const CarrierCode& carrierCode,
                              const DateTime& currentTktDate,
                              const DateTime& originalTktIssueDate,
                              DeleteList& deleteList)
{
  return VoluntaryRefundsConfigDAO::instance().get(
      deleteList, carrierCode, currentTktDate, originalTktIssueDate);
}

namespace
{
struct IsApplicable
{
  IsApplicable(const DateTime& currentDate, const DateTime& originalTktIssueDate)
    : _currentDate(currentDate), _originalTktIssueDate(originalTktIssueDate)
  {
  }

  bool operator()(const VoluntaryRefundsConfig* vrc)
  {
    return (vrc->expireDate().date() > _currentDate.date() &&
            vrc->effDate().date() <= _currentDate.date() &&
            vrc->applDate() > _originalTktIssueDate);
  }

protected:
  const DateTime& _currentDate;
  const DateTime& _originalTktIssueDate;
};
}

const DateTime&
VoluntaryRefundsConfigDAO::get(DeleteList& del,
                               const CarrierCode& key,
                               const DateTime& currentDate,
                               const DateTime& originalTktIssueDate)
{
  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);

  std::vector<VoluntaryRefundsConfig*>::iterator curr =
      std::find_if(ptr->begin(), ptr->end(), IsApplicable(currentDate, originalTktIssueDate));

  return curr != ptr->end() ? (*curr)->applDate() : DateTime::emptyDate();
}

std::vector<VoluntaryRefundsConfig*>*
VoluntaryRefundsConfigDAO::create(CarrierKey carrier)
{
  std::vector<VoluntaryRefundsConfig*>* ret = new std::vector<VoluntaryRefundsConfig*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetVoluntaryRefundsConfig tc(dbAdapter->getAdapter());
    tc.find(*ret, carrier._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryRefundsConfigDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryRefundsConfigDAO::destroy(CarrierKey key, std::vector<VoluntaryRefundsConfig*>* recs)
{
  std::vector<VoluntaryRefundsConfig*>::iterator i;
  for (i = recs->begin(); i != recs->end(); ++i)
    delete *i;
  delete recs;
}

std::string
VoluntaryRefundsConfigDAO::_name("VoluntaryRefundsConfig");
std::string
VoluntaryRefundsConfigDAO::_cacheClass("Rules");
DAOHelper<VoluntaryRefundsConfigDAO>
VoluntaryRefundsConfigDAO::_helper(_name);
VoluntaryRefundsConfigDAO* VoluntaryRefundsConfigDAO::_instance = nullptr;
}
