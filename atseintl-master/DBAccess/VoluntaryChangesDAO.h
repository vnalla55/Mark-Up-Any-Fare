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
class VoluntaryChangesInfo;
class DeleteList;

typedef HashKey<VendorCode, int> VoluntaryChangesKey;

class VoluntaryChangesDAO
    : public DataAccessObject<VoluntaryChangesKey, std::vector<VoluntaryChangesInfo*> >
{
public:
  static VoluntaryChangesDAO& instance();
  const VoluntaryChangesInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, VoluntaryChangesKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<VoluntaryChangesDAO>;
  static DAOHelper<VoluntaryChangesDAO> _helper;
  VoluntaryChangesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<VoluntaryChangesKey, std::vector<VoluntaryChangesInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<VoluntaryChangesInfo*>* create(VoluntaryChangesKey key) override;
  void destroy(VoluntaryChangesKey key, std::vector<VoluntaryChangesInfo*>* vc) override;

  virtual sfc::CompressedData*
  compress(const std::vector<VoluntaryChangesInfo*>* vect) const override;

  virtual std::vector<VoluntaryChangesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static VoluntaryChangesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: VoluntaryChangesHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> VoluntaryChangesHistoricalKey;

class VoluntaryChangesHistoricalDAO
    : public HistoricalDataAccessObject<VoluntaryChangesHistoricalKey,
                                        std::vector<VoluntaryChangesInfo*> >
{
public:
  static VoluntaryChangesHistoricalDAO& instance();
  const VoluntaryChangesInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, VoluntaryChangesHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    VoluntaryChangesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(VoluntaryChangesHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<VoluntaryChangesInfo*>* vect) const override;

  virtual std::vector<VoluntaryChangesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<VoluntaryChangesHistoricalDAO>;
  static DAOHelper<VoluntaryChangesHistoricalDAO> _helper;
  VoluntaryChangesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<VoluntaryChangesHistoricalKey,
                                 std::vector<VoluntaryChangesInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<VoluntaryChangesInfo*>* create(VoluntaryChangesHistoricalKey key) override;
  void destroy(VoluntaryChangesHistoricalKey key, std::vector<VoluntaryChangesInfo*>* vc) override;

private:
  static VoluntaryChangesHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
