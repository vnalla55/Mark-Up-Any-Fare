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

class NeutralValidatingAirlineInfo;
class DeleteList;

typedef HashKey<NationCode, CrsCode, SettlementPlanType> NeutralValidatingAirlineKey;

class NeutralValidatingAirlineDAO
    : public DataAccessObject<NeutralValidatingAirlineKey,
                              std::vector<NeutralValidatingAirlineInfo*> >
{
public:
  static NeutralValidatingAirlineDAO& instance();

  const std::vector<NeutralValidatingAirlineInfo*>& get(DeleteList& del,
                                                        const NationCode& country,
                                                        const CrsCode& gds,
                                                        const SettlementPlanType& spType,
                                                        const DateTime& ticketDate);

  NeutralValidatingAirlineKey createKey(const NeutralValidatingAirlineInfo* nva);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool translateKey(const ObjectKey& objectKey, NeutralValidatingAirlineKey& key) const override;
  void translateKey(const NeutralValidatingAirlineKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<NeutralValidatingAirlineDAO>;
  static DAOHelper<NeutralValidatingAirlineDAO> _helper;

  NeutralValidatingAirlineDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NeutralValidatingAirlineKey, std::vector<NeutralValidatingAirlineInfo*> >(
          cacheSize, cacheType, 1)
  {
  }

  std::vector<NeutralValidatingAirlineInfo*>* create(NeutralValidatingAirlineKey key) override;
  void destroy(NeutralValidatingAirlineKey key,
               std::vector<NeutralValidatingAirlineInfo*>* nvaList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static NeutralValidatingAirlineDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<NationCode, CrsCode, SettlementPlanType, DateTime, DateTime>
    NeutralValidatingAirlineHistoricalKey;

class NeutralValidatingAirlineHistoricalDAO
    : public HistoricalDataAccessObject<NeutralValidatingAirlineHistoricalKey,
                                        std::vector<NeutralValidatingAirlineInfo*> >
{
public:
  static NeutralValidatingAirlineHistoricalDAO& instance();

  const std::vector<NeutralValidatingAirlineInfo*>& get(DeleteList& del,
                                                        const NationCode& country,
                                                        const CrsCode& gds,
                                                        const SettlementPlanType& spType,
                                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    NeutralValidatingAirlineHistoricalKey& key) const override;
  void translateKey(const NeutralValidatingAirlineHistoricalKey& key,
                    ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    NeutralValidatingAirlineHistoricalKey& nvaKey,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<NeutralValidatingAirlineHistoricalDAO>;
  static DAOHelper<NeutralValidatingAirlineHistoricalDAO> _helper;

  NeutralValidatingAirlineHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NeutralValidatingAirlineHistoricalKey,
                                 std::vector<NeutralValidatingAirlineInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<NeutralValidatingAirlineInfo*>*
  create(NeutralValidatingAirlineHistoricalKey key) override;
  void destroy(NeutralValidatingAirlineHistoricalKey nvaKey,
               std::vector<NeutralValidatingAirlineInfo*>* nvaList) override;

private:
  static NeutralValidatingAirlineHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}

