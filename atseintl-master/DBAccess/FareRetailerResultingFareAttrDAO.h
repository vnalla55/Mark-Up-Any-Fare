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
class FareRetailerResultingFareAttrInfo;
class DeleteList;

typedef HashKey<uint64_t> FareRetailerResultingFareAttrKey;

class FareRetailerResultingFareAttrDAO
    : public DataAccessObject<FareRetailerResultingFareAttrKey, std::vector<FareRetailerResultingFareAttrInfo*> >
{
public:
  static FareRetailerResultingFareAttrDAO& instance();

  const FareRetailerResultingFareAttrInfo*   get(DeleteList& del,
                                                 uint64_t resultingFareAttrItemNo,
                                                 DateTime adjustedTicketDate,
                                                 DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerResultingFareAttrKey& key) const override
  {
    return key.initialized = objectKey.getValue("RESULTINGFAREATTRITEMNO", key._a);
  }

  FareRetailerResultingFareAttrKey createKey(const FareRetailerResultingFareAttrInfo* info);

  void translateKey(const FareRetailerResultingFareAttrKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("RESULTINGFAREATTRITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerResultingFareAttrDAO>;
  static DAOHelper<FareRetailerResultingFareAttrDAO> _helper;
  FareRetailerResultingFareAttrDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareRetailerResultingFareAttrKey, std::vector<FareRetailerResultingFareAttrInfo*> >(
          cacheSize, cacheType, 1)
  {
  }
  virtual std::vector<FareRetailerResultingFareAttrInfo*>* create(FareRetailerResultingFareAttrKey key) override;
  void destroy(FareRetailerResultingFareAttrKey key, std::vector<FareRetailerResultingFareAttrInfo*>* recs) override;

private:
  static FareRetailerResultingFareAttrDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareRetailerResultingFareAttrHistoricalKey;

class FareRetailerResultingFareAttrHistoricalDAO
    : public HistoricalDataAccessObject<FareRetailerResultingFareAttrHistoricalKey,
                                        std::vector<FareRetailerResultingFareAttrInfo*> >
{
public:
  static FareRetailerResultingFareAttrHistoricalDAO& instance();

  const FareRetailerResultingFareAttrInfo*
  get(DeleteList& del, uint64_t resultingFareAttrItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerResultingFareAttrHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("RESULTINGFAREATTRITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareRetailerResultingFareAttrHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("RESULTINGFAREATTRITEMNO", key._a);
  }

  void translateKey(const FareRetailerResultingFareAttrHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("RESULTINGFAREATTRITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void setKeyDateRange(FareRetailerResultingFareAttrHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareRetailerResultingFareAttrHistoricalKey createKey(const FareRetailerResultingFareAttrInfo* info,
                                          const DateTime& startDate = DateTime::emptyDate(),
                                          const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerResultingFareAttrHistoricalDAO>;
  static DAOHelper<FareRetailerResultingFareAttrHistoricalDAO> _helper;

  FareRetailerResultingFareAttrHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareRetailerResultingFareAttrHistoricalKey,
                                 std::vector<FareRetailerResultingFareAttrInfo*> >(cacheSize, cacheType, 1)
  {
  }
  std::vector<FareRetailerResultingFareAttrInfo*>* create(FareRetailerResultingFareAttrHistoricalKey key) override;
  void destroy(FareRetailerResultingFareAttrHistoricalKey key, std::vector<FareRetailerResultingFareAttrInfo*>* recs) override;

private:
  static FareRetailerResultingFareAttrHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse



