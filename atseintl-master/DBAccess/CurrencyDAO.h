//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
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
class Currency;
class DeleteList;

class CurrencyDAO : public DataAccessObject<CurrencyKey, std::vector<Currency*>, false>
{
public:
  static CurrencyDAO& instance();

  const Currency*
  get(DeleteList& del, const CurrencyCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<CurrencyCode>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, CurrencyKey& key) const override
  {
    return key.initialized = objectKey.getValue("CURRENCYCODE", key._a);
  }

  CurrencyKey createKey(Currency* info);

  void translateKey(const CurrencyKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CURRENCYCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Currency, CurrencyDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CurrencyDAO>;

  static DAOHelper<CurrencyDAO> _helper;

  CurrencyDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CurrencyKey, std::vector<Currency*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Currency*>* create(CurrencyKey key) override;
  void destroy(CurrencyKey key, std::vector<Currency*>* recs) override;

  virtual void load() override;

  virtual sfc::CompressedData* compress(const std::vector<Currency*>* vect) const override;

  virtual std::vector<Currency*>* uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  static CurrencyDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CurrencyHistoricalDAO
// --------------------------------------------------
class CurrencyHistoricalDAO
    : public HistoricalDataAccessObject<CurrencyCode, std::vector<Currency*>, false>
{
public:
  static CurrencyHistoricalDAO& instance();

  const Currency*
  get(DeleteList& del, const CurrencyCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<CurrencyCode>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, CurrencyCode& key) const override
  {
    return objectKey.getValue("CURRENCYCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<Currency*>* vect) const override;

  virtual std::vector<Currency*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CurrencyHistoricalDAO>;

  static DAOHelper<CurrencyHistoricalDAO> _helper;

  CurrencyHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CurrencyCode, std::vector<Currency*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Currency*>* create(CurrencyCode key) override;
  void destroy(CurrencyCode key, std::vector<Currency*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CurrencyHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
