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
class VisitAnotherCountry;
class DeleteList;

typedef HashKey<VendorCode, int> VisitAnotherCountryKey;

class VisitAnotherCountryDAO
    : public DataAccessObject<VisitAnotherCountryKey, std::vector<VisitAnotherCountry*> >
{
public:
  static VisitAnotherCountryDAO& instance();
  const VisitAnotherCountry*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, VisitAnotherCountryKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<VisitAnotherCountryDAO>;
  static DAOHelper<VisitAnotherCountryDAO> _helper;
  VisitAnotherCountryDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<VisitAnotherCountryKey, std::vector<VisitAnotherCountry*> >(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<VisitAnotherCountry*>* create(VisitAnotherCountryKey key) override;
  void destroy(VisitAnotherCountryKey key, std::vector<VisitAnotherCountry*>* t) override;

private:
  static VisitAnotherCountryDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: VisitAnotherCountryHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> VisitAnotherCountryHistoricalKey;

class VisitAnotherCountryHistoricalDAO
    : public HistoricalDataAccessObject<VisitAnotherCountryHistoricalKey,
                                        std::vector<VisitAnotherCountry*> >
{
public:
  static VisitAnotherCountryHistoricalDAO& instance();
  const VisitAnotherCountry*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, VisitAnotherCountryHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    VisitAnotherCountryHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(VisitAnotherCountryHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<VisitAnotherCountryHistoricalDAO>;
  static DAOHelper<VisitAnotherCountryHistoricalDAO> _helper;
  VisitAnotherCountryHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<VisitAnotherCountryHistoricalKey,
                                 std::vector<VisitAnotherCountry*> >(cacheSize, cacheType)
  {
  }
  std::vector<VisitAnotherCountry*>* create(VisitAnotherCountryHistoricalKey key) override;
  void destroy(VisitAnotherCountryHistoricalKey key, std::vector<VisitAnotherCountry*>* t) override;

private:
  static VisitAnotherCountryHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
