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
class FareDisplayInclCd;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, InclusionCode>
FareDisplayInclCdKey;

class FareDisplayInclCdDAO
    : public DataAccessObject<FareDisplayInclCdKey, std::vector<FareDisplayInclCd*>, false>
{
public:
  static FareDisplayInclCdDAO& instance();

  const std::vector<FareDisplayInclCd*>& get(DeleteList& del,
                                             const Indicator& userApplType,
                                             const UserApplCode& userAppl,
                                             const Indicator& pseudoCityType,
                                             const PseudoCityCode& pseudoCity,
                                             const InclusionCode& inclusionCode,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDisplayInclCdKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDisplayInclCdKey createKey(FareDisplayInclCd* info);

  void translateKey(const FareDisplayInclCdKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDisplayInclCd, FareDisplayInclCdDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDisplayInclCdDAO>;

  static DAOHelper<FareDisplayInclCdDAO> _helper;

  FareDisplayInclCdDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDisplayInclCdKey, std::vector<FareDisplayInclCd*>, false>(cacheSize,
                                                                                     cacheType)
  {
  }

  std::vector<FareDisplayInclCd*>* create(FareDisplayInclCdKey key) override;
  void destroy(FareDisplayInclCdKey key, std::vector<FareDisplayInclCd*>* recs) override;

  virtual void load() override;

private:
  static FareDisplayInclCdDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDisplayInclCdHistoricalDAO
// --------------------------------------------------
class FareDisplayInclCdHistoricalDAO
    : public HistoricalDataAccessObject<FareDisplayInclCdKey,
                                        std::vector<FareDisplayInclCd*>,
                                        false>
{
public:
  static FareDisplayInclCdHistoricalDAO& instance();

  const std::vector<FareDisplayInclCd*>& get(DeleteList& del,
                                             const Indicator& userApplType,
                                             const UserApplCode& userAppl,
                                             const Indicator& pseudoCityType,
                                             const PseudoCityCode& pseudoCity,
                                             const InclusionCode& inclusionCode,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDisplayInclCdKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("INCLUSIONCODE", key._e);
  }

  FareDisplayInclCdKey createKey(const FareDisplayInclCd* info,
                                 const DateTime& startDate = DateTime::emptyDate(),
                                 const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareDisplayInclCdKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("INCLUSIONCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FareDisplayInclCd, FareDisplayInclCdHistoricalDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDisplayInclCdHistoricalDAO>;

  static DAOHelper<FareDisplayInclCdHistoricalDAO> _helper;

  FareDisplayInclCdHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDisplayInclCdKey, std::vector<FareDisplayInclCd*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDisplayInclCd*>* create(FareDisplayInclCdKey key) override;
  void destroy(FareDisplayInclCdKey key, std::vector<FareDisplayInclCd*>* recs) override;
  void load() override;

private:
  static FareDisplayInclCdHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

