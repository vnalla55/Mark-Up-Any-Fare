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
class FareFocusDisplayCatTypeInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusDisplayCatTypeKey;

class FareFocusDisplayCatTypeDAO
    : public DataAccessObject<FareFocusDisplayCatTypeKey, std::vector<FareFocusDisplayCatTypeInfo*> >
{
public:
  static FareFocusDisplayCatTypeDAO& instance();

  const FareFocusDisplayCatTypeInfo*
  get(DeleteList& del,
      uint64_t displayCatTypeItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusDisplayCatTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("DISPLAYCATTYPEITEMNO", key._a);
  }

  FareFocusDisplayCatTypeKey createKey(const FareFocusDisplayCatTypeInfo* info);

  void translateKey(const FareFocusDisplayCatTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DISPLAYCATTYPEITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusDisplayCatTypeDAO>;
  static DAOHelper<FareFocusDisplayCatTypeDAO> _helper;
  FareFocusDisplayCatTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusDisplayCatTypeKey, std::vector<FareFocusDisplayCatTypeInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<FareFocusDisplayCatTypeInfo*>* create(FareFocusDisplayCatTypeKey key) override;
  void destroy(FareFocusDisplayCatTypeKey key, std::vector<FareFocusDisplayCatTypeInfo*>* recs) override;

private:
  static FareFocusDisplayCatTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusDisplayCatTypeHistoricalKey;

class FareFocusDisplayCatTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusDisplayCatTypeHistoricalKey,
                                        std::vector<FareFocusDisplayCatTypeInfo*> >
{
public:
  static FareFocusDisplayCatTypeHistoricalDAO& instance();

  const FareFocusDisplayCatTypeInfo*
  get(DeleteList& del, uint64_t displayCatTypeItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusDisplayCatTypeHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("DISPLAYCATTYPEITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusDisplayCatTypeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("DISPLAYCATTYPEITEMNO", key._a);
  }

  void translateKey(const FareFocusDisplayCatTypeHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DISPLAYCATTYPEITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusDisplayCatTypeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusDisplayCatTypeHistoricalKey createKey(const FareFocusDisplayCatTypeInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusDisplayCatTypeHistoricalDAO>;
  static DAOHelper<FareFocusDisplayCatTypeHistoricalDAO> _helper;

  FareFocusDisplayCatTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusDisplayCatTypeHistoricalKey,
                                 std::vector<FareFocusDisplayCatTypeInfo*> >(cacheSize, cacheType,2)
  {
  }
  std::vector<FareFocusDisplayCatTypeInfo*>* create(FareFocusDisplayCatTypeHistoricalKey key) override;
  void
  destroy(FareFocusDisplayCatTypeHistoricalKey key, std::vector<FareFocusDisplayCatTypeInfo*>* recs) override;

private:
  static FareFocusDisplayCatTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
