//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class NUCInfo;
class DeleteList;

typedef HashKey<CurrencyCode> NUCAllCarriersKey;
class NUCAllCarriersDAO : public DataAccessObject<NUCAllCarriersKey, std::vector<NUCInfo*>, false>
{
public:
  static NUCAllCarriersDAO& instance();

  const std::vector<NUCInfo*>& get(DeleteList& del,
                                   const CurrencyCode& currency,
                                   const DateTime& from_date,
                                   const DateTime& to_date);

  bool translateKey(const ObjectKey& objectKey, NUCAllCarriersKey& key) const override
  {
    return key.initialized = objectKey.getValue("CUR", key._a);
  }

  NUCAllCarriersKey createKey(NUCInfo* info);

  void translateKey(const NUCAllCarriersKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CUR", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NUCInfo, NUCAllCarriersDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NUCAllCarriersDAO>;
  static DAOHelper<NUCAllCarriersDAO> _helper;

  NUCAllCarriersDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NUCAllCarriersKey, std::vector<NUCInfo*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<NUCInfo*>* create(NUCAllCarriersKey key) override;
  void destroy(NUCAllCarriersKey key, std::vector<NUCInfo*>* t) override;

private:
  struct isEffective;
  static NUCAllCarriersDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CurrencyCode> NUCAllCarriersHistoricalKey;
class NUCAllCarriersHistoricalDAO
    : public HistoricalDataAccessObject<NUCAllCarriersHistoricalKey, std::vector<NUCInfo*> >
{
public:
  static NUCAllCarriersHistoricalDAO& instance();

  const std::vector<NUCInfo*>& get(DeleteList& del,
                                   const CurrencyCode& currency,
                                   const DateTime& from_date,
                                   const DateTime& to_Date);

  bool translateKey(const ObjectKey& objectKey, NUCAllCarriersHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CUR", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NUCAllCarriersHistoricalDAO>;
  static DAOHelper<NUCAllCarriersHistoricalDAO> _helper;

  NUCAllCarriersHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NUCAllCarriersHistoricalKey, std::vector<NUCInfo*> >(cacheSize,
                                                                                      cacheType)
  {
  }

  std::vector<NUCInfo*>* create(NUCAllCarriersHistoricalKey key) override;
  void destroy(NUCAllCarriersHistoricalKey key, std::vector<NUCInfo*>* t) override;

private:
  static NUCAllCarriersHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
