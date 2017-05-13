//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class FareDispInclRuleTrf;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispInclRuleTrfKey;

class FareDispInclRuleTrfDAO
    : public DataAccessObject<FareDispInclRuleTrfKey, std::vector<FareDispInclRuleTrf*>, false>
{
public:
  static FareDispInclRuleTrfDAO& instance();

  const std::vector<FareDispInclRuleTrf*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclRuleTrfKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispInclRuleTrfKey createKey(FareDispInclRuleTrf* info);

  void translateKey(const FareDispInclRuleTrfKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispInclRuleTrf, FareDispInclRuleTrfDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispInclRuleTrfDAO>;

  static DAOHelper<FareDispInclRuleTrfDAO> _helper;

  FareDispInclRuleTrfDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispInclRuleTrfKey, std::vector<FareDispInclRuleTrf*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }

  std::vector<FareDispInclRuleTrf*>* create(FareDispInclRuleTrfKey key) override;
  void destroy(FareDispInclRuleTrfKey key, std::vector<FareDispInclRuleTrf*>* recs) override;

  virtual void load() override;

private:
  static FareDispInclRuleTrfDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispInclRuleTrfHistoricalDAO
// --------------------------------------------------
class FareDispInclRuleTrfHistoricalDAO
    : public HistoricalDataAccessObject<FareDispInclRuleTrfKey,
                                        std::vector<FareDispInclRuleTrf*>,
                                        false>
{
public:
  static FareDispInclRuleTrfHistoricalDAO& instance();

  const std::vector<FareDispInclRuleTrf*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclRuleTrfKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispInclRuleTrfHistoricalDAO>;

  static DAOHelper<FareDispInclRuleTrfHistoricalDAO> _helper;

  FareDispInclRuleTrfHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispInclRuleTrfKey, std::vector<FareDispInclRuleTrf*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispInclRuleTrf*>* create(FareDispInclRuleTrfKey key) override;
  void destroy(FareDispInclRuleTrfKey key, std::vector<FareDispInclRuleTrf*>* recs) override;

private:
  static FareDispInclRuleTrfHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

