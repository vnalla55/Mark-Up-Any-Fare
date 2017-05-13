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
class MinStayRestriction;
class DeleteList;

typedef HashKey<VendorCode, int> MinStayRestrictionKey;

class MinStayRestrictionDAO
    : public DataAccessObject<MinStayRestrictionKey, std::vector<MinStayRestriction*> >
{
public:
  static MinStayRestrictionDAO& instance();
  const MinStayRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MinStayRestrictionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  MinStayRestrictionKey createKey(MinStayRestriction* info);

  void translateKey(const MinStayRestrictionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MinStayRestriction, MinStayRestrictionDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinStayRestriction*>* vect) const override;

  virtual std::vector<MinStayRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinStayRestrictionDAO>;
  static DAOHelper<MinStayRestrictionDAO> _helper;
  MinStayRestrictionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MinStayRestrictionKey, std::vector<MinStayRestriction*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  void load() override;
  std::vector<MinStayRestriction*>* create(MinStayRestrictionKey key) override;
  void destroy(MinStayRestrictionKey key, std::vector<MinStayRestriction*>* t) override;

private:
  static MinStayRestrictionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MinStayRestrictionHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> MinStayRestrictionHistoricalKey;

class MinStayRestrictionHistoricalDAO
    : public HistoricalDataAccessObject<MinStayRestrictionHistoricalKey,
                                        std::vector<MinStayRestriction*> >
{
public:
  static MinStayRestrictionHistoricalDAO& instance();
  const MinStayRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MinStayRestrictionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MinStayRestrictionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(MinStayRestrictionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinStayRestriction*>* vect) const override;

  virtual std::vector<MinStayRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinStayRestrictionHistoricalDAO>;
  static DAOHelper<MinStayRestrictionHistoricalDAO> _helper;
  MinStayRestrictionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MinStayRestrictionHistoricalKey,
                                 std::vector<MinStayRestriction*> >(cacheSize, cacheType)
  {
  }
  std::vector<MinStayRestriction*>* create(MinStayRestrictionHistoricalKey key) override;
  void destroy(MinStayRestrictionHistoricalKey key, std::vector<MinStayRestriction*>* t) override;

private:
  static MinStayRestrictionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
