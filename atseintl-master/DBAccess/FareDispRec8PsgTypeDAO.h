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
class FareDispRec8PsgType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispRec8PsgTypeKey;

class FareDispRec8PsgTypeDAO
    : public DataAccessObject<FareDispRec8PsgTypeKey, std::vector<FareDispRec8PsgType*> >
{
public:
  static FareDispRec8PsgTypeDAO& instance();

  const std::vector<FareDispRec8PsgType*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispRec8PsgTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispRec8PsgTypeKey createKey(FareDispRec8PsgType* info);

  void translateKey(const FareDispRec8PsgTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispRec8PsgType, FareDispRec8PsgTypeDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispRec8PsgTypeDAO>;

  static DAOHelper<FareDispRec8PsgTypeDAO> _helper;

  FareDispRec8PsgTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispRec8PsgTypeKey, std::vector<FareDispRec8PsgType*> >(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<FareDispRec8PsgType*>* create(FareDispRec8PsgTypeKey key) override;
  void destroy(FareDispRec8PsgTypeKey key, std::vector<FareDispRec8PsgType*>* recs) override;

  virtual void load() override;

private:
  static FareDispRec8PsgTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispRec8PsgTypeHistoricalDAO
// --------------------------------------------------
class FareDispRec8PsgTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareDispRec8PsgTypeKey, std::vector<FareDispRec8PsgType*> >
{
public:
  static FareDispRec8PsgTypeHistoricalDAO& instance();

  const std::vector<FareDispRec8PsgType*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispRec8PsgTypeKey& key) const override
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

  friend class DAOHelper<FareDispRec8PsgTypeHistoricalDAO>;

  static DAOHelper<FareDispRec8PsgTypeHistoricalDAO> _helper;

  FareDispRec8PsgTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispRec8PsgTypeKey, std::vector<FareDispRec8PsgType*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispRec8PsgType*>* create(FareDispRec8PsgTypeKey key) override;
  void destroy(FareDispRec8PsgTypeKey key, std::vector<FareDispRec8PsgType*>* recs) override;

private:
  static FareDispRec8PsgTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

