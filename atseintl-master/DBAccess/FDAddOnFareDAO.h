//----------------------------------------------------------------------------
//  File:   FDAddOnFareDAO.h
//  Created: May 23, 2005
//  Authors: Partha Kumar Chakraborti
//
//  Description:
//
//  Change History:
//    date - initials - description
//
//  Copyright Sabre 2005
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
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
class AddonFareInfo;
class DeleteList;

typedef HashKey<LocCode, LocCode, CarrierCode> FDAddOnFareKey;

class FDAddOnFareDAO : public DataAccessObject<FDAddOnFareKey, std::vector<AddonFareInfo*> >
{
public:
  static FDAddOnFareDAO& instance();

  const std::vector<AddonFareInfo*>& get(DeleteList& del,
                                         const LocCode& gatewayMarket,
                                         const LocCode& interiorMarket,
                                         const CarrierCode& carrier,
                                         const RecordScope& crossRefType,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FDAddOnFareKey& key) const override
  {
    return key.initialized = objectKey.getValue("GATEWAYMARKET", key._a) &&
                             objectKey.getValue("INTERIORMARKET", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  FDAddOnFareKey createKey(const AddonFareInfo* info);

  void translateKey(const FDAddOnFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("GATEWAYMARKET", key._a);
    objectKey.setValue("INTERIORMARKET", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonFareInfo, FDAddOnFareDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FDAddOnFareDAO>;
  static DAOHelper<FDAddOnFareDAO> _helper;
  FDAddOnFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDAddOnFareKey, std::vector<AddonFareInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<AddonFareInfo*>* create(FDAddOnFareKey key) override;
  void destroy(FDAddOnFareKey key, std::vector<AddonFareInfo*>* t) override;
  void getGlobalDirFrmTariffXRef(DeleteList& del,
                                 std::vector<AddonFareInfo*>& addOnFareInfoList,
                                 const RecordScope& crossRefType,
                                 const DateTime& ticketDate);
  void load() override;

private:
  struct isEffective;
  static FDAddOnFareDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<LocCode, LocCode, CarrierCode, DateTime, DateTime> FDAddOnFareHistoricalKey;

class FDAddOnFareHistoricalDAO
    : public HistoricalDataAccessObject<FDAddOnFareHistoricalKey, std::vector<AddonFareInfo*> >
{
public:
  static FDAddOnFareHistoricalDAO& instance();

  const std::vector<AddonFareInfo*>& get(DeleteList& del,
                                         const LocCode& gatewayMarket,
                                         const LocCode& interiorMarket,
                                         const CarrierCode& carrier,
                                         const RecordScope& crossRefType,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FDAddOnFareHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("GATEWAYMARKET", key._a)
                             && objectKey.getValue("INTERIORMARKET", key._b)
                             && objectKey.getValue("CARRIER", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FDAddOnFareHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("GATEWAYMARKET", key._a) &&
                             objectKey.getValue("INTERIORMARKET", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  void setKeyDateRange(FDAddOnFareHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FDAddOnFareHistoricalDAO>;
  static DAOHelper<FDAddOnFareHistoricalDAO> _helper;
  FDAddOnFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FDAddOnFareHistoricalKey, std::vector<AddonFareInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<AddonFareInfo*>* create(FDAddOnFareHistoricalKey key) override;
  void destroy(FDAddOnFareHistoricalKey key, std::vector<AddonFareInfo*>* t) override;
  void getGlobalDirFrmTariffXRef(DeleteList& del,
                                 std::vector<AddonFareInfo*>& addOnFareInfoList,
                                 const RecordScope& crossRefType,
                                 const DateTime& ticketDate);

private:
  static FDAddOnFareHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
