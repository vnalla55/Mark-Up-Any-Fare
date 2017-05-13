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
class MinFareAppl;
class DeleteList;

typedef HashKey<VendorCode, int, CarrierCode> MinFareApplKey;

class MinFareApplDAO : public DataAccessObject<MinFareApplKey, std::vector<MinFareAppl*> >
{
public:
  static MinFareApplDAO& instance();

  const std::vector<MinFareAppl*>& get(DeleteList& del,
                                       const VendorCode& textTblVendor,
                                       int textTblItemNo,
                                       const CarrierCode& governingCarrier,
                                       const DateTime& date,
                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MinFareApplKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEXTTBLVENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("GOVERNINGCARRIER", key._c);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MinFareAppl*>* vect) const override;

  virtual std::vector<MinFareAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareApplDAO>;
  static DAOHelper<MinFareApplDAO> _helper;
  MinFareApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MinFareApplKey, std::vector<MinFareAppl*> >(cacheSize, cacheType, 3)
  {
  }
  std::vector<MinFareAppl*>* create(MinFareApplKey key) override;
  void destroy(MinFareApplKey key, std::vector<MinFareAppl*>* t) override;

private:
  static MinFareApplDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class MinFareApplDAO

// Historical Stuff
// /////////////////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, int, CarrierCode, DateTime, DateTime> MinFareApplHistoricalKey;

class MinFareApplHistoricalDAO
    : public HistoricalDataAccessObject<MinFareApplHistoricalKey, std::vector<MinFareAppl*> >
{
public:
  static MinFareApplHistoricalDAO& instance();

  const std::vector<MinFareAppl*>& get(DeleteList& del,
                                       const VendorCode& textTblVendor,
                                       int textTblItemNo,
                                       const CarrierCode& governingCarrier,
                                       const DateTime& date,
                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MinFareApplHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEXTTBLVENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("GOVERNINGCARRIER", key._c) &&
                             objectKey.getValue("STARTDATE", key._d) &&
                             objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MinFareApplHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("TEXTTBLVENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("GOVERNINGCARRIER", key._c);
  }

  void setKeyDateRange(MinFareApplHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MinFareAppl*>* vect) const override;

  virtual std::vector<MinFareAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareApplHistoricalDAO>;
  static DAOHelper<MinFareApplHistoricalDAO> _helper;
  MinFareApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MinFareApplHistoricalKey, std::vector<MinFareAppl*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<MinFareAppl*>* create(MinFareApplHistoricalKey key) override;
  void destroy(MinFareApplHistoricalKey key, std::vector<MinFareAppl*>* t) override;

private:
  static MinFareApplHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class MinFareApplHistoricalDAO
} // namespace tse
