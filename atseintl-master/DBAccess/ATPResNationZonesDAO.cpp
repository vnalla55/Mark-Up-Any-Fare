//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/ATPResNationZonesDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/ATPResNationZones.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAllATPResNationZones.h"

// Warning!
// If changing this DAO, in particular, adding filtering and creation of the vector
// on every call to get(...), change or disable TSSCache implementation of getATPResNationZones
// in TSSCache.h. Currently it relies on get(...) returning vector<ATPResNationZones*>
// from the cache, not a newly created vector. See comment in TSSCache.h

namespace tse
{
log4cxx::LoggerPtr
ATPResNationZonesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ATPResNationZonesDAO"));

ATPResNationZonesDAO&
ATPResNationZonesDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ATPResNationZones*>&
getATPResNationZonesData(const NationCode& key, DeleteList& deleteList)
{
  ATPResNationZonesDAO& dao = ATPResNationZonesDAO::instance();
  return dao.get(deleteList, key);
}

const std::vector<ATPResNationZones*>&
ATPResNationZonesDAO::get(DeleteList& del, const NationCode& key)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(NationKey(key));
  del.copy(ptr);

  return *ptr;
}

NationKey
ATPResNationZonesDAO::createKey(ATPResNationZones* info)
{
  return NationKey(info->nation());
}

void
ATPResNationZonesDAO::load()
{
  StartupLoader<QueryGetAllATPResNationZones, ATPResNationZones, ATPResNationZonesDAO>();
}

std::vector<ATPResNationZones*>*
ATPResNationZonesDAO::create(NationKey key)
{
  return new std::vector<ATPResNationZones*>;
}

void
ATPResNationZonesDAO::destroy(NationKey key, std::vector<ATPResNationZones*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
ATPResNationZonesDAO::compress(const std::vector<ATPResNationZones*>* vect) const
{
  return compressVector(vect);
}

std::vector<ATPResNationZones*>*
ATPResNationZonesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<ATPResNationZones>(compressed);
}

std::string
ATPResNationZonesDAO::_name("ATPResNationZones");
std::string
ATPResNationZonesDAO::_cacheClass("Common");

DAOHelper<ATPResNationZonesDAO>
ATPResNationZonesDAO::_helper(_name);

ATPResNationZonesDAO* ATPResNationZonesDAO::_instance = nullptr;

} // namespace tse
