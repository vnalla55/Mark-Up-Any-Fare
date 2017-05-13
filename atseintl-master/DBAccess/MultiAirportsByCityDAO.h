//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class MultiAirportsByCity;
class DeleteList;

class MultiAirportsByCityDAO
    : public DataAccessObject<LocCodeKey, std::vector<MultiAirportsByCity*>, false>
{
public:
  static MultiAirportsByCityDAO& instance();
  const std::vector<MultiAirportsByCity*>&
  get(DeleteList& del, const LocCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  LocCodeKey createKey(MultiAirportsByCity* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MultiAirportsByCity, MultiAirportsByCityDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<MultiAirportsByCity*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<MultiAirportsByCity*>* recs) override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MultiAirportsByCityDAO>;
  static DAOHelper<MultiAirportsByCityDAO> _helper;
  MultiAirportsByCityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<MultiAirportsByCity*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;

private:
  static MultiAirportsByCityDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
