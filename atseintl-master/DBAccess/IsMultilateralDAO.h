//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class IndustryFareAppl;

typedef HashKey<VendorCode, RuleNumber> RuleIndexKey;

class IsMultilateralDAO
    : public DataAccessObject<CharKey, std::multimap<RuleIndexKey, IndustryFareAppl*>, false>
{
public:
  static IsMultilateralDAO& instance();

  bool isMultilateral(const VendorCode& vendor,
                      const RuleNumber& rule,
                      const LocCode& loc1,
                      const LocCode& loc2,
                      const DateTime& startDate,
                      const DateTime& endDate,
                      const DateTime& ticketDate);
  bool translateKey(const ObjectKey& objectKey, CharKey& key) const override
  {
    return key.initialized = objectKey.getValue("SELECTIONTYPE", key._a);
  }

  CharKey createKey(IndustryFareAppl* info);

  void translateKey(const CharKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SELECTIONTYPE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<IndustryFareAppl, IsMultilateralDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IsMultilateralDAO>;
  static DAOHelper<IsMultilateralDAO> _helper;
  IsMultilateralDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CharKey, std::multimap<RuleIndexKey, IndustryFareAppl*>, false>(cacheSize,
                                                                                       cacheType)
  {
  }
  std::multimap<RuleIndexKey, IndustryFareAppl*>* create(CharKey key) override;
  void destroy(CharKey key, std::multimap<RuleIndexKey, IndustryFareAppl*>* recs) override;

  virtual void load() override;

private:
  static IsMultilateralDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: IsMultilateralHistoricalDAO
// --------------------------------------------------
class IsMultilateralHistoricalDAO
    : public HistoricalDataAccessObject<Indicator,
                                        std::multimap<RuleIndexKey, IndustryFareAppl*>,
                                        false>
{
public:
  static IsMultilateralHistoricalDAO& instance();

  bool isMultilateral(const VendorCode& vendor,
                      const RuleNumber& rule,
                      const LocCode& loc1,
                      const LocCode& loc2,
                      const DateTime& date,
                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, Indicator& key) const override
  {
    return objectKey.getValue("SELECTIONTYPE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IsMultilateralHistoricalDAO>;
  static DAOHelper<IsMultilateralHistoricalDAO> _helper;
  IsMultilateralHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<Indicator, std::multimap<RuleIndexKey, IndustryFareAppl*>, false>(
          cacheSize, cacheType)
  {
  }
  std::multimap<RuleIndexKey, IndustryFareAppl*>* create(Indicator key) override;
  void destroy(Indicator key, std::multimap<RuleIndexKey, IndustryFareAppl*>* recs) override;

  virtual void load() override;

private:
  static IsMultilateralHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
