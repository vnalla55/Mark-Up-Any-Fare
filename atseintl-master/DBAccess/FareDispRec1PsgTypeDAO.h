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
class FareDispRec1PsgType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDispRec1PsgTypeKey;

class FareDispRec1PsgTypeDAO
    : public DataAccessObject<FareDispRec1PsgTypeKey, std::vector<FareDispRec1PsgType*> >
{
public:
  static FareDispRec1PsgTypeDAO& instance();

  const std::vector<FareDispRec1PsgType*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispRec1PsgTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDispRec1PsgTypeKey createKey(FareDispRec1PsgType* info);

  void translateKey(const FareDispRec1PsgTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispRec1PsgType, FareDispRec1PsgTypeDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispRec1PsgTypeDAO>;

  static DAOHelper<FareDispRec1PsgTypeDAO> _helper;

  FareDispRec1PsgTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispRec1PsgTypeKey, std::vector<FareDispRec1PsgType*> >(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<FareDispRec1PsgType*>* create(FareDispRec1PsgTypeKey key) override;
  void destroy(FareDispRec1PsgTypeKey key, std::vector<FareDispRec1PsgType*>* recs) override;

  virtual void load() override;

private:
  static FareDispRec1PsgTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispRec1PsgTypeHistoricalDAO
// --------------------------------------------------
class FareDispRec1PsgTypeHistoricalDAO
    : public HistoricalDataAccessObject<FareDispRec1PsgTypeKey, std::vector<FareDispRec1PsgType*> >
{
public:
  static FareDispRec1PsgTypeHistoricalDAO& instance();

  const std::vector<FareDispRec1PsgType*>& get(DeleteList& del,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const InclusionCode& inclusionCode,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispRec1PsgTypeKey& key) const override
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

  friend class DAOHelper<FareDispRec1PsgTypeHistoricalDAO>;

  static DAOHelper<FareDispRec1PsgTypeHistoricalDAO> _helper;

  FareDispRec1PsgTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispRec1PsgTypeKey, std::vector<FareDispRec1PsgType*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispRec1PsgType*>* create(FareDispRec1PsgTypeKey key) override;
  void destroy(FareDispRec1PsgTypeKey key, std::vector<FareDispRec1PsgType*>* recs) override;

private:
  static FareDispRec1PsgTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

