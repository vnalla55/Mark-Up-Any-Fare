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

class CountrySettlementPlanInfo;
class DeleteList;

typedef HashKey<NationCode> CountrySettlementPlanKey;

class CountrySettlementPlanDAO
    : public DataAccessObject<CountrySettlementPlanKey, std::vector<CountrySettlementPlanInfo*> >
{
public:
  static CountrySettlementPlanDAO& instance();

  const std::vector<CountrySettlementPlanInfo*>&
  get(DeleteList& del, const NationCode& countryCode, const DateTime& ticketDate);

  CountrySettlementPlanKey createKey(const CountrySettlementPlanInfo* info);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool translateKey(const ObjectKey& objectKey, CountrySettlementPlanKey& key) const override;
  void translateKey(const CountrySettlementPlanKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CountrySettlementPlanDAO>;
  static DAOHelper<CountrySettlementPlanDAO> _helper;

  CountrySettlementPlanDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CountrySettlementPlanKey, std::vector<CountrySettlementPlanInfo*> >(
          cacheSize, cacheType, 1)
  {
  }

  std::vector<CountrySettlementPlanInfo*>* create(CountrySettlementPlanKey key) override;
  void
  destroy(CountrySettlementPlanKey key, std::vector<CountrySettlementPlanInfo*>* cspList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static CountrySettlementPlanDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<NationCode, DateTime, DateTime> CountrySettlementPlanHistoricalKey;

class CountrySettlementPlanHistoricalDAO
    : public HistoricalDataAccessObject<CountrySettlementPlanHistoricalKey, std::vector<CountrySettlementPlanInfo*> >
{
public:
  static CountrySettlementPlanHistoricalDAO& instance();

  const std::vector<CountrySettlementPlanInfo*>&
  get(DeleteList& del, const NationCode& countryCode, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, CountrySettlementPlanHistoricalKey& key) const override;
  void
  translateKey(const CountrySettlementPlanHistoricalKey& key, ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    CountrySettlementPlanHistoricalKey& key,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CountrySettlementPlanHistoricalDAO>;
  static DAOHelper<CountrySettlementPlanHistoricalDAO> _helper;

  CountrySettlementPlanHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CountrySettlementPlanHistoricalKey,
                                 std::vector<CountrySettlementPlanInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<CountrySettlementPlanInfo*>* create(CountrySettlementPlanHistoricalKey key) override;
  void destroy(CountrySettlementPlanHistoricalKey key,
               std::vector<CountrySettlementPlanInfo*>* cspList) override;

private:
  static CountrySettlementPlanHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}

