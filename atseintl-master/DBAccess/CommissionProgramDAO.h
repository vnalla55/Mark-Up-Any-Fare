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
class CommissionProgramInfo;
class DeleteList;

typedef HashKey<VendorCode, uint64_t> CommissionProgramKey;

class CommissionProgramDAO
    : public DataAccessObject<CommissionProgramKey, std::vector<CommissionProgramInfo*> >
{
public:
  static CommissionProgramDAO& instance();

  const std::vector<CommissionProgramInfo*>& get(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 uint64_t contractId,
                                                 DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionProgramKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CONTRACTID", key._b);
  }

  CommissionProgramKey createKey(const CommissionProgramInfo* info);

  void translateKey(const CommissionProgramKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CONTRACTID", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionProgramDAO>;
  static DAOHelper<CommissionProgramDAO> _helper;
  CommissionProgramDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CommissionProgramKey, std::vector<CommissionProgramInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<CommissionProgramInfo*>* create(CommissionProgramKey key) override;
  void destroy(CommissionProgramKey key, std::vector<CommissionProgramInfo*>* recs) override;

private:
  static CommissionProgramDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, uint64_t, DateTime, DateTime> CommissionProgramHistoricalKey;

class CommissionProgramHistoricalDAO
    : public HistoricalDataAccessObject<CommissionProgramHistoricalKey,
                                        std::vector<CommissionProgramInfo*> >
{
public:
  static CommissionProgramHistoricalDAO& instance();

  const std::vector<CommissionProgramInfo*>& get(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 uint64_t contractId,
                                                 DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionProgramHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CONTRACTID", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CommissionProgramHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CONTRACTID", key._b);
  }

  void translateKey(const CommissionProgramHistoricalKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CONTRACTID", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(CommissionProgramHistoricalKey& key,
                       const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  CommissionProgramHistoricalKey createKey(const CommissionProgramInfo* info,
                                           const DateTime& startDate = DateTime::emptyDate(),
                                           const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionProgramHistoricalDAO>;
  static DAOHelper<CommissionProgramHistoricalDAO> _helper;

  CommissionProgramHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CommissionProgramHistoricalKey,
                                 std::vector<CommissionProgramInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<CommissionProgramInfo*>* create(CommissionProgramHistoricalKey key) override;
  void destroy(CommissionProgramHistoricalKey key,
               std::vector<CommissionProgramInfo*>* recs) override;

private:
  static CommissionProgramHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}// tse
