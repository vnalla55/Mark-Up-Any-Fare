//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BrandedFare;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode> BrandedFareKey;

class BrandedFareDAO : public DataAccessObject<BrandedFareKey, std::vector<BrandedFare*> >
{
public:
  static BrandedFareDAO& instance();

  const std::vector<BrandedFare*>& get(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BrandedFareKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  BrandedFareKey createKey(const BrandedFare* info);

  void translateKey(const BrandedFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BrandedFareDAO>;
  static DAOHelper<BrandedFareDAO> _helper;
  BrandedFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BrandedFareKey, std::vector<BrandedFare*> >(cacheSize, cacheType)
  {
  }
  virtual std::vector<BrandedFare*>* create(BrandedFareKey key) override;
  void destroy(BrandedFareKey key, std::vector<BrandedFare*>* recs) override;

private:
  static BrandedFareDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, CarrierCode, DateTime, DateTime> BrandedFareHistoricalKey;

class BrandedFareHistoricalDAO
    : public HistoricalDataAccessObject<BrandedFareHistoricalKey, std::vector<BrandedFare*> >
{
public:
  static BrandedFareHistoricalDAO& instance();

  const std::vector<BrandedFare*>& get(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BrandedFareHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    BrandedFareHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const BrandedFareHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(BrandedFareHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  BrandedFareHistoricalKey createKey(const BrandedFare* info,
                                     const DateTime& startDate = DateTime::emptyDate(),
                                     const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BrandedFareHistoricalDAO>;
  static DAOHelper<BrandedFareHistoricalDAO> _helper;

  BrandedFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BrandedFareHistoricalKey, std::vector<BrandedFare*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<BrandedFare*>* create(BrandedFareHistoricalKey key) override;
  void destroy(BrandedFareHistoricalKey key, std::vector<BrandedFare*>* recs) override;

private:
  static BrandedFareHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
