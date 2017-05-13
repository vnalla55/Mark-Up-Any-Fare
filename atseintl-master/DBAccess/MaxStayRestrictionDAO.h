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
class MaxStayRestriction;
class DeleteList;

typedef HashKey<VendorCode, int> MaxStayRestrictionKey;

class MaxStayRestrictionDAO
    : public DataAccessObject<MaxStayRestrictionKey, std::vector<MaxStayRestriction*> >
{
public:
  static MaxStayRestrictionDAO& instance();
  const MaxStayRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MaxStayRestrictionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  MaxStayRestrictionKey createKey(MaxStayRestriction* info);

  void translateKey(const MaxStayRestrictionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MaxStayRestriction, MaxStayRestrictionDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MaxStayRestriction*>* vect) const override;

  virtual std::vector<MaxStayRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MaxStayRestrictionDAO>;
  static DAOHelper<MaxStayRestrictionDAO> _helper;
  MaxStayRestrictionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MaxStayRestrictionKey, std::vector<MaxStayRestriction*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  void load() override;
  std::vector<MaxStayRestriction*>* create(MaxStayRestrictionKey key) override;
  void destroy(MaxStayRestrictionKey key, std::vector<MaxStayRestriction*>* t) override;

private:
  static MaxStayRestrictionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MaxStayRestrictionHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> MaxStayRestrictionHistoricalKey;

class MaxStayRestrictionHistoricalDAO
    : public HistoricalDataAccessObject<MaxStayRestrictionHistoricalKey,
                                        std::vector<MaxStayRestriction*> >
{
public:
  static MaxStayRestrictionHistoricalDAO& instance();
  const MaxStayRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MaxStayRestrictionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MaxStayRestrictionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(MaxStayRestrictionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MaxStayRestriction*>* vect) const override;

  virtual std::vector<MaxStayRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MaxStayRestrictionHistoricalDAO>;
  static DAOHelper<MaxStayRestrictionHistoricalDAO> _helper;
  MaxStayRestrictionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MaxStayRestrictionHistoricalKey,
                                 std::vector<MaxStayRestriction*> >(cacheSize, cacheType)
  {
  }
  std::vector<MaxStayRestriction*>* create(MaxStayRestrictionHistoricalKey key) override;
  void destroy(MaxStayRestrictionHistoricalKey key, std::vector<MaxStayRestriction*>* t) override;

private:
  static MaxStayRestrictionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
