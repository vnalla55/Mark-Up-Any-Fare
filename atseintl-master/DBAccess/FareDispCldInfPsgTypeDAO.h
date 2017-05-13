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
class FareDispCldInfPsgType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispCldInfPsgTypeKey;

class FareDispCldInfPsgTypeDAO
    : public DataAccessObject<FareDispCldInfPsgTypeKey, std::vector<FareDispCldInfPsgType*> >
{
public:
  static FareDispCldInfPsgTypeDAO& instance();

  const std::vector<FareDispCldInfPsgType*>& get(DeleteList& del,
                                                 const Indicator& userApplType,
                                                 const UserApplCode& userAppl,
                                                 const Indicator& pseudoCityType,
                                                 const PseudoCityCode& pseudoCity,
                                                 const InclusionCode& inclusionCode,
                                                 const Indicator& psgTypeInd,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispCldInfPsgTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispCldInfPsgTypeKey createKey(FareDispCldInfPsgType* info);

  void translateKey(const FareDispCldInfPsgTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispCldInfPsgType, FareDispCldInfPsgTypeDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispCldInfPsgTypeDAO>;

  static DAOHelper<FareDispCldInfPsgTypeDAO> _helper;

  FareDispCldInfPsgTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispCldInfPsgTypeKey, std::vector<FareDispCldInfPsgType*> >(cacheSize,
                                                                                       cacheType)
  {
  }

  std::vector<FareDispCldInfPsgType*>* create(FareDispCldInfPsgTypeKey key) override;
  void destroy(FareDispCldInfPsgTypeKey key, std::vector<FareDispCldInfPsgType*>* recs) override;

  virtual void load() override;

private:
  static FareDispCldInfPsgTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispCldInfPsgTypeHistoricalDAO
// --------------------------------------------------
class FareDispCldInfPsgTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareDispCldInfPsgTypeKey,
                                        std::vector<FareDispCldInfPsgType*> >
{
public:
  static FareDispCldInfPsgTypeHistoricalDAO& instance();

  const std::vector<FareDispCldInfPsgType*>& get(DeleteList& del,
                                                 const Indicator& userApplType,
                                                 const UserApplCode& userAppl,
                                                 const Indicator& pseudoCityType,
                                                 const PseudoCityCode& pseudoCity,
                                                 const InclusionCode& inclusionCode,
                                                 const Indicator& psgTypeInd,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispCldInfPsgTypeKey& key) const override
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

  friend class DAOHelper<FareDispCldInfPsgTypeHistoricalDAO>;

  static DAOHelper<FareDispCldInfPsgTypeHistoricalDAO> _helper;

  FareDispCldInfPsgTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispCldInfPsgTypeKey, std::vector<FareDispCldInfPsgType*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispCldInfPsgType*>* create(FareDispCldInfPsgTypeKey key) override;
  void destroy(FareDispCldInfPsgTypeKey key, std::vector<FareDispCldInfPsgType*>* recs) override;

private:
  static FareDispCldInfPsgTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

