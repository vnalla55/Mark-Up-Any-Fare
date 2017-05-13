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
class FareFocusPsgTypeInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusPsgTypeKey;

class FareFocusPsgTypeDAO
    : public DataAccessObject<FareFocusPsgTypeKey, std::vector<FareFocusPsgTypeInfo*> >
{
public:
  static FareFocusPsgTypeDAO& instance();

  const FareFocusPsgTypeInfo*
  get(DeleteList& del,
      uint64_t psgTypeItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusPsgTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("PSGTYPEITEMNO", key._a);
  }

  FareFocusPsgTypeKey createKey(const FareFocusPsgTypeInfo* info);

  void translateKey(const FareFocusPsgTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSGTYPEITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusPsgTypeDAO>;
  static DAOHelper<FareFocusPsgTypeDAO> _helper;
  FareFocusPsgTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusPsgTypeKey, std::vector<FareFocusPsgTypeInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<FareFocusPsgTypeInfo*>* create(FareFocusPsgTypeKey key) override;
  void destroy(FareFocusPsgTypeKey key, std::vector<FareFocusPsgTypeInfo*>* recs) override;

private:
  static FareFocusPsgTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusPsgTypeHistoricalKey;

class FareFocusPsgTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusPsgTypeHistoricalKey,
                                        std::vector<FareFocusPsgTypeInfo*> >
{
public:
  static FareFocusPsgTypeHistoricalDAO& instance();

  const FareFocusPsgTypeInfo*
  get(DeleteList& del, uint64_t psgTypeItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusPsgTypeHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("PSGTYPEITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusPsgTypeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("PSGTYPEITEMNO", key._a);
  }

  void translateKey(const FareFocusPsgTypeHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSGTYPEITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusPsgTypeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusPsgTypeHistoricalKey createKey(const FareFocusPsgTypeInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusPsgTypeHistoricalDAO>;
  static DAOHelper<FareFocusPsgTypeHistoricalDAO> _helper;

  FareFocusPsgTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusPsgTypeHistoricalKey,
                                 std::vector<FareFocusPsgTypeInfo*> >(cacheSize, cacheType,2)
  {
  }
  std::vector<FareFocusPsgTypeInfo*>* create(FareFocusPsgTypeHistoricalKey key) override;
  void
  destroy(FareFocusPsgTypeHistoricalKey key, std::vector<FareFocusPsgTypeInfo*>* recs) override;

private:
  static FareFocusPsgTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
