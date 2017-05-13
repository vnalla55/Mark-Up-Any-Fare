//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

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
class IndustryFareBasisMod;
class DeleteList;

typedef HashKey<CarrierCode, Indicator, UserApplCode> IndustryFareBasisModKey;

class IndustryFareBasisModDAO
    : public DataAccessObject<IndustryFareBasisModKey, std::vector<IndustryFareBasisMod*>, false>
{
public:
  static IndustryFareBasisModDAO& instance();

  const std::vector<const IndustryFareBasisMod*>& get(DeleteList& del,
                                                      const CarrierCode& carrier,
                                                      const Indicator& userApplType,
                                                      const UserApplCode& userAppl,
                                                      const DateTime& date,
                                                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IndustryFareBasisModKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("USERAPPLTYPE", key._b) &&
                             objectKey.getValue("USERAPPLCODE", key._c);
  }

  IndustryFareBasisModKey createKey(IndustryFareBasisMod* info);

  void translateKey(const IndustryFareBasisModKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("USERAPPLTYPE", key._b);
    objectKey.setValue("USERAPPLCODE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<IndustryFareBasisMod, IndustryFareBasisModDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<IndustryFareBasisModDAO>;

  static DAOHelper<IndustryFareBasisModDAO> _helper;

  IndustryFareBasisModDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IndustryFareBasisModKey, std::vector<IndustryFareBasisMod*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<IndustryFareBasisMod*>* create(IndustryFareBasisModKey key) override;
  void destroy(IndustryFareBasisModKey key, std::vector<IndustryFareBasisMod*>* recs) override;

  virtual void load() override;

private:
  static IndustryFareBasisModDAO* _instance;
  struct isEffective;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: IndustryFareBasisModHistoricalDAO
// --------------------------------------------------
class IndustryFareBasisModHistoricalDAO
    : public HistoricalDataAccessObject<IndustryFareBasisModKey,
                                        std::vector<IndustryFareBasisMod*>,
                                        false>
{
public:
  static IndustryFareBasisModHistoricalDAO& instance();

  const std::vector<const IndustryFareBasisMod*>& get(DeleteList& del,
                                                      const CarrierCode& carrier,
                                                      const Indicator& userApplType,
                                                      const UserApplCode& userAppl,
                                                      const DateTime& date,
                                                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IndustryFareBasisModKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("USERAPPLTYPE", key._b) &&
                             objectKey.getValue("USERAPPLCODE", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<IndustryFareBasisModHistoricalDAO>;

  static DAOHelper<IndustryFareBasisModHistoricalDAO> _helper;

  IndustryFareBasisModHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<IndustryFareBasisModKey,
                                 std::vector<IndustryFareBasisMod*>,
                                 false>(cacheSize, cacheType)
  {
  }

  std::vector<IndustryFareBasisMod*>* create(IndustryFareBasisModKey key) override;
  void destroy(IndustryFareBasisModKey key, std::vector<IndustryFareBasisMod*>* recs) override;

  virtual void load() override;

private:
  static IndustryFareBasisModHistoricalDAO* _instance;
  struct groupByKey;
  struct isEffective;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
