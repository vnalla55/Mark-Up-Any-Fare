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
class ServicesDescription;

typedef HashKey<ServiceGroupDescription> ServicesDescriptionKey;
typedef HashKey<ServiceGroupDescription, DateTime, DateTime> ServicesDescriptionHistoricalKey;

class ServicesDescriptionDAO
    : public DataAccessObject<ServicesDescriptionKey, std::vector<ServicesDescription*> >
{
public:
  friend class DAOHelper<ServicesDescriptionDAO>;

  static ServicesDescriptionDAO& instance();

  const ServicesDescription*
  get(DeleteList& del, const ServiceGroupDescription& value, const DateTime& ticketDate);

  ServicesDescriptionKey createKey(ServicesDescription* info);

  bool translateKey(const ObjectKey& objectKey, ServicesDescriptionKey& key) const override
  {
    return key.initialized = objectKey.getValue("VALUE", key._a);
  }

  void translateKey(const ServicesDescriptionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VALUE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ServicesDescription, ServicesDescriptionDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  ServicesDescriptionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ServicesDescriptionKey, std::vector<ServicesDescription*> >(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<ServicesDescription*>* create(ServicesDescriptionKey key) override;
  void destroy(ServicesDescriptionKey key, std::vector<ServicesDescription*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<ServicesDescriptionDAO> _helper;
  static ServicesDescriptionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class ServicesDescriptionHistoricalDAO
    : public HistoricalDataAccessObject<ServicesDescriptionHistoricalKey,
                                        std::vector<ServicesDescription*> >
{
public:
  friend class DAOHelper<ServicesDescriptionHistoricalDAO>;

  static ServicesDescriptionHistoricalDAO& instance();

  const ServicesDescription*
  get(DeleteList& del, const ServiceGroupDescription& carrier, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, ServicesDescriptionHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("VALUE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    ServicesDescriptionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);

    return key.initialized = objectKey.getValue("VALUE", key._a);
  }

  void
  setKeyDateRange(ServicesDescriptionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  ServicesDescriptionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ServicesDescriptionHistoricalKey,
                                 std::vector<ServicesDescription*> >(cacheSize, cacheType)
  {
  }
  std::vector<ServicesDescription*>* create(ServicesDescriptionHistoricalKey key) override;
  void destroy(ServicesDescriptionHistoricalKey key, std::vector<ServicesDescription*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<ServicesDescriptionHistoricalDAO> _helper;
  static ServicesDescriptionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

