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
class FareDispInclDsplType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispInclDsplTypeKey;

class FareDispInclDsplTypeDAO
    : public DataAccessObject<FareDispInclDsplTypeKey, std::vector<FareDispInclDsplType*>, false>
{
public:
  static FareDispInclDsplTypeDAO& instance();

  const std::vector<FareDispInclDsplType*>& get(DeleteList& del,
                                                const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const InclusionCode& inclusionCode,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclDsplTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispInclDsplTypeKey createKey(FareDispInclDsplType* info);

  void translateKey(const FareDispInclDsplTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispInclDsplType, FareDispInclDsplTypeDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispInclDsplTypeDAO>;

  static DAOHelper<FareDispInclDsplTypeDAO> _helper;

  FareDispInclDsplTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispInclDsplTypeKey, std::vector<FareDispInclDsplType*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispInclDsplType*>* create(FareDispInclDsplTypeKey key) override;
  void destroy(FareDispInclDsplTypeKey key, std::vector<FareDispInclDsplType*>* recs) override;

  virtual void load() override;

private:
  static FareDispInclDsplTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispInclDsplTypeHistoricalDAO
// --------------------------------------------------
class FareDispInclDsplTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareDispInclDsplTypeKey,
                                        std::vector<FareDispInclDsplType*>,
                                        false>
{
public:
  static FareDispInclDsplTypeHistoricalDAO& instance();

  const std::vector<FareDispInclDsplType*>& get(DeleteList& del,
                                                const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const InclusionCode& inclusionCode,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispInclDsplTypeKey& key) const override
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

  friend class DAOHelper<FareDispInclDsplTypeHistoricalDAO>;

  static DAOHelper<FareDispInclDsplTypeHistoricalDAO> _helper;

  FareDispInclDsplTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispInclDsplTypeKey,
                                 std::vector<FareDispInclDsplType*>,
                                 false>(cacheSize, cacheType)
  {
  }

  std::vector<FareDispInclDsplType*>* create(FareDispInclDsplTypeKey key) override;
  void destroy(FareDispInclDsplTypeKey key, std::vector<FareDispInclDsplType*>* recs) override;

private:
  static FareDispInclDsplTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

