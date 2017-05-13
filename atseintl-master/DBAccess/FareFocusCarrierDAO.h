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
class FareFocusCarrierInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusCarrierKey;

class FareFocusCarrierDAO
    : public DataAccessObject<FareFocusCarrierKey, std::vector<FareFocusCarrierInfo*> >
{
public:
  static FareFocusCarrierDAO& instance();

  const std::vector<FareFocusCarrierInfo*>&
  get(DeleteList& del,
      uint64_t carrierItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusCarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIERITEMNO", key._a);
  }

  FareFocusCarrierKey createKey(const FareFocusCarrierInfo* info);

  void translateKey(const FareFocusCarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIERITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusCarrierDAO>;
  static DAOHelper<FareFocusCarrierDAO> _helper;
  FareFocusCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusCarrierKey, std::vector<FareFocusCarrierInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusCarrierInfo*>* create(FareFocusCarrierKey key) override;
  void destroy(FareFocusCarrierKey key, std::vector<FareFocusCarrierInfo*>* recs) override;

private:
  static FareFocusCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusCarrierHistoricalKey;

class FareFocusCarrierHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusCarrierHistoricalKey,
                                        std::vector<FareFocusCarrierInfo*> >
{
public:
  static FareFocusCarrierHistoricalDAO& instance();

  const std::vector<FareFocusCarrierInfo*>&
  get(DeleteList& del, uint64_t carrierItemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusCarrierHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("CARRIERITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusCarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIERITEMNO", key._a);
  }

  void translateKey(const FareFocusCarrierHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIERITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusCarrierHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusCarrierHistoricalKey createKey(const FareFocusCarrierInfo* info,
                                          const DateTime& startDate = DateTime::emptyDate(),
                                          const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusCarrierHistoricalDAO>;
  static DAOHelper<FareFocusCarrierHistoricalDAO> _helper;

  FareFocusCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusCarrierHistoricalKey,
                                 std::vector<FareFocusCarrierInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusCarrierInfo*>* create(FareFocusCarrierHistoricalKey key) override;
  void
  destroy(FareFocusCarrierHistoricalKey key, std::vector<FareFocusCarrierInfo*>* recs) override;

private:
  static FareFocusCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse


