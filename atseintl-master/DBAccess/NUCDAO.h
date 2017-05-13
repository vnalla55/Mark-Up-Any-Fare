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
class NUCInfo;
class DeleteList;

typedef HashKey<CurrencyCode, CarrierCode> NUCKey;

class NUCDAO : public DataAccessObject<NUCKey, std::vector<NUCInfo*>, false>
{
public:
  static NUCDAO& instance();

  const std::vector<NUCInfo*>& get(DeleteList& del,
                                   const CurrencyCode& currency,
                                   const CarrierCode& carrier,
                                   const DateTime& date,
                                   const DateTime& ticketDate);

  NUCInfo* getFirst(DeleteList& del,
                    const CurrencyCode& currency,
                    const CarrierCode& carrier,
                    const DateTime& date,
                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NUCKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CUR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  NUCKey createKey(NUCInfo* info);

  void translateKey(const NUCKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CUR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NUCInfo, NUCDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NUCDAO>;
  static DAOHelper<NUCDAO> _helper;
  NUCDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NUCKey, std::vector<NUCInfo*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;

  std::vector<NUCInfo*>* create(NUCKey key) override;

  void destroy(NUCKey key, std::vector<NUCInfo*>* t) override;

private:
  struct isEffective;
  static NUCDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CurrencyCode, CarrierCode, DateTime, DateTime> NUCHistoricalKey;

class NUCHistoricalDAO : public HistoricalDataAccessObject<NUCHistoricalKey, std::vector<NUCInfo*> >
{
public:
  static NUCHistoricalDAO& instance();

  const std::vector<NUCInfo*>& get(DeleteList& del,
                                   const CurrencyCode& currency,
                                   const CarrierCode& carrier,
                                   const DateTime& ticketDate,
                                   const DateTime& date);
  NUCInfo* getFirst(DeleteList& del,
                    const CurrencyCode& currency,
                    const CarrierCode& carrier,
                    const DateTime& date,
                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NUCHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CUR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool
  translateKey(const ObjectKey& objectKey, NUCHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("CUR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void setKeyDateRange(NUCHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<NUCHistoricalDAO>;

  static DAOHelper<NUCHistoricalDAO> _helper;

  NUCHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NUCHistoricalKey, std::vector<NUCInfo*> >(cacheSize, cacheType)
  {
  }

  std::vector<NUCInfo*>* create(NUCHistoricalKey key) override;

  void destroy(NUCHistoricalKey key, std::vector<NUCInfo*>* t) override;

private:
  static NUCHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
