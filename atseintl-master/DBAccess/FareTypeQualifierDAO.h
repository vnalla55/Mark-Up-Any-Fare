//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class FareTypeQualifier;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, FareType> FareTypeQualifierKey;

class FareTypeQualifierDAO
    : public DataAccessObject<FareTypeQualifierKey, std::vector<FareTypeQualifier*>, false>
{
public:
  static FareTypeQualifierDAO& instance();
  const std::vector<FareTypeQualifier*>& get(DeleteList& del,
                                             Indicator userApplType,
                                             const UserApplCode& userAppl,
                                             const FareType& fareTypeQualifier,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareTypeQualifierKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("FARETYPEQUALIFIER", key._c);
  }

  FareTypeQualifierKey createKey(FareTypeQualifier* info);

  void translateKey(const FareTypeQualifierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("FARETYPEQUALIFIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareTypeQualifier, FareTypeQualifierDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareTypeQualifierDAO>;
  static DAOHelper<FareTypeQualifierDAO> _helper;
  FareTypeQualifierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareTypeQualifierKey, std::vector<FareTypeQualifier*>, false>(cacheSize,
                                                                                     cacheType)
  {
  }

  virtual void load() override;
  std::vector<FareTypeQualifier*>* create(FareTypeQualifierKey key) override;
  void destroy(FareTypeQualifierKey key, std::vector<FareTypeQualifier*>* t) override;

private:
  static FareTypeQualifierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: FareTypeQualifierHistoricalDAO
// --------------------------------------------------
class FareTypeQualifierHistoricalDAO
    : public HistoricalDataAccessObject<FareTypeQualifierKey,
                                        std::vector<FareTypeQualifier*>,
                                        false>
{
public:
  static FareTypeQualifierHistoricalDAO& instance();
  const std::vector<FareTypeQualifier*>& get(DeleteList& del,
                                             Indicator userApplType,
                                             const UserApplCode& userAppl,
                                             const FareType& fareTypeQualifier,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareTypeQualifierKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("FARETYPEQUALIFIER", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareTypeQualifierHistoricalDAO>;
  static DAOHelper<FareTypeQualifierHistoricalDAO> _helper;
  FareTypeQualifierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareTypeQualifierKey, std::vector<FareTypeQualifier*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareTypeQualifier*>* create(FareTypeQualifierKey key) override;
  void destroy(FareTypeQualifierKey key, std::vector<FareTypeQualifier*>* t) override;

  virtual void load() override;

private:
  struct groupByKey;
  static FareTypeQualifierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
