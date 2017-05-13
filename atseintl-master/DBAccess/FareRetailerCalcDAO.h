#pragma once

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
class FareRetailerCalcInfo;
class DeleteList;

typedef HashKey<uint64_t> FareRetailerCalcKey;

class FareRetailerCalcDAO
    : public DataAccessObject<FareRetailerCalcKey, std::vector<FareRetailerCalcInfo*> >
{
public:
  static FareRetailerCalcDAO& instance();

  const std::vector<FareRetailerCalcInfo*>&
  get(DeleteList& del,
      uint64_t fareRetailerCalcItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerCalcKey& key) const override
  {
    return key.initialized = objectKey.getValue("FARERETAILERCALCITEMNO", key._a);
  }

  FareRetailerCalcKey createKey(const FareRetailerCalcInfo* info);

  void translateKey(const FareRetailerCalcKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARERETAILERCALCITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerCalcDAO>;
  static DAOHelper<FareRetailerCalcDAO> _helper;
  FareRetailerCalcDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareRetailerCalcKey, std::vector<FareRetailerCalcInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<FareRetailerCalcInfo*>* create(FareRetailerCalcKey key) override;
  void destroy(FareRetailerCalcKey key, std::vector<FareRetailerCalcInfo*>* recs) override;

private:
  static FareRetailerCalcDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareRetailerCalcHistoricalKey;

class FareRetailerCalcHistoricalDAO
    : public HistoricalDataAccessObject<FareRetailerCalcHistoricalKey,
                                        std::vector<FareRetailerCalcInfo*> >
{
public:
  static FareRetailerCalcHistoricalDAO& instance();

  const std::vector<FareRetailerCalcInfo*>&
  get(DeleteList& del, uint64_t fareRetailerCalcItemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerCalcHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("FARERETAILERCALCITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareRetailerCalcHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("FARERETAILERCALCITEMNO", key._a);
  }

  void translateKey(const FareRetailerCalcHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARERETAILERCALCITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareRetailerCalcHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareRetailerCalcHistoricalKey createKey(const FareRetailerCalcInfo* info,
                                          const DateTime& startDate = DateTime::emptyDate(),
                                          const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerCalcHistoricalDAO>;
  static DAOHelper<FareRetailerCalcHistoricalDAO> _helper;

  FareRetailerCalcHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareRetailerCalcHistoricalKey,
                                 std::vector<FareRetailerCalcInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<FareRetailerCalcInfo*>* create(FareRetailerCalcHistoricalKey key) override;
  void
  destroy(FareRetailerCalcHistoricalKey key, std::vector<FareRetailerCalcInfo*>* recs) override;

private:
  static FareRetailerCalcHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse




