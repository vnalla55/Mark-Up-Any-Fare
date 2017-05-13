#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class FareRetailerRuleLookupInfo;
class DeleteList;

typedef HashKey<Indicator, PseudoCityCode, PseudoCityCode> FareRetailerRuleLookupKey;

class FareRetailerRuleLookupDAO
  : public DataAccessObject<FareRetailerRuleLookupKey, std::vector<FareRetailerRuleLookupInfo*> >
{
public:
  static FareRetailerRuleLookupDAO& instance();

  const FareRetailerRuleLookupInfo*
  get(DeleteList& del,
      Indicator applicationType,
      const PseudoCityCode& sourcePcc,
      const PseudoCityCode& pcc,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerRuleLookupKey& key) const override
  {
    return key.initialized =  objectKey.getValue("APPLICATIONTYPE", key._a) &&
                              objectKey.getValue("SOURCEPSEUDOCITY", key._b) &&
                              objectKey.getValue("PSEUDOCITY", key._c);
  }

  FareRetailerRuleLookupKey createKey(const FareRetailerRuleLookupInfo* info);

  void translateKey(const FareRetailerRuleLookupKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("APPLICATIONTYPE", key._a);
    objectKey.setValue("SOURCEPSEUDOCITY", key._b);
    objectKey.setValue("PSEUDOCITY", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerRuleLookupDAO>;
  static DAOHelper<FareRetailerRuleLookupDAO> _helper;
  FareRetailerRuleLookupDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareRetailerRuleLookupKey, std::vector<FareRetailerRuleLookupInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareRetailerRuleLookupInfo*>* create(FareRetailerRuleLookupKey key) override;
  void destroy(FareRetailerRuleLookupKey key, std::vector<FareRetailerRuleLookupInfo*>* recs) override;

private:
  static FareRetailerRuleLookupDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<Indicator, PseudoCityCode, PseudoCityCode, DateTime, DateTime> FareRetailerRuleLookupHistoricalKey;

class FareRetailerRuleLookupHistoricalDAO
    : public HistoricalDataAccessObject<FareRetailerRuleLookupHistoricalKey,
                                        std::vector<FareRetailerRuleLookupInfo*> >
{
public:
  static FareRetailerRuleLookupHistoricalDAO& instance();

  const FareRetailerRuleLookupInfo*
  get(DeleteList& del, Indicator applicationType, const PseudoCityCode& sourcePcc, const PseudoCityCode& pcc, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerRuleLookupHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("APPLICATIONTYPE", key._a) &&
                             objectKey.getValue("SOURCEPSEUDOCITY", key._b) &&
                             objectKey.getValue("PSEUDOCITY", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareRetailerRuleLookupHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("APPLICATIONTYPE", key._a) &&
                             objectKey.getValue("SOURCEPSEUDOCITY", key._b) &&
                             objectKey.getValue("PSEUDOCITY", key._c);
  }

  void translateKey(const FareRetailerRuleLookupHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("APPLICATIONTYPE", key._a);
    objectKey.setValue("SOURCEPSEUDOCITY", key._b);
    objectKey.setValue("PSEUDOCITY", key._c);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  void setKeyDateRange(FareRetailerRuleLookupHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareRetailerRuleLookupHistoricalKey createKey(const FareRetailerRuleLookupInfo* info,
                                                const DateTime& startDate = DateTime::emptyDate(),
                                                const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerRuleLookupHistoricalDAO>;
  static DAOHelper<FareRetailerRuleLookupHistoricalDAO> _helper;

  FareRetailerRuleLookupHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareRetailerRuleLookupHistoricalKey,
                                 std::vector<FareRetailerRuleLookupInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareRetailerRuleLookupInfo*>* create(FareRetailerRuleLookupHistoricalKey key) override;
  void destroy(FareRetailerRuleLookupHistoricalKey key, std::vector<FareRetailerRuleLookupInfo*>* recs) override;

private:
  static FareRetailerRuleLookupHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

