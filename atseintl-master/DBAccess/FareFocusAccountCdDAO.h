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
class FareFocusAccountCdInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusAccountCdKey;

class FareFocusAccountCdDAO
    : public DataAccessObject<FareFocusAccountCdKey, std::vector<FareFocusAccountCdInfo*> >
{
public:
  static FareFocusAccountCdDAO& instance();

  const FareFocusAccountCdInfo*
  get(DeleteList& del,
      uint64_t accountCdItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusAccountCdKey& key) const override
  {
    return key.initialized = objectKey.getValue("ACCOUNTCDITEMNO", key._a);
  }

  FareFocusAccountCdKey createKey(const FareFocusAccountCdInfo* info);

  void translateKey(const FareFocusAccountCdKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ACCOUNTCDITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusAccountCdDAO>;
  static DAOHelper<FareFocusAccountCdDAO> _helper;
  FareFocusAccountCdDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusAccountCdKey, std::vector<FareFocusAccountCdInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusAccountCdInfo*>* create(FareFocusAccountCdKey key) override;
  void destroy(FareFocusAccountCdKey key, std::vector<FareFocusAccountCdInfo*>* recs) override;

private:
  static FareFocusAccountCdDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusAccountCdHistoricalKey;

class FareFocusAccountCdHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusAccountCdHistoricalKey,
                                        std::vector<FareFocusAccountCdInfo*> >
{
public:
  static FareFocusAccountCdHistoricalDAO& instance();

  const FareFocusAccountCdInfo*
  get(DeleteList& del, uint64_t accountCdItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusAccountCdHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("ACCOUNTCDITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusAccountCdHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("ACCOUNTCDITEMNO", key._a);
  }

  void translateKey(const FareFocusAccountCdHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ACCOUNTCDITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusAccountCdHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusAccountCdHistoricalKey createKey(const FareFocusAccountCdInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusAccountCdHistoricalDAO>;
  static DAOHelper<FareFocusAccountCdHistoricalDAO> _helper;

  FareFocusAccountCdHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusAccountCdHistoricalKey,
                                 std::vector<FareFocusAccountCdInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusAccountCdInfo*>* create(FareFocusAccountCdHistoricalKey key) override;
  void
  destroy(FareFocusAccountCdHistoricalKey key, std::vector<FareFocusAccountCdInfo*>* recs) override;

private:
  static FareFocusAccountCdHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
