//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
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
class SvcFeesCxrResultingFCLInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesCxrResultingFCLKey;

class SvcFeesCxrResultingFCLDAO
    : public DataAccessObject<SvcFeesCxrResultingFCLKey, std::vector<SvcFeesCxrResultingFCLInfo*> >
{
public:
  static SvcFeesCxrResultingFCLDAO& instance();

  const std::vector<SvcFeesCxrResultingFCLInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesCxrResultingFCLKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesCxrResultingFCLKey createKey(SvcFeesCxrResultingFCLInfo* info);

  void translateKey(const SvcFeesCxrResultingFCLKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesCxrResultingFCLInfo, SvcFeesCxrResultingFCLDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesCxrResultingFCLDAO>;
  static DAOHelper<SvcFeesCxrResultingFCLDAO> _helper;
  SvcFeesCxrResultingFCLDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesCxrResultingFCLKey, std::vector<SvcFeesCxrResultingFCLInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesCxrResultingFCLInfo*>* create(SvcFeesCxrResultingFCLKey key) override;
  void
  destroy(SvcFeesCxrResultingFCLKey key, std::vector<SvcFeesCxrResultingFCLInfo*>* recs) override;

  void load() override;

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesCxrResultingFCLInfo*>* vect) const override;

  virtual std::vector<SvcFeesCxrResultingFCLInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static SvcFeesCxrResultingFCLDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesCxrResultingFCLHistoricalKey;

class SvcFeesCxrResultingFCLHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesCxrResultingFCLHistoricalKey,
                                        std::vector<SvcFeesCxrResultingFCLInfo*> >
{
public:
  static SvcFeesCxrResultingFCLHistoricalDAO& instance();

  const std::vector<SvcFeesCxrResultingFCLInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, SvcFeesCxrResultingFCLHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesCxrResultingFCLHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SvcFeesCxrResultingFCLHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesCxrResultingFCLInfo*>* vect) const override;

  virtual std::vector<SvcFeesCxrResultingFCLInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesCxrResultingFCLHistoricalDAO>;
  static DAOHelper<SvcFeesCxrResultingFCLHistoricalDAO> _helper;
  SvcFeesCxrResultingFCLHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesCxrResultingFCLHistoricalKey,
                                 std::vector<SvcFeesCxrResultingFCLInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesCxrResultingFCLInfo*>*
  create(SvcFeesCxrResultingFCLHistoricalKey key) override;
  void destroy(SvcFeesCxrResultingFCLHistoricalKey key,
               std::vector<SvcFeesCxrResultingFCLInfo*>* t) override;

private:
  static SvcFeesCxrResultingFCLHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
