//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
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
class SvcFeesTktDesignatorInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesTicketDesignatorKey;

class SvcFeesTicketDesignatorDAO
    : public DataAccessObject<SvcFeesTicketDesignatorKey, std::vector<SvcFeesTktDesignatorInfo*> >
{
public:
  static SvcFeesTicketDesignatorDAO& instance();

  const std::vector<SvcFeesTktDesignatorInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesTicketDesignatorKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesTicketDesignatorKey createKey(SvcFeesTktDesignatorInfo* info);

  void translateKey(const SvcFeesTicketDesignatorKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesTktDesignatorInfo, SvcFeesTicketDesignatorDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesTicketDesignatorDAO>;
  static DAOHelper<SvcFeesTicketDesignatorDAO> _helper;
  SvcFeesTicketDesignatorDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesTicketDesignatorKey, std::vector<SvcFeesTktDesignatorInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesTktDesignatorInfo*>* create(SvcFeesTicketDesignatorKey key) override;
  void
  destroy(SvcFeesTicketDesignatorKey key, std::vector<SvcFeesTktDesignatorInfo*>* recs) override;

  void load() override;

private:
  static SvcFeesTicketDesignatorDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesTicketDesignatorHistoricalKey;

class SvcFeesTicketDesignatorHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesTicketDesignatorHistoricalKey,
                                        std::vector<SvcFeesTktDesignatorInfo*> >
{
public:
  static SvcFeesTicketDesignatorHistoricalDAO& instance();

  const std::vector<SvcFeesTktDesignatorInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, SvcFeesTicketDesignatorHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesTicketDesignatorHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SvcFeesTicketDesignatorHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesTicketDesignatorHistoricalDAO>;
  static DAOHelper<SvcFeesTicketDesignatorHistoricalDAO> _helper;
  SvcFeesTicketDesignatorHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesTicketDesignatorHistoricalKey,
                                 std::vector<SvcFeesTktDesignatorInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesTktDesignatorInfo*>* create(SvcFeesTicketDesignatorHistoricalKey key) override;
  void destroy(SvcFeesTicketDesignatorHistoricalKey key,
               std::vector<SvcFeesTktDesignatorInfo*>* t) override;

private:
  static SvcFeesTicketDesignatorHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
