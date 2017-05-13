//----------------------------------------------------------------------------
//  File: FDSuppressFareDAO.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDSuppressFare.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
typedef HashKey<PseudoCityCode, Indicator, TJRGroup, CarrierCode> FDSuppressFareKey;

class FDSuppressFareDAO
    : public DataAccessObject<FDSuppressFareKey, std::vector<const FDSuppressFare*> >
{
public:
  static FDSuppressFareDAO& instance();

  std::vector<const tse::FDSuppressFare*>& get(DeleteList& del,
                                               const PseudoCityCode& pseudoCityCode,
                                               const Indicator pseudoCityType,
                                               const TJRGroup& ssgGroupNo,
                                               const CarrierCode& carrierCode,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FDSuppressFareKey& key) const override
  {
    // Do not translate CARRIERCODE key._d, as it is always NULL in cachekeys
    return key.initialized = objectKey.getValue("PSEUDOCITYCODE", key._a) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._b) &&
                             objectKey.getValue("SSGGROUPNO", key._c); //&&
    // objectKey.getValue("CARRIERCODE", key._d);
  }

  FDSuppressFareKey createKey(const FDSuppressFare* info);

  void translateKey(const FDSuppressFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITYCODE", key._a);
    objectKey.setValue("PSEUDOCITYTYPE", key._b);
    objectKey.setValue("SSGGROUPNO", key._c);
    // objectKey.setValue("CARRIERCODE", key._d) ;
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FDSuppressFare, FDSuppressFareDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDSuppressFareDAO>;

  static DAOHelper<FDSuppressFareDAO> _helper;

  FDSuppressFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDSuppressFareKey, std::vector<const FDSuppressFare*> >(cacheSize, cacheType)
  {
  }

  std::vector<const tse::FDSuppressFare*>* create(FDSuppressFareKey key) override;

  void destroy(FDSuppressFareKey key, std::vector<const tse::FDSuppressFare*>* t) override;

  void load() override;

private:
  static FDSuppressFareDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FDSuppressFareHistoricalDAO
// --------------------------------------------------
class FDSuppressFareHistoricalDAO
    : public HistoricalDataAccessObject<FDSuppressFareKey, std::vector<const FDSuppressFare*> >
{
public:
  static FDSuppressFareHistoricalDAO& instance();

  std::vector<const tse::FDSuppressFare*>& get(DeleteList& del,
                                               const PseudoCityCode& pseudoCityCode,
                                               const Indicator pseudoCityType,
                                               const TJRGroup& ssgGroupNo,
                                               const CarrierCode& carrierCode,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FDSuppressFareKey& key) const override
  {
    // Do not translate CARRIERCODE key._d, as it is always NULL in cachekeys
    return key.initialized = objectKey.getValue("PSEUDOCITYCODE", key._a) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._b) &&
                             objectKey.getValue("SSGGROUPNO", key._c); //&&
    // objectKey.getValue("CARRIERCODE", key._d);
  }

  FDSuppressFareKey createKey(const FDSuppressFare* info,
                              const DateTime& startDate = DateTime::emptyDate(),
                              const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FDSuppressFareKey& key, ObjectKey& objectKey) const override
  {
    // Do not translate CARRIERCODE key._d, as it is always NULL in cachekeys
    objectKey.setValue("PSEUDOCITYCODE", key._a);
    objectKey.setValue("PSEUDOCITYTYPE", key._b);
    objectKey.setValue("SSGGROUPNO", key._c);
    // objectKey.setValue("CARRIERCODE", key._d) ;
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FDSuppressFare, FDSuppressFareHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDSuppressFareHistoricalDAO>;

  static DAOHelper<FDSuppressFareHistoricalDAO> _helper;

  FDSuppressFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FDSuppressFareKey, std::vector<const FDSuppressFare*> >(cacheSize,
                                                                                         cacheType)
  {
  }

  std::vector<const tse::FDSuppressFare*>* create(FDSuppressFareKey key) override;

  void destroy(FDSuppressFareKey key, std::vector<const tse::FDSuppressFare*>* t) override;
  void load() override;

private:
  static FDSuppressFareHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // End of namespace tse
