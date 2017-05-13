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
class EndOnEnd;
class DeleteList;

typedef HashKey<VendorCode, int> EndOnEndKey;

class EndOnEndDAO : public DataAccessObject<EndOnEndKey, std::vector<const EndOnEnd*> >
{
public:
  static EndOnEndDAO& instance();
  const EndOnEnd*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, EndOnEndKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  EndOnEndKey createKey(const EndOnEnd* info);

  void translateKey(const EndOnEndKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<EndOnEnd, EndOnEndDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<EndOnEndDAO>;
  static DAOHelper<EndOnEndDAO> _helper;
  EndOnEndDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<EndOnEndKey, std::vector<const EndOnEnd*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<const EndOnEnd*>* create(EndOnEndKey key) override;
  void destroy(EndOnEndKey key, std::vector<const EndOnEnd*>* recs) override;

private:
  static EndOnEndDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: EndOnEndHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> EndOnEndHistoricalKey;

class EndOnEndHistoricalDAO
    : public HistoricalDataAccessObject<EndOnEndHistoricalKey, std::vector<const EndOnEnd*> >
{
public:
  static EndOnEndHistoricalDAO& instance();
  const EndOnEnd*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, EndOnEndHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    EndOnEndHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(EndOnEndHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<EndOnEndHistoricalDAO>;
  static DAOHelper<EndOnEndHistoricalDAO> _helper;
  EndOnEndHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<EndOnEndHistoricalKey, std::vector<const EndOnEnd*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<const EndOnEnd*>* create(EndOnEndHistoricalKey key) override;
  void destroy(EndOnEndHistoricalKey key, std::vector<const EndOnEnd*>* recs) override;

private:
  static EndOnEndHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
