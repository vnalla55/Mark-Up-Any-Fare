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
class AdvResTktInfo;
class DeleteList;

typedef HashKey<VendorCode, int> AdvResTktKey;

class AdvResTktDAO : public DataAccessObject<AdvResTktKey, std::vector<AdvResTktInfo*> >
{
public:
  static AdvResTktDAO& instance();

  const AdvResTktInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AdvResTktKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  AdvResTktKey createKey(AdvResTktInfo* info);

  void translateKey(const AdvResTktKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AdvResTktInfo, AdvResTktDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AdvResTktInfo*>* vect) const override;

  virtual std::vector<AdvResTktInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AdvResTktDAO>;
  static DAOHelper<AdvResTktDAO> _helper;
  AdvResTktDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AdvResTktKey, std::vector<AdvResTktInfo*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<AdvResTktInfo*>* create(AdvResTktKey key) override;
  void destroy(AdvResTktKey key, std::vector<AdvResTktInfo*>* t) override;

private:
  static AdvResTktDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: AdvResTktHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> AdvResTktHistoricalKey;

class AdvResTktHistoricalDAO
    : public HistoricalDataAccessObject<AdvResTktHistoricalKey, std::vector<AdvResTktInfo*> >
{
public:
  static AdvResTktHistoricalDAO& instance();
  const AdvResTktInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AdvResTktHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AdvResTktHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(AdvResTktHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AdvResTktInfo*>* vect) const override;

  virtual std::vector<AdvResTktInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AdvResTktHistoricalDAO>;
  static DAOHelper<AdvResTktHistoricalDAO> _helper;
  AdvResTktHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AdvResTktHistoricalKey, std::vector<AdvResTktInfo*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<AdvResTktInfo*>* create(AdvResTktHistoricalKey key) override;
  void destroy(AdvResTktHistoricalKey key, std::vector<AdvResTktInfo*>* t) override;

private:
  static AdvResTktHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

