//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------
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
class TransfersInfo1;
class DeleteList;

typedef HashKey<VendorCode, int> TransfersKey;

class Transfers1DAO : public DataAccessObject<TransfersKey, std::vector<TransfersInfo1*> >
{
public:
  static Transfers1DAO& instance();
  const TransfersInfo1*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TransfersKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TransfersKey createKey(TransfersInfo1* info);

  void translateKey(const TransfersKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TransfersInfo1, Transfers1DAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TransfersInfo1*>* vect) const override;

  virtual std::vector<TransfersInfo1*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<Transfers1DAO>;
  static DAOHelper<Transfers1DAO> _helper;
  Transfers1DAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TransfersKey, std::vector<TransfersInfo1*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<TransfersInfo1*>* create(TransfersKey key) override;
  void destroy(TransfersKey key, std::vector<TransfersInfo1*>* t) override;

private:
  static Transfers1DAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: Transfers1HistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TransfersHistoricalKey;

class Transfers1HistoricalDAO
    : public HistoricalDataAccessObject<TransfersHistoricalKey, std::vector<TransfersInfo1*> >
{
public:
  static Transfers1HistoricalDAO& instance();
  const TransfersInfo1*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TransfersHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TransfersHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(TransfersHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TransfersInfo1*>* vect) const override;

  virtual std::vector<TransfersInfo1*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<Transfers1HistoricalDAO>;
  static DAOHelper<Transfers1HistoricalDAO> _helper;
  Transfers1HistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TransfersHistoricalKey, std::vector<TransfersInfo1*> >(cacheSize,
                                                                                        cacheType)
  {
  }
  std::vector<TransfersInfo1*>* create(TransfersHistoricalKey key) override;
  void destroy(TransfersHistoricalKey key, std::vector<TransfersInfo1*>* t) override;

private:
  static Transfers1HistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
