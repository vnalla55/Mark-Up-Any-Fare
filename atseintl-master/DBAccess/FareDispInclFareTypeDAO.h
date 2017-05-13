//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseBoostStringTypes.h"
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
class FareDispInclFareType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispInclFareTypeKey;

class FareDispInclFareTypeDAO
    : public DataAccessObject<FareDispInclFareTypeKey, std::vector<FareDispInclFareType*>, false>
{
public:
  static FareDispInclFareTypeDAO& instance();

  const std::vector<FareDispInclFareType*>& get(DeleteList& del,
                                                const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const InclusionCode& inclusionCode,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclFareTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispInclFareTypeKey createKey(FareDispInclFareType* info);

  void translateKey(const FareDispInclFareTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispInclFareType, FareDispInclFareTypeDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispInclFareTypeDAO>;

  static DAOHelper<FareDispInclFareTypeDAO> _helper;

  FareDispInclFareTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispInclFareTypeKey, std::vector<FareDispInclFareType*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispInclFareType*>* create(FareDispInclFareTypeKey key) override;
  void destroy(FareDispInclFareTypeKey key, std::vector<FareDispInclFareType*>* recs) override;

  virtual void load() override;

private:
  static FareDispInclFareTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispInclFareTypeHistoricalDAO
// --------------------------------------------------
class FareDispInclFareTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareDispInclFareTypeKey,
                                        std::vector<FareDispInclFareType*>,
                                        false>
{
public:
  static FareDispInclFareTypeHistoricalDAO& instance();

  const std::vector<FareDispInclFareType*>& get(DeleteList& del,
                                                const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const InclusionCode& inclusionCode,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclFareTypeKey& key) const override
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

  friend class DAOHelper<FareDispInclFareTypeHistoricalDAO>;

  static DAOHelper<FareDispInclFareTypeHistoricalDAO> _helper;

  FareDispInclFareTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispInclFareTypeKey,
                                 std::vector<FareDispInclFareType*>,
                                 false>(cacheSize, cacheType)
  {
  }

  std::vector<FareDispInclFareType*>* create(FareDispInclFareTypeKey key) override;
  void destroy(FareDispInclFareTypeKey key, std::vector<FareDispInclFareType*>* recs) override;

private:
  static FareDispInclFareTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

