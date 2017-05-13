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
class Tours;
class DeleteList;

typedef HashKey<VendorCode, int> ToursKey;

class ToursDAO : public DataAccessObject<ToursKey, std::vector<Tours*> >
{
public:
  static ToursDAO& instance();
  const Tours*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ToursKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ToursDAO>;
  static DAOHelper<ToursDAO> _helper;
  ToursDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ToursKey, std::vector<Tours*> >(cacheSize, cacheType)
  {
  }
  std::vector<Tours*>* create(ToursKey key) override;
  void destroy(ToursKey key, std::vector<Tours*>* t) override;

private:
  static ToursDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: ToursHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> ToursHistoricalKey;

class ToursHistoricalDAO
    : public HistoricalDataAccessObject<ToursHistoricalKey, std::vector<Tours*> >
{
public:
  static ToursHistoricalDAO& instance();
  const Tours*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ToursHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool
  translateKey(const ObjectKey& objectKey, ToursHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(ToursHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ToursHistoricalDAO>;
  static DAOHelper<ToursHistoricalDAO> _helper;
  ToursHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ToursHistoricalKey, std::vector<Tours*> >(cacheSize, cacheType)
  {
  }
  std::vector<Tours*>* create(ToursHistoricalKey key) override;
  void destroy(ToursHistoricalKey key, std::vector<Tours*>* t) override;

private:
  static ToursHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
