//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class SurfaceTransfersInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SurfaceTransfersKey;

class SurfaceTransfersDAO
    : public DataAccessObject<SurfaceTransfersKey, std::vector<SurfaceTransfersInfo*> >
{
public:
  static SurfaceTransfersDAO& instance();
  const std::vector<SurfaceTransfersInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurfaceTransfersKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurfaceTransfersDAO>;
  static DAOHelper<SurfaceTransfersDAO> _helper;
  SurfaceTransfersDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SurfaceTransfersKey, std::vector<SurfaceTransfersInfo*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  std::vector<SurfaceTransfersInfo*>* create(SurfaceTransfersKey key) override;
  void destroy(SurfaceTransfersKey key, std::vector<SurfaceTransfersInfo*>* t) override;

private:
  static SurfaceTransfersDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SurfaceTransfersHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> SurfaceTransfersHistoricalKey;

class SurfaceTransfersHistoricalDAO
    : public HistoricalDataAccessObject<SurfaceTransfersHistoricalKey,
                                        std::vector<SurfaceTransfersInfo*> >
{
public:
  static SurfaceTransfersHistoricalDAO& instance();
  const std::vector<SurfaceTransfersInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurfaceTransfersHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SurfaceTransfersHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SurfaceTransfersHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurfaceTransfersHistoricalDAO>;
  static DAOHelper<SurfaceTransfersHistoricalDAO> _helper;
  SurfaceTransfersHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SurfaceTransfersHistoricalKey,
                                 std::vector<SurfaceTransfersInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<SurfaceTransfersInfo*>* create(SurfaceTransfersHistoricalKey key) override;
  void destroy(SurfaceTransfersHistoricalKey key, std::vector<SurfaceTransfersInfo*>* t) override;

private:
  static SurfaceTransfersHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
