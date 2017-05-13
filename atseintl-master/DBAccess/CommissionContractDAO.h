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
class CommissionContractInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, PseudoCityCode> CommissionContractKey;

class CommissionContractDAO
    : public DataAccessObject<CommissionContractKey, std::vector<CommissionContractInfo*> >
{
public:
  static CommissionContractDAO& instance();

  const std::vector<CommissionContractInfo*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const PseudoCityCode& pcc,
                                                  DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionContractKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("PCC", key._c);
  }

  CommissionContractKey createKey(const CommissionContractInfo* info);

  void translateKey(const CommissionContractKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("PCC", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionContractDAO>;
  static DAOHelper<CommissionContractDAO> _helper;
  CommissionContractDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CommissionContractKey, std::vector<CommissionContractInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<CommissionContractInfo*>* create(CommissionContractKey key) override;
  void destroy(CommissionContractKey key, std::vector<CommissionContractInfo*>* recs) override;

private:
  static CommissionContractDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, CarrierCode, PseudoCityCode, DateTime, DateTime> CommissionContractHistoricalKey;

class CommissionContractHistoricalDAO
    : public HistoricalDataAccessObject<CommissionContractHistoricalKey,
                                        std::vector<CommissionContractInfo*> >
{
public:
  static CommissionContractHistoricalDAO& instance();

  const std::vector<CommissionContractInfo*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const PseudoCityCode& pcc,
                                                  DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionContractHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("PCC", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CommissionContractHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("PCC", key._c);
  }

  void translateKey(const CommissionContractHistoricalKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("PCC", key._c);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  void setKeyDateRange(CommissionContractHistoricalKey& key,
                       const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  CommissionContractHistoricalKey createKey(const CommissionContractInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionContractHistoricalDAO>;
  static DAOHelper<CommissionContractHistoricalDAO> _helper;

  CommissionContractHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CommissionContractHistoricalKey,
                                 std::vector<CommissionContractInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<CommissionContractInfo*>* create(CommissionContractHistoricalKey key) override;
  void destroy(CommissionContractHistoricalKey key,
               std::vector<CommissionContractInfo*>* recs) override;

private:
  static CommissionContractHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}// tse
