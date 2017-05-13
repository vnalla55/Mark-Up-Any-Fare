//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class AddonFareInfo;
class DeleteList;

typedef HashKey<LocCode, CarrierCode> AddOnFareKey;

class AddOnFareDAO : public DataAccessObject<AddOnFareKey, std::vector<AddonFareInfo*> >
{
public:
  static AddOnFareDAO& instance();

  const std::vector<AddonFareInfo*>& get(DeleteList& del,
                                         const LocCode& interiorMarket,
                                         const CarrierCode& carrier,
                                         const DateTime& date,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool translateKey(const ObjectKey& objectKey, AddOnFareKey& key) const override
  {
    return key.initialized = objectKey.getValue("INTERIORMARKET", key._a) &&
                             objectKey.getValue("CARRIER", key._b);
  }

  AddOnFareKey createKey(AddonFareInfo* info);

  void translateKey(const AddOnFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("INTERIORMARKET", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonFareInfo, AddOnFareDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AddonFareInfo*>* vect) const override;

  virtual std::vector<AddonFareInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnFareDAO>;
  static DAOHelper<AddOnFareDAO> _helper;
  AddOnFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnFareKey, std::vector<AddonFareInfo*> >(cacheSize, cacheType, 5)
  {
  }
  void load() override;
  std::vector<AddonFareInfo*>* create(AddOnFareKey key) override;
  void destroy(AddOnFareKey key, std::vector<AddonFareInfo*>* t) override;

private:
  struct isEffective;
  struct groupByKey;
  static AddOnFareDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnFareDAO

// Historical Stuff ////////////////////////////////////////////////////////////////////////////
typedef HashKey<LocCode, CarrierCode, DateTime, DateTime> AddOnFareHistoricalKey;

class AddOnFareHistoricalDAO
    : public HistoricalDataAccessObject<AddOnFareHistoricalKey, std::vector<AddonFareInfo*> >
{
public:
  static AddOnFareHistoricalDAO& instance();

  const std::vector<AddonFareInfo*>& get(DeleteList& del,
                                         const LocCode& interiorMarket,
                                         const CarrierCode& carrier,
                                         const DateTime& date,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool translateKey(const ObjectKey& objectKey, AddOnFareHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("INTERIORMARKET", key._a)
                             && objectKey.getValue("CARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddOnFareHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("INTERIORMARKET", key._a) &&
                             objectKey.getValue("CARRIER", key._b);
  }

  void setKeyDateRange(AddOnFareHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AddonFareInfo*>* vect) const override;

  virtual std::vector<AddonFareInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnFareHistoricalDAO>;
  static DAOHelper<AddOnFareHistoricalDAO> _helper;
  AddOnFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddOnFareHistoricalKey, std::vector<AddonFareInfo*> >(
          cacheSize, cacheType, 5)
  {
  }
  std::vector<AddonFareInfo*>* create(AddOnFareHistoricalKey key) override;
  void destroy(AddOnFareHistoricalKey key, std::vector<AddonFareInfo*>* t) override;

private:
  struct isEffective;
  static AddOnFareHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnFareHistDAO
} // namespace tse
