//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

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

class VoluntaryRefundsInfo;
class DeleteList;

typedef HashKey<VendorCode, int> VoluntaryRefundsKey;

class VoluntaryRefundsDAO
    : public DataAccessObject<VoluntaryRefundsKey, std::vector<VoluntaryRefundsInfo*> >
{
public:
  static VoluntaryRefundsDAO& instance();

  const VoluntaryRefundsInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, VoluntaryRefundsKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b));
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<VoluntaryRefundsDAO>;

  static DAOHelper<VoluntaryRefundsDAO> _helper;

  VoluntaryRefundsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<VoluntaryRefundsKey, std::vector<VoluntaryRefundsInfo*> >(
          cacheSize, cacheType, 3)
  {
  }

  std::vector<VoluntaryRefundsInfo*>* create(VoluntaryRefundsKey key) override;
  void destroy(VoluntaryRefundsKey key, std::vector<VoluntaryRefundsInfo*>* vr) override;

private:
  static VoluntaryRefundsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: VoluntaryRefundsHistoricalDAO
// --------------------------------------------------

typedef HashKey<VendorCode, int, DateTime, DateTime> VoluntaryRefundsHistoricalKey;

class VoluntaryRefundsHistoricalDAO
    : public HistoricalDataAccessObject<VoluntaryRefundsHistoricalKey,
                                        std::vector<VoluntaryRefundsInfo*> >
{
public:
  static VoluntaryRefundsHistoricalDAO& instance();

  const VoluntaryRefundsInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, VoluntaryRefundsHistoricalKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
                objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d));
  }

  bool translateKey(const ObjectKey& objectKey,
                    VoluntaryRefundsHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b));
  }

  void setKeyDateRange(VoluntaryRefundsHistoricalKey& key, const DateTime& ticketDate) const
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<VoluntaryRefundsHistoricalDAO>;

  static DAOHelper<VoluntaryRefundsHistoricalDAO> _helper;

  VoluntaryRefundsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<VoluntaryRefundsHistoricalKey,
                                 std::vector<VoluntaryRefundsInfo*> >(cacheSize, cacheType, 3)
  {
  }

  std::vector<VoluntaryRefundsInfo*>* create(VoluntaryRefundsHistoricalKey key) override;
  void destroy(VoluntaryRefundsHistoricalKey key, std::vector<VoluntaryRefundsInfo*>* vr) override;

private:
  static VoluntaryRefundsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

