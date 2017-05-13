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
class MultiAirportCity;
class DeleteList;

class MultiCityAirportDAO
    : public DataAccessObject<LocCodeKey, std::vector<MultiAirportCity*>, false>
{
public:
  static MultiCityAirportDAO& instance();
  const std::vector<MultiAirportCity*>&
  get(DeleteList& del, const LocCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  LocCodeKey createKey(MultiAirportCity* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MultiAirportCity, MultiCityAirportDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MultiAirportCity*>* vect) const override;

  virtual std::vector<MultiAirportCity*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MultiCityAirportDAO>;
  static DAOHelper<MultiCityAirportDAO> _helper;
  MultiCityAirportDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<MultiAirportCity*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;

  std::vector<MultiAirportCity*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<MultiAirportCity*>* recs) override;

private:
  static MultiCityAirportDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
