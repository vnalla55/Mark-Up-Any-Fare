//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/ChildCacheNotifier.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/TaxCarrierFlightInfo.h"

namespace tse
{
typedef HashKey<VendorCode, int> TaxCarrierFlightKey;

class TaxCarrierFlightDAO
    : public DataAccessObject<TaxCarrierFlightKey, std::vector<TaxCarrierFlightInfo*> >,
      public ChildCacheNotifier<TaxCarrierFlightKey>
{
public:
  static TaxCarrierFlightDAO& instance();
  const TaxCarrierFlightInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCarrierFlightKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TaxCarrierFlightKey createKey(TaxCarrierFlightInfo* info);

  void translateKey(const TaxCarrierFlightKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxCarrierFlightInfo, TaxCarrierFlightDAO>(flatKey, objectKey)
        .success();
  }

  Logger& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCarrierFlightDAO>;
  static DAOHelper<TaxCarrierFlightDAO> _helper;
  TaxCarrierFlightDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxCarrierFlightKey, std::vector<TaxCarrierFlightInfo*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  void load() override;
  std::vector<TaxCarrierFlightInfo*>* create(TaxCarrierFlightKey key) override;
  void destroy(TaxCarrierFlightKey key, std::vector<TaxCarrierFlightInfo*>* t) override;
  virtual size_t clear() override;

private:
  static TaxCarrierFlightDAO* _instance;
  static Logger _logger;
};

// --------------------------------------------------
// Historical DAO: TaxCarrierFlightHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TaxCarrierFlightHistoricalKey;

class TaxCarrierFlightHistoricalDAO
    : public HistoricalDataAccessObject<TaxCarrierFlightHistoricalKey,
                                        std::vector<TaxCarrierFlightInfo*> >
{
public:
  static TaxCarrierFlightHistoricalDAO& instance();
  const TaxCarrierFlightInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCarrierFlightHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxCarrierFlightHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(TaxCarrierFlightHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCarrierFlightHistoricalDAO>;
  static DAOHelper<TaxCarrierFlightHistoricalDAO> _helper;
  TaxCarrierFlightHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxCarrierFlightHistoricalKey,
                                 std::vector<TaxCarrierFlightInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<TaxCarrierFlightInfo*>* create(TaxCarrierFlightHistoricalKey key) override;
  void destroy(TaxCarrierFlightHistoricalKey key, std::vector<TaxCarrierFlightInfo*>* t) override;

private:
  static TaxCarrierFlightHistoricalDAO* _instance;
  static Logger _logger;
};
} // namespace tse

