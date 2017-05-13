//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class ContractPreference;
class DeleteList;

typedef HashKey<PseudoCityCode, CarrierCode, RuleNumber> ContractPreferenceKey;

class ContractPreferenceDAO
    : public DataAccessObject<ContractPreferenceKey, std::vector<ContractPreference*>, false>
{
public:
  static ContractPreferenceDAO& instance();
  const std::vector<ContractPreference*>& get(DeleteList& del,
                                              const PseudoCityCode& pseudoCity,
                                              const CarrierCode& carrier,
                                              const RuleNumber& rule,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ContractPreferenceKey& key) const override
  {
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("RULE", key._c);
  }

  ContractPreferenceKey createKey(const ContractPreference* info);

  void translateKey(const ContractPreferenceKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITY", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ContractPreference, ContractPreferenceDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<ContractPreference*>* vect) const override;

  virtual std::vector<ContractPreference*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ContractPreferenceDAO>;
  static DAOHelper<ContractPreferenceDAO> _helper;
  ContractPreferenceDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ContractPreferenceKey, std::vector<ContractPreference*>, false>(cacheSize,
                                                                                       cacheType)
  {
  }

  std::vector<ContractPreference*>* create(ContractPreferenceKey key) override;
  void destroy(ContractPreferenceKey key, std::vector<ContractPreference*>* recs) override;
  void load() override;

private:
  static ContractPreferenceDAO* _instance;
  struct isEffective;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: ContractPreferenceHistoricalDAO
// --------------------------------------------------
class ContractPreferenceHistoricalDAO
    : public HistoricalDataAccessObject<ContractPreferenceKey,
                                        std::vector<ContractPreference*>,
                                        false>
{
public:
  static ContractPreferenceHistoricalDAO& instance();
  const std::vector<ContractPreference*>& get(DeleteList& del,
                                              const PseudoCityCode& pseudoCity,
                                              const CarrierCode& carrier,
                                              const RuleNumber& rule,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ContractPreferenceKey& key) const override
  {
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("RULE", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<ContractPreference*>* vect) const override;

  virtual std::vector<ContractPreference*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ContractPreferenceHistoricalDAO>;
  static DAOHelper<ContractPreferenceHistoricalDAO> _helper;
  ContractPreferenceHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ContractPreferenceKey, std::vector<ContractPreference*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<ContractPreference*>* create(ContractPreferenceKey key) override;
  void destroy(ContractPreferenceKey key, std::vector<ContractPreference*>* recs) override;

  virtual void load() override;

private:
  static ContractPreferenceHistoricalDAO* _instance;
  struct isEffective;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
