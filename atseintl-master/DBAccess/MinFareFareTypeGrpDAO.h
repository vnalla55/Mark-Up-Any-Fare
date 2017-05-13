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
class MinFareFareTypeGrp;
class DeleteList;

class MinFareFareTypeGrpDAO
    : public DataAccessObject<StringKey, std::vector<MinFareFareTypeGrp*>, false>
{
public:
  static MinFareFareTypeGrpDAO& instance();
  const MinFareFareTypeGrp*
  get(DeleteList& del, const std::string& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, StringKey& key) const override
  {
    return key.initialized = objectKey.getValue("SPECIALPROCESSNAME", key._a);
  }

  StringKey createKey(MinFareFareTypeGrp* info);

  void translateKey(const StringKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SPECIALPROCESSNAME", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MinFareFareTypeGrp, MinFareFareTypeGrpDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinFareFareTypeGrp*>* vect) const override;

  virtual std::vector<MinFareFareTypeGrp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareFareTypeGrpDAO>;
  static DAOHelper<MinFareFareTypeGrpDAO> _helper;
  MinFareFareTypeGrpDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StringKey, std::vector<MinFareFareTypeGrp*>, false>(cacheSize, cacheType, 2)
  {
  }
  virtual void load() override;
  std::vector<MinFareFareTypeGrp*>* create(StringKey key) override;
  void destroy(StringKey key, std::vector<MinFareFareTypeGrp*>* t) override;

private:
  struct isEffective;
  static MinFareFareTypeGrpDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: MinFareFareTypeGrpHistoricalDAO
// --------------------------------------------------
class MinFareFareTypeGrpHistoricalDAO
    : public HistoricalDataAccessObject<std::string, std::vector<MinFareFareTypeGrp*>, false>
{
public:
  static MinFareFareTypeGrpHistoricalDAO& instance();
  const MinFareFareTypeGrp*
  get(DeleteList& del, const std::string& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, std::string& key) const override
  {
    return objectKey.getValue("SPECIALPROCESSNAME", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinFareFareTypeGrp*>* vect) const override;

  virtual std::vector<MinFareFareTypeGrp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareFareTypeGrpHistoricalDAO>;
  static DAOHelper<MinFareFareTypeGrpHistoricalDAO> _helper;
  MinFareFareTypeGrpHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<std::string, std::vector<MinFareFareTypeGrp*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<MinFareFareTypeGrp*>* create(std::string key) override;
  void destroy(std::string key, std::vector<MinFareFareTypeGrp*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static MinFareFareTypeGrpHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
