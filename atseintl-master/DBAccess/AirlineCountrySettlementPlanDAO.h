//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
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

class AirlineCountrySettlementPlanInfo;
class DeleteList;

typedef HashKey<NationCode, CrsCode, CarrierCode, SettlementPlanType>
    AirlineCountrySettlementPlanKey;

class AirlineCountrySettlementPlanDAO
    : public DataAccessObject<AirlineCountrySettlementPlanKey, std::vector<AirlineCountrySettlementPlanInfo*> >
{
public:
  static AirlineCountrySettlementPlanDAO& instance();

  const std::vector<AirlineCountrySettlementPlanInfo*>& get(DeleteList& del,
                                                            const NationCode& country,
                                                            const CrsCode& gds,
                                                            const CarrierCode& airline,
                                                            const SettlementPlanType& spType,
                                                            const DateTime& ticketDate);

  const std::vector<AirlineCountrySettlementPlanInfo*>& get(DeleteList& del,
                                                            const NationCode& country,
                                                            const CrsCode& gds,
                                                            const SettlementPlanType& spType,
                                                            const DateTime& date);

  const std::vector<AirlineCountrySettlementPlanInfo*>& get(DeleteList& del,
                                                            const CrsCode& gds,
                                                            const NationCode& country,
                                                            const CarrierCode& airline,
                                                            const DateTime& date);

  AirlineCountrySettlementPlanKey createKey(const AirlineCountrySettlementPlanInfo* acsp);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool
  translateKey(const ObjectKey& objectKey, AirlineCountrySettlementPlanKey& key) const override;
  void
  translateKey(const AirlineCountrySettlementPlanKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AirlineCountrySettlementPlanDAO>;
  static DAOHelper<AirlineCountrySettlementPlanDAO> _helper;

  AirlineCountrySettlementPlanDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AirlineCountrySettlementPlanKey,
                       std::vector<AirlineCountrySettlementPlanInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<AirlineCountrySettlementPlanInfo*>*
  create(AirlineCountrySettlementPlanKey key) override;
  void destroy(AirlineCountrySettlementPlanKey key,
               std::vector<AirlineCountrySettlementPlanInfo*>* acspList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static AirlineCountrySettlementPlanDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};


// ----------------------------------------------------------------------------
// Historical DAO: AirlineCountrySettlementPlanHistoricalDAO
// ----------------------------------------------------------------------------

typedef HashKey<NationCode, CrsCode, CarrierCode, SettlementPlanType, DateTime, DateTime>
AirlineCountrySettlementPlanHistoricalKey;

class AirlineCountrySettlementPlanHistoricalDAO
    : public HistoricalDataAccessObject<AirlineCountrySettlementPlanHistoricalKey,
                                        std::vector<AirlineCountrySettlementPlanInfo*> >
{
public:
  static AirlineCountrySettlementPlanHistoricalDAO& instance();

  const std::vector<AirlineCountrySettlementPlanInfo*>& get(DeleteList& del,
                                                            const NationCode& country,
                                                            const CrsCode& gds,
                                                            const CarrierCode& airline,
                                                            const SettlementPlanType& spType,
                                                            const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    AirlineCountrySettlementPlanHistoricalKey& key) const override;
  void translateKey(const AirlineCountrySettlementPlanHistoricalKey& key,
                    ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    AirlineCountrySettlementPlanHistoricalKey& key,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AirlineCountrySettlementPlanHistoricalDAO>;
  static DAOHelper<AirlineCountrySettlementPlanHistoricalDAO> _helper;

  AirlineCountrySettlementPlanHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AirlineCountrySettlementPlanHistoricalKey,
                                 std::vector<AirlineCountrySettlementPlanInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<AirlineCountrySettlementPlanInfo*>*
  create(AirlineCountrySettlementPlanHistoricalKey key) override;
  void destroy(AirlineCountrySettlementPlanHistoricalKey key,
               std::vector<AirlineCountrySettlementPlanInfo*>* acspList) override;

private:
  static AirlineCountrySettlementPlanHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // name space

