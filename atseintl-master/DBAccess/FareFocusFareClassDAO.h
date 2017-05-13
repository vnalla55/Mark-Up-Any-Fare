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
class FareFocusFareClassInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusFareClassKey;

class FareFocusFareClassDAO
    : public DataAccessObject<FareFocusFareClassKey, std::vector<FareFocusFareClassInfo*> >
{
public:
  static FareFocusFareClassDAO& instance();

  const std::vector<FareFocusFareClassInfo*>&
  get(DeleteList& del,
      uint64_t fareClassItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusFareClassKey& key) const override
  {
    return key.initialized = objectKey.getValue("FARECLASSITEMNO", key._a);
  }

  FareFocusFareClassKey createKey(const FareFocusFareClassInfo* info);

  void translateKey(const FareFocusFareClassKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARECLASSITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
    compress(const std::vector<FareFocusFareClassInfo*>* vect) const override;

  virtual std::vector<FareFocusFareClassInfo*>*
    uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusFareClassDAO>;
  static DAOHelper<FareFocusFareClassDAO> _helper;
  FareFocusFareClassDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusFareClassKey, std::vector<FareFocusFareClassInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusFareClassInfo*>* create(FareFocusFareClassKey key) override;
  void destroy(FareFocusFareClassKey key, std::vector<FareFocusFareClassInfo*>* recs) override;

private:
  static FareFocusFareClassDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusFareClassHistoricalKey;

class FareFocusFareClassHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusFareClassHistoricalKey,
                                        std::vector<FareFocusFareClassInfo*> >
{
public:
  static FareFocusFareClassHistoricalDAO& instance();

  const std::vector<FareFocusFareClassInfo*>&
  get(DeleteList& del, uint64_t fareClassItemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusFareClassHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("FARECLASSITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusFareClassHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("FARECLASSITEMNO", key._a);
  }

  void translateKey(const FareFocusFareClassHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARECLASSITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusFareClassHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusFareClassHistoricalKey createKey(const FareFocusFareClassInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusFareClassHistoricalDAO>;
  static DAOHelper<FareFocusFareClassHistoricalDAO> _helper;

  FareFocusFareClassHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusFareClassHistoricalKey,
                                 std::vector<FareFocusFareClassInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusFareClassInfo*>* create(FareFocusFareClassHistoricalKey key) override;
  void
  destroy(FareFocusFareClassHistoricalKey key, std::vector<FareFocusFareClassInfo*>* recs) override;

  virtual sfc::CompressedData*
    compress(const std::vector<FareFocusFareClassInfo*>* vect) const override;

  virtual std::vector<FareFocusFareClassInfo*>*
    uncompress(const sfc::CompressedData& compressed) const override;

private:
  static FareFocusFareClassHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
