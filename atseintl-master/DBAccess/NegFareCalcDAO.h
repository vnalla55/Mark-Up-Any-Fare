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
class NegFareCalcInfo;
class DeleteList;

typedef HashKey<VendorCode, int> NegFareCalcKey;

class NegFareCalcDAO : public DataAccessObject<NegFareCalcKey, std::vector<NegFareCalcInfo*> >
{
public:
  static NegFareCalcDAO& instance();
  const std::vector<NegFareCalcInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareCalcKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareCalcDAO>;
  static DAOHelper<NegFareCalcDAO> _helper;
  NegFareCalcDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NegFareCalcKey, std::vector<NegFareCalcInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<NegFareCalcInfo*>* create(NegFareCalcKey key) override;
  void destroy(NegFareCalcKey key, std::vector<NegFareCalcInfo*>* t) override;

private:
  static NegFareCalcDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: NegFareCalcHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> NegFareCalcHistoricalKey;

class NegFareCalcHistoricalDAO
    : public HistoricalDataAccessObject<NegFareCalcHistoricalKey, std::vector<NegFareCalcInfo*> >
{
public:
  static NegFareCalcHistoricalDAO& instance();
  const std::vector<NegFareCalcInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareCalcHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    NegFareCalcHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(NegFareCalcHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareCalcHistoricalDAO>;
  static DAOHelper<NegFareCalcHistoricalDAO> _helper;
  NegFareCalcHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NegFareCalcHistoricalKey, std::vector<NegFareCalcInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<NegFareCalcInfo*>* create(NegFareCalcHistoricalKey key) override;
  void destroy(NegFareCalcHistoricalKey key, std::vector<NegFareCalcInfo*>* t) override;

private:
  static NegFareCalcHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
