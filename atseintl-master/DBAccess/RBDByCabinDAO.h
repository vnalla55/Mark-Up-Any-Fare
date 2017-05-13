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
class RBDByCabinInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode> RBDByCabinKey;

class RBDByCabinDAO
  : public DataAccessObject<RBDByCabinKey, std::vector<RBDByCabinInfo*> >
{
public:
  static RBDByCabinDAO& instance();

  const std::vector<RBDByCabinInfo*>&
  get(DeleteList& del,
      const VendorCode& vendor,
      const CarrierCode& carrier,
      DateTime tvlDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, RBDByCabinKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b);
  }

  RBDByCabinKey createKey(const RBDByCabinInfo* info);

  void translateKey(const RBDByCabinKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  virtual void load() override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RBDByCabinDAO>;
  static DAOHelper<RBDByCabinDAO> _helper;
  RBDByCabinDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<RBDByCabinKey, std::vector<RBDByCabinInfo*> >(
          cacheSize, cacheType, 2)
  {
  }

  virtual std::vector<RBDByCabinInfo*>* create(RBDByCabinKey key) override;
  void destroy(RBDByCabinKey key, std::vector<RBDByCabinInfo*>* recs) override;

private:
  static RBDByCabinDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, CarrierCode, DateTime, DateTime> RBDByCabinHistoricalKey;

class RBDByCabinHistoricalDAO
  : public HistoricalDataAccessObject<RBDByCabinHistoricalKey,
                                      std::vector<RBDByCabinInfo*> >
{
public:
  static RBDByCabinHistoricalDAO& instance();

  const std::vector<RBDByCabinInfo*>&
  get(DeleteList& del,
      const VendorCode& vendor,
      const CarrierCode& carrier,
      DateTime tvlDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, RBDByCabinHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    RBDByCabinHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const RBDByCabinHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(RBDByCabinHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  RBDByCabinHistoricalKey createKey(const RBDByCabinInfo* info,
                                    const DateTime& startDate = DateTime::emptyDate(),
                                    const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RBDByCabinHistoricalDAO>;
  static DAOHelper<RBDByCabinHistoricalDAO> _helper;

  RBDByCabinHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<RBDByCabinHistoricalKey,
                                 std::vector<RBDByCabinInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<RBDByCabinInfo*>* create(RBDByCabinHistoricalKey key) override;
  void destroy(RBDByCabinHistoricalKey key, std::vector<RBDByCabinInfo*>* recs) override;

private:
  static RBDByCabinHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
