//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DaoFilterIteratorUtils.h"
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
class BankerSellRate;
class DeleteList;

class BankerSellRateDAO : public DataAccessObject<CurrencyKey, std::vector<BankerSellRate*> >
{
public:
  static BankerSellRateDAO& instance();

  const std::vector<BankerSellRate*>& get(DeleteList& del,
                                          const CurrencyCode& primeCur,
                                          const CurrencyCode& cur,
                                          const DateTime& date,
                                          const DateTime& ticketDate);

  FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
  getRange(DeleteList& del,
           const CurrencyCode& primeCur,
           const CurrencyCode& cur,
           const DateTime& date,
           const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CurrencyKey& key) const override
  {
    return key.initialized = objectKey.getValue("CURRENCYCODE", key._a);
  }

  CurrencyKey createKey(BankerSellRate* info);

  void translateKey(const CurrencyKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CURRENCYCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BankerSellRate, BankerSellRateDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
    compress(const std::vector<BankerSellRate*>* vect) const override;

  virtual std::vector<BankerSellRate*>*
    uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BankerSellRateDAO>;

  static DAOHelper<BankerSellRateDAO> _helper;

  BankerSellRateDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CurrencyKey, std::vector<BankerSellRate*> >(cacheSize, cacheType)
  {
  }

  void load() override;

  std::vector<BankerSellRate*>* create(CurrencyKey key) override;

  void destroy(CurrencyKey key, std::vector<BankerSellRate*>* recs) override;

private:
  struct compareCur;
  struct groupByKey;
  static BankerSellRateDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class BankerSellRateDAO

// Historical Stuff //////////////////////////////////////////////////////////////////////
typedef HashKey<CurrencyCode, DateTime, DateTime> BankerSellRateHistoricalKey;

class BankerSellRateHistoricalDAO
    : public HistoricalDataAccessObject<BankerSellRateHistoricalKey, std::vector<BankerSellRate*> >
{
public:
  static BankerSellRateHistoricalDAO& instance();

  const std::vector<BankerSellRate*>& get(DeleteList& del,
                                          const CurrencyCode& primeCur,
                                          const CurrencyCode& cur,
                                          const DateTime& ticketDate,
                                          const DateTime& date);

  FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
  getRange(DeleteList& del,
           const CurrencyCode& primeCur,
           const CurrencyCode& cur,
           const DateTime& ticketDate,
           const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, BankerSellRateHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CURRENCYCODE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    BankerSellRateHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CURRENCYCODE", key._a);
  }

  void setKeyDateRange(BankerSellRateHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
    compress(const std::vector<BankerSellRate*>* vect) const override;

  virtual std::vector<BankerSellRate*>*
    uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BankerSellRateHistoricalDAO>;
  static DAOHelper<BankerSellRateHistoricalDAO> _helper;

  BankerSellRateHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BankerSellRateHistoricalKey, std::vector<BankerSellRate*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<BankerSellRate*>* create(BankerSellRateHistoricalKey key) override;
  void destroy(BankerSellRateHistoricalKey key, std::vector<BankerSellRate*>* t) override;

private:
  struct IsNotCur;
  static BankerSellRateHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;

}; // class BankerSellRateHistoricalDAO

} // namespace tse
