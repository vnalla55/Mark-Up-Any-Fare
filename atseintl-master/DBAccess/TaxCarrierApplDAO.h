//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
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
class TaxCarrierAppl;
class DeleteList;

typedef HashKey<VendorCode, int> TaxCarrierApplKey;

class TaxCarrierApplDAO : public DataAccessObject<TaxCarrierApplKey, std::vector<TaxCarrierAppl*> >
{
public:
  static TaxCarrierApplDAO& instance();
  const TaxCarrierAppl*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCarrierApplKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TaxCarrierApplKey createKey(TaxCarrierAppl* info);

  void translateKey(const TaxCarrierApplKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxCarrierAppl, TaxCarrierApplDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxCarrierAppl*>* vect) const override;

  virtual std::vector<TaxCarrierAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCarrierApplDAO>;
  static DAOHelper<TaxCarrierApplDAO> _helper;
  TaxCarrierApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxCarrierApplKey, std::vector<TaxCarrierAppl*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<TaxCarrierAppl*>* create(TaxCarrierApplKey key) override;
  void destroy(TaxCarrierApplKey key, std::vector<TaxCarrierAppl*>* t) override;

private:
  static TaxCarrierApplDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TaxCarrierApplHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TaxCarrierApplHistoricalKey;

class TaxCarrierApplHistoricalDAO
    : public HistoricalDataAccessObject<TaxCarrierApplHistoricalKey, std::vector<TaxCarrierAppl*> >
{
public:
  static TaxCarrierApplHistoricalDAO& instance();
  const TaxCarrierAppl*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCarrierApplHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxCarrierApplHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(TaxCarrierApplHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCarrierApplHistoricalDAO>;
  static DAOHelper<TaxCarrierApplHistoricalDAO> _helper;
  TaxCarrierApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxCarrierApplHistoricalKey, std::vector<TaxCarrierAppl*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<TaxCarrierAppl*>* create(TaxCarrierApplHistoricalKey key) override;
  void destroy(TaxCarrierApplHistoricalKey key, std::vector<TaxCarrierAppl*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<TaxCarrierAppl*>* vect) const override;

  virtual std::vector<TaxCarrierAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static TaxCarrierApplHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

