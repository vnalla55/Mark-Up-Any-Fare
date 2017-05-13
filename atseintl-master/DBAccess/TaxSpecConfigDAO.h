#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxSpecConfigReg;
class DeleteList;

typedef HashKey<TaxSpecConfigName> TaxSpecConfigKey;

class TaxSpecConfigDAO
    : public DataAccessObject<TaxSpecConfigKey, std::vector<TaxSpecConfigReg*>, false>
{
public:
  static TaxSpecConfigDAO& instance();
  std::vector<TaxSpecConfigReg*>&
  get(DeleteList& del, const TaxSpecConfigName& name, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxSpecConfigKey& key) const override
  {
    return key.initialized = objectKey.getValue("SPECCONFIGNAME", key._a);
  }

  TaxSpecConfigKey createKey(TaxSpecConfigReg* info);

  void translateKey(const TaxSpecConfigKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SPECCONFIGNAME", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxSpecConfigReg, TaxSpecConfigDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxSpecConfigDAO>;
  static DAOHelper<TaxSpecConfigDAO> _helper;
  TaxSpecConfigDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxSpecConfigKey, std::vector<TaxSpecConfigReg*>, false>(
          cacheSize, cacheType, 3)
  {
  }
  virtual void load() override;
  std::vector<TaxSpecConfigReg*>* create(TaxSpecConfigKey key) override;
  void destroy(TaxSpecConfigKey key, std::vector<TaxSpecConfigReg*>* t) override;

private:
  static TaxSpecConfigDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TaxSpecConfigHistoricalDAO
// --------------------------------------------------

typedef HashKey<TaxSpecConfigName, DateTime, DateTime> TaxSpecConfigHistoricalKey;

class TaxSpecConfigHistoricalDAO : public HistoricalDataAccessObject<TaxSpecConfigHistoricalKey,
                                                                     std::vector<TaxSpecConfigReg*>,
                                                                     false>
{
public:
  static TaxSpecConfigHistoricalDAO& instance();

  std::vector<TaxSpecConfigReg*>&
  get(DeleteList& del, const TaxSpecConfigName& name, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxSpecConfigHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("SPECCONFIGNAME", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxSpecConfigHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("SPECCONFIGNAME", key._a);
  }

  void setKeyDateRange(TaxSpecConfigHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxSpecConfigHistoricalDAO>;
  static DAOHelper<TaxSpecConfigHistoricalDAO> _helper;
  TaxSpecConfigHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxSpecConfigHistoricalKey, std::vector<TaxSpecConfigReg*>, false>(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<TaxSpecConfigReg*>* create(TaxSpecConfigHistoricalKey key) override;
  void destroy(TaxSpecConfigHistoricalKey key, std::vector<TaxSpecConfigReg*>* t) override;

private:
  static TaxSpecConfigHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
