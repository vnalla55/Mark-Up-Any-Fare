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
class FareFocusLookupInfo;
class DeleteList;

typedef HashKey<PseudoCityCode> FareFocusLookupKey;

class FareFocusLookupDAO
  : public DataAccessObject<FareFocusLookupKey, std::vector<FareFocusLookupInfo*> >
{
public:
  static FareFocusLookupDAO& instance();

  const FareFocusLookupInfo*
  get(DeleteList& del,
      const PseudoCityCode& pcc,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusLookupKey& key) const override
  {
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a);
  }

  FareFocusLookupKey createKey(const FareFocusLookupInfo* info);

  void translateKey(const FareFocusLookupKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITY", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusLookupDAO>;
  static DAOHelper<FareFocusLookupDAO> _helper;
  FareFocusLookupDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusLookupKey, std::vector<FareFocusLookupInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusLookupInfo*>* create(FareFocusLookupKey key) override;
  void destroy(FareFocusLookupKey key, std::vector<FareFocusLookupInfo*>* recs) override;

private:
  static FareFocusLookupDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<PseudoCityCode, DateTime, DateTime> FareFocusLookupHistoricalKey;

class FareFocusLookupHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusLookupHistoricalKey,
                                        std::vector<FareFocusLookupInfo*> >
{
public:
  static FareFocusLookupHistoricalDAO& instance();

  const FareFocusLookupInfo*
  get(DeleteList& del, const PseudoCityCode& pcc, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusLookupHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusLookupHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a);
  }

  void translateKey(const FareFocusLookupHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITY", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void setKeyDateRange(FareFocusLookupHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusLookupHistoricalKey createKey(const FareFocusLookupInfo* info,
                                         const DateTime& startDate = DateTime::emptyDate(),
                                         const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusLookupHistoricalDAO>;
  static DAOHelper<FareFocusLookupHistoricalDAO> _helper;

  FareFocusLookupHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusLookupHistoricalKey,
                                 std::vector<FareFocusLookupInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusLookupInfo*>* create(FareFocusLookupHistoricalKey key) override;
  void destroy(FareFocusLookupHistoricalKey key, std::vector<FareFocusLookupInfo*>* recs) override;

private:
  static FareFocusLookupHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
