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
class AccompaniedTravelInfo;
class DeleteList;

typedef HashKey<VendorCode, int> AccompaniedTravelKey;

class AccompaniedTravelDAO
    : public DataAccessObject<AccompaniedTravelKey, std::vector<AccompaniedTravelInfo*> >
{
public:
  static AccompaniedTravelDAO& instance();

  const AccompaniedTravelInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AccompaniedTravelKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  AccompaniedTravelKey createKey(const AccompaniedTravelInfo* info);

  void translateKey(const AccompaniedTravelKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AccompaniedTravelInfo, AccompaniedTravelDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AccompaniedTravelDAO>;
  static DAOHelper<AccompaniedTravelDAO> _helper;
  AccompaniedTravelDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AccompaniedTravelKey, std::vector<AccompaniedTravelInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<AccompaniedTravelInfo*>* create(AccompaniedTravelKey key) override;
  void destroy(AccompaniedTravelKey key, std::vector<AccompaniedTravelInfo*>* t) override;
  void load() override;

private:
  static AccompaniedTravelDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: AccompaniedTravelHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> AccompaniedTravelHistoricalKey;

class AccompaniedTravelHistoricalDAO
    : public HistoricalDataAccessObject<AccompaniedTravelHistoricalKey,
                                        std::vector<AccompaniedTravelInfo*> >
{
public:
  static AccompaniedTravelHistoricalDAO& instance();
  const AccompaniedTravelInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AccompaniedTravelHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AccompaniedTravelHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(AccompaniedTravelHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AccompaniedTravelHistoricalDAO>;
  static DAOHelper<AccompaniedTravelHistoricalDAO> _helper;
  AccompaniedTravelHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AccompaniedTravelHistoricalKey,
                                 std::vector<AccompaniedTravelInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<AccompaniedTravelInfo*>* create(AccompaniedTravelHistoricalKey key) override;
  void destroy(AccompaniedTravelHistoricalKey key, std::vector<AccompaniedTravelInfo*>* t) override;

private:
  static AccompaniedTravelHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

