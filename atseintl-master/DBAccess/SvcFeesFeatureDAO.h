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
class SvcFeesFeatureInfo;
class DeleteList;

typedef HashKey<VendorCode, long long> SvcFeesFeatureKey;

class SvcFeesFeatureDAO
    : public DataAccessObject<SvcFeesFeatureKey, std::vector<SvcFeesFeatureInfo*> >
{
public:
  static SvcFeesFeatureDAO& instance();

  const std::vector<SvcFeesFeatureInfo*>&
  get(DeleteList& del, const VendorCode& vendor, long long itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesFeatureKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SvcFeesFeatureKey createKey(const SvcFeesFeatureInfo* info);

  void translateKey(const SvcFeesFeatureKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesFeatureDAO>;
  static DAOHelper<SvcFeesFeatureDAO> _helper;
  SvcFeesFeatureDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesFeatureKey, std::vector<SvcFeesFeatureInfo*> >(cacheSize, cacheType)
  {
  }
  virtual std::vector<SvcFeesFeatureInfo*>* create(SvcFeesFeatureKey key) override;
  void destroy(SvcFeesFeatureKey key, std::vector<SvcFeesFeatureInfo*>* recs) override;

private:
  static SvcFeesFeatureDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, long long, DateTime, DateTime> SvcFeesFeatureHistoricalKey;

class SvcFeesFeatureHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesFeatureHistoricalKey,
                                        std::vector<SvcFeesFeatureInfo*> >
{
public:
  static SvcFeesFeatureHistoricalDAO& instance();

  const std::vector<SvcFeesFeatureInfo*>&
  get(DeleteList& del, const VendorCode& vendor, long long itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesFeatureHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesFeatureHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void translateKey(const SvcFeesFeatureHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(SvcFeesFeatureHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  SvcFeesFeatureHistoricalKey createKey(const SvcFeesFeatureInfo* info,
                                        const DateTime& startDate = DateTime::emptyDate(),
                                        const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesFeatureHistoricalDAO>;
  static DAOHelper<SvcFeesFeatureHistoricalDAO> _helper;

  SvcFeesFeatureHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesFeatureHistoricalKey, std::vector<SvcFeesFeatureInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<SvcFeesFeatureInfo*>* create(SvcFeesFeatureHistoricalKey key) override;
  void destroy(SvcFeesFeatureHistoricalKey key, std::vector<SvcFeesFeatureInfo*>* recs) override;

private:
  static SvcFeesFeatureHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
