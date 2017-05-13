//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/YQYRFees.h"

namespace tse
{
class YQYRFees;
class DeleteList;

class YQYRFeesDAO : public DataAccessObject<CarrierKey, std::vector<YQYRFees*> >
{
public:
  static YQYRFeesDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const std::vector<YQYRFees*>& get(DeleteList& del, const CarrierCode& key);

  const std::string& cacheClass() override { return _cacheClass; }

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(YQYRFees* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<YQYRFees, YQYRFeesDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<YQYRFees*>* vect) const override;

  virtual std::vector<YQYRFees*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<YQYRFeesDAO>;

  static DAOHelper<YQYRFeesDAO> _helper;

  YQYRFeesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<YQYRFees*> >(cacheSize, cacheType, 6)
  {
  }

  virtual void load() override;

  std::vector<YQYRFees*>* create(CarrierKey key) override;

  void destroy(CarrierKey key, std::vector<YQYRFees*>* t) override;

  friend class TaxYqDaoTest;

private:
  struct isEffective;
  static YQYRFeesDAO* _instance;
};

// --------------------------------------------------
// Historical DAO: YQYRFeesHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> YQYRFeesHistoricalKey;

class YQYRFeesHistoricalDAO
    : public HistoricalDataAccessObject<YQYRFeesHistoricalKey, std::vector<YQYRFees*> >
{
public:
  static YQYRFeesHistoricalDAO& instance();

  const std::vector<YQYRFees*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& ticketDate);

  const std::string& cacheClass() override { return _cacheClass; }

  bool translateKey(const ObjectKey& objectKey, YQYRFeesHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    YQYRFeesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(YQYRFeesHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  virtual sfc::CompressedData* compress(const std::vector<YQYRFees*>* vect) const override;

  virtual std::vector<YQYRFees*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  class IsNotEffectiveYQHist
  {
  public:
    IsNotEffectiveYQHist(const DateTime& ticketDate)
      : _ticketDateNoTime(ticketDate.date()),
        _ticketDate(ticketDate),
        _ticketDateWithTime(ticketDate)
    {
    }

    // returns true if expression fails
    bool operator()(const YQYRFees* rec) const
    {
      if (_ticketDateNoTime > rec->lastTktDate().date())
        return true;
      if (_ticketDateNoTime < rec->firstTktDate().date())
        return true;

      if (_ticketDateWithTime.historicalIncludesTime())
      {
        if (_ticketDateWithTime < rec->createDate())
          return true;
      }
      else if (_ticketDateNoTime < rec->createDate().date())
        return true;

      return false;
    }

  private:
    const boost::gregorian::date _ticketDateNoTime;
    DateTime _ticketDate;
    DateTime _ticketDateWithTime;
  };
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<YQYRFeesHistoricalDAO>;

  static DAOHelper<YQYRFeesHistoricalDAO> _helper;

  YQYRFeesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<YQYRFeesHistoricalKey, std::vector<YQYRFees*> >(
          cacheSize, cacheType, 5)
  {
  }

  std::vector<YQYRFees*>* create(YQYRFeesHistoricalKey key) override;

  void destroy(YQYRFeesHistoricalKey key, std::vector<YQYRFees*>* t) override;

private:
  static log4cxx::LoggerPtr _logger;
  static YQYRFeesHistoricalDAO* _instance;
};
} // namespace tse

