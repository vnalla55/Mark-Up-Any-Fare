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
#include "DBAccess/DAOUtils.h"
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
class TicketingFeesInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode> TicketingFeesKey;

class TicketingFeesDAO : public DataAccessObject<TicketingFeesKey, std::vector<TicketingFeesInfo*> >
{
public:
  static TicketingFeesDAO& instance();

  const std::vector<TicketingFeesInfo*>& get(DeleteList& del,
                                             const VendorCode& vendor,
                                             const CarrierCode& validatingCarrier,
                                             const DateTime& date,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TicketingFeesKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b)));
  }

  TicketingFeesKey createKey(TicketingFeesInfo* info);

  void translateKey(const TicketingFeesKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TicketingFeesInfo, TicketingFeesDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketingFeesDAO>;
  static DAOHelper<TicketingFeesDAO> _helper;
  TicketingFeesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TicketingFeesKey, std::vector<TicketingFeesInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<TicketingFeesInfo*>* create(TicketingFeesKey key) override;
  void destroy(TicketingFeesKey key, std::vector<TicketingFeesInfo*>* recs) override;

  void load() override;

private:
  static TicketingFeesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, CarrierCode, DateTime, DateTime> TicketingFeesHistoricalKey;

class TicketingFeesHistoricalDAO
    : public HistoricalDataAccessObject<TicketingFeesHistoricalKey,
                                        std::vector<TicketingFeesInfo*> >
{
public:
  static TicketingFeesHistoricalDAO& instance();

  const std::vector<TicketingFeesInfo*>& get(DeleteList& del,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const DateTime& date,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TicketingFeesHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    TicketingFeesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void setKeyDateRange(TicketingFeesHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketingFeesHistoricalDAO>;
  static DAOHelper<TicketingFeesHistoricalDAO> _helper;
  TicketingFeesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TicketingFeesHistoricalKey, std::vector<TicketingFeesInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<TicketingFeesInfo*>* create(TicketingFeesHistoricalKey key) override;
  void destroy(TicketingFeesHistoricalKey key, std::vector<TicketingFeesInfo*>* t) override;

private:
  static TicketingFeesHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
