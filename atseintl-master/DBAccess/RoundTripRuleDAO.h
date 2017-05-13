//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class RoundTripRuleItem;
class DeleteList;

typedef HashKey<VendorCode, int> RoundTripRuleKey;

class RoundTripRuleDAO : public DataAccessObject<RoundTripRuleKey, std::vector<RoundTripRuleItem*> >
{
public:
  static RoundTripRuleDAO& instance();
  RoundTripRuleItem*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RoundTripRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoundTripRuleDAO>;
  static DAOHelper<RoundTripRuleDAO> _helper;
  RoundTripRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<RoundTripRuleKey, std::vector<RoundTripRuleItem*> >(cacheSize, cacheType)
  {
  }
  std::vector<RoundTripRuleItem*>* create(RoundTripRuleKey key) override;
  void destroy(RoundTripRuleKey key, std::vector<RoundTripRuleItem*>* t) override;

private:
  static RoundTripRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: RoundTripRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> RoundTripRuleHistoricalKey;

class RoundTripRuleHistoricalDAO
    : public HistoricalDataAccessObject<RoundTripRuleHistoricalKey,
                                        std::vector<RoundTripRuleItem*> >
{
public:
  static RoundTripRuleHistoricalDAO& instance();
  RoundTripRuleItem*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RoundTripRuleHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    RoundTripRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(RoundTripRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoundTripRuleHistoricalDAO>;
  static DAOHelper<RoundTripRuleHistoricalDAO> _helper;
  RoundTripRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<RoundTripRuleHistoricalKey, std::vector<RoundTripRuleItem*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<RoundTripRuleItem*>* create(RoundTripRuleHistoricalKey key) override;
  void destroy(RoundTripRuleHistoricalKey key, std::vector<RoundTripRuleItem*>* t) override;

private:
  static RoundTripRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
