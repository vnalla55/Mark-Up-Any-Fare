//-------------------------------------------------------------------------------
// Copyright 2015, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

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
class IntralineCarrierInfo;
class DeleteList;

typedef HashKey<std::string> NameKey;

class IntralineCarrierDAO :
      public DataAccessObject<NameKey, std::vector<IntralineCarrierInfo*>, false>
{
public:
  static IntralineCarrierDAO& instance();

  const std::vector<IntralineCarrierInfo*>&
    get(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, NameKey& key) const override
  {
    return key.initialized = objectKey.getValue("NAME", WHOLE_TBL_STR_KEY._a);
  }

  NameKey createKey(IntralineCarrierInfo* info) { return WHOLE_TBL_STR_KEY; }

  void translateKey(const NameKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NAME", WHOLE_TBL_STR_KEY._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<IntralineCarrierInfo, IntralineCarrierDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual size_t clear() override;
protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IntralineCarrierDAO>;
  static DAOHelper<IntralineCarrierDAO> _helper;
  IntralineCarrierDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2)
    : DataAccessObject<NameKey, std::vector<IntralineCarrierInfo*>, false>(
          cacheSize, cacheType, version)
  {
  }
  virtual void load() override;
  std::vector<IntralineCarrierInfo*>* create(NameKey key) override;
  void destroy(NameKey key, std::vector<IntralineCarrierInfo*>* recs) override;

private:
  static NameKey WHOLE_TBL_STR_KEY;
  static IntralineCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class IntralineCarrierDAO

// --------------------------------------------------
// Historical DAO: IntralineCarrierHistoricalDAO
// --------------------------------------------------
class IntralineCarrierHistoricalDAO :
      public HistoricalDataAccessObject<std::string, std::vector<IntralineCarrierInfo*>, false>
{
public:
  static IntralineCarrierHistoricalDAO& instance();
  const std::vector<IntralineCarrierInfo*>&
   get(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, std::string& key) const override
  {
    return objectKey.getValue("NAME", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IntralineCarrierHistoricalDAO>;
  static DAOHelper<IntralineCarrierHistoricalDAO> _helper;
  IntralineCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<std::string, std::vector<IntralineCarrierInfo*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }
  virtual void load() override;
  std::vector<IntralineCarrierInfo*>* create(std::string key) override;
  void destroy(std::string key, std::vector<IntralineCarrierInfo*>* recs) override;

private:
  struct groupByKey;
  static IntralineCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class IntralineCarrierHistoricalDAO
} // namespace tse
