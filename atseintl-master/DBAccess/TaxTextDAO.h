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
class TaxText;
class DeleteList;

typedef HashKey<VendorCode, int> TaxTextKey;

class TaxTextDAO : public DataAccessObject<TaxTextKey, std::vector<TaxText*> >
{
public:
  static TaxTextDAO& instance();
  const TaxText*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxTextKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TaxTextKey createKey(TaxText* info);

  void translateKey(const TaxTextKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxText, TaxTextDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxText*>* vect) const override;

  virtual std::vector<TaxText*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxTextDAO>;
  static DAOHelper<TaxTextDAO> _helper;
  TaxTextDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxTextKey, std::vector<TaxText*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<TaxText*>* create(TaxTextKey key) override;
  void destroy(TaxTextKey key, std::vector<TaxText*>* t) override;

private:
  static TaxTextDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TaxTextHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TaxTextHistoricalKey;

class TaxTextHistoricalDAO
    : public HistoricalDataAccessObject<TaxTextHistoricalKey, std::vector<TaxText*> >
{
public:
  static TaxTextHistoricalDAO& instance();
  const TaxText*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxTextHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxTextHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(TaxTextHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxText*>* vect) const override;

  virtual std::vector<TaxText*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxTextHistoricalDAO>;
  static DAOHelper<TaxTextHistoricalDAO> _helper;
  TaxTextHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxTextHistoricalKey, std::vector<TaxText*> >(cacheSize, cacheType)
  {
  }
  std::vector<TaxText*>* create(TaxTextHistoricalKey key) override;
  void destroy(TaxTextHistoricalKey key, std::vector<TaxText*>* t) override;

private:
  static TaxTextHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

