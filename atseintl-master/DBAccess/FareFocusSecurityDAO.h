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
class FareFocusSecurityInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusSecurityKey;

class FareFocusSecurityDAO
    : public DataAccessObject<FareFocusSecurityKey, std::vector<FareFocusSecurityInfo*> >
{
public:
  static FareFocusSecurityDAO& instance();

  const FareFocusSecurityInfo*
  get(DeleteList& del,
      uint64_t securityItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusSecurityKey& key) const override
  {
    return key.initialized = objectKey.getValue("SECURITYITEMNO", key._a);
  }

  FareFocusSecurityKey createKey(const FareFocusSecurityInfo* info);

  void translateKey(const FareFocusSecurityKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SECURITYITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusSecurityDAO>;
  static DAOHelper<FareFocusSecurityDAO> _helper;
  FareFocusSecurityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusSecurityKey, std::vector<FareFocusSecurityInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusSecurityInfo*>* create(FareFocusSecurityKey key) override;
  void destroy(FareFocusSecurityKey key, std::vector<FareFocusSecurityInfo*>* recs) override;

private:
  static FareFocusSecurityDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusSecurityHistoricalKey;

class FareFocusSecurityHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusSecurityHistoricalKey,
                                        std::vector<FareFocusSecurityInfo*> >
{
public:
  static FareFocusSecurityHistoricalDAO& instance();

  const FareFocusSecurityInfo*
  get(DeleteList& del, uint64_t securityItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusSecurityHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("SECURITYITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusSecurityHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("SECURITYITEMNO", key._a);
  }

  void translateKey(const FareFocusSecurityHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SECURITYITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusSecurityHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusSecurityHistoricalKey createKey(const FareFocusSecurityInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusSecurityHistoricalDAO>;
  static DAOHelper<FareFocusSecurityHistoricalDAO> _helper;

  FareFocusSecurityHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusSecurityHistoricalKey,
                                 std::vector<FareFocusSecurityInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusSecurityInfo*>* create(FareFocusSecurityHistoricalKey key) override;
  void
  destroy(FareFocusSecurityHistoricalKey key, std::vector<FareFocusSecurityInfo*>* recs) override;

private:
  static FareFocusSecurityHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
