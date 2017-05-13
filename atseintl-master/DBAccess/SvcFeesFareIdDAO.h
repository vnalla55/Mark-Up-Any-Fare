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
class SvcFeesFareIdInfo;
class DeleteList;

typedef HashKey<VendorCode, long long> SvcFeesFareIdKey;

class SvcFeesFareIdDAO : public DataAccessObject<SvcFeesFareIdKey, std::vector<SvcFeesFareIdInfo*> >
{
public:
  static SvcFeesFareIdDAO& instance();

  const std::vector<SvcFeesFareIdInfo*>&
  get(DeleteList& del, const VendorCode& vendor, long long itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesFareIdKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SvcFeesFareIdKey createKey(const SvcFeesFareIdInfo* info);

  void translateKey(const SvcFeesFareIdKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesFareIdDAO>;
  static DAOHelper<SvcFeesFareIdDAO> _helper;
  SvcFeesFareIdDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesFareIdKey, std::vector<SvcFeesFareIdInfo*> >(cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<SvcFeesFareIdInfo*>* create(SvcFeesFareIdKey key) override;
  void destroy(SvcFeesFareIdKey key, std::vector<SvcFeesFareIdInfo*>* recs) override;

private:
  static SvcFeesFareIdDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, long long, DateTime, DateTime> SvcFeesFareIdHistoricalKey;

class SvcFeesFareIdHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesFareIdHistoricalKey,
                                        std::vector<SvcFeesFareIdInfo*> >
{
public:
  static SvcFeesFareIdHistoricalDAO& instance();

  const std::vector<SvcFeesFareIdInfo*>&
  get(DeleteList& del, const VendorCode& vendor, long long itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesFareIdHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesFareIdHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void translateKey(const SvcFeesFareIdHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(SvcFeesFareIdHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  SvcFeesFareIdHistoricalKey createKey(const SvcFeesFareIdInfo* info,
                                       const DateTime& startDate = DateTime::emptyDate(),
                                       const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesFareIdHistoricalDAO>;
  static DAOHelper<SvcFeesFareIdHistoricalDAO> _helper;

  SvcFeesFareIdHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesFareIdHistoricalKey, std::vector<SvcFeesFareIdInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesFareIdInfo*>* create(SvcFeesFareIdHistoricalKey key) override;
  void destroy(SvcFeesFareIdHistoricalKey key, std::vector<SvcFeesFareIdInfo*>* recs) override;

private:
  static SvcFeesFareIdHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
