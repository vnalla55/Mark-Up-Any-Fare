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
class FareFocusDaytimeApplInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusDaytimeApplKey;

class FareFocusDaytimeApplDAO
    : public DataAccessObject<FareFocusDaytimeApplKey, std::vector<FareFocusDaytimeApplInfo*> >
{
public:
  static FareFocusDaytimeApplDAO& instance();

  const FareFocusDaytimeApplInfo*
  get(DeleteList& del,
      uint64_t dayTimeApplItemNo,
      DateTime createDate,
      DateTime expireDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusDaytimeApplKey& key) const override
  {
    return key.initialized = objectKey.getValue("DAYTIMEAPPLITEMNO", key._a);
  }

  FareFocusDaytimeApplKey createKey(const FareFocusDaytimeApplInfo* info);

  void translateKey(const FareFocusDaytimeApplKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DAYTIMEAPPLITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusDaytimeApplDAO>;
  static DAOHelper<FareFocusDaytimeApplDAO> _helper;
  FareFocusDaytimeApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusDaytimeApplKey, std::vector<FareFocusDaytimeApplInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<FareFocusDaytimeApplInfo*>* create(FareFocusDaytimeApplKey key) override;
  void destroy(FareFocusDaytimeApplKey key, std::vector<FareFocusDaytimeApplInfo*>* recs) override;

private:
  static FareFocusDaytimeApplDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusDaytimeApplHistoricalKey;

class FareFocusDaytimeApplHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusDaytimeApplHistoricalKey,
                                        std::vector<FareFocusDaytimeApplInfo*> >
{
public:
  static FareFocusDaytimeApplHistoricalDAO& instance();

  const FareFocusDaytimeApplInfo*
  get(DeleteList& del, uint64_t dayTimeApplItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusDaytimeApplHistoricalKey& key) const override
  {
    objectKey.getValue("CREATEDATE", key._b);
    objectKey.getValue("EXPIREDATE", key._c);
    return key.initialized = objectKey.getValue("DAYTIMEAPPLITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
      FareFocusDaytimeApplHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("DAYTIMEAPPLITEMNO", key._a);
  }

  void translateKey(const FareFocusDaytimeApplHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DAYTIMEAPPLITEMNO", key._a);
    objectKey.setValue("CREATEDATE", key._b);
    objectKey.setValue("EXPIREDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusDaytimeApplHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusDaytimeApplHistoricalKey createKey(const FareFocusDaytimeApplInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusDaytimeApplHistoricalDAO>;
  static DAOHelper<FareFocusDaytimeApplHistoricalDAO> _helper;

  FareFocusDaytimeApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusDaytimeApplHistoricalKey,
                                 std::vector<FareFocusDaytimeApplInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<FareFocusDaytimeApplInfo*>* create(FareFocusDaytimeApplHistoricalKey key) override;
  void
  destroy(FareFocusDaytimeApplHistoricalKey key, std::vector<FareFocusDaytimeApplInfo*>* recs) override;

private:
  static FareFocusDaytimeApplHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
