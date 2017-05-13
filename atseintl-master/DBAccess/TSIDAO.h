//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TSIInfo;
class DeleteList;

class TSIDAO : public DataAccessObject<IntKey, TSIInfo, false>
{
public:
  static TSIDAO& instance();

  const TSIInfo* get(DeleteList& vector, int key);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    return key.initialized = objectKey.getValue("TSI", key._a);
  }

  IntKey createKey(TSIInfo* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TSI", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TSIInfo, TSIDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TSIDAO>;
  static DAOHelper<TSIDAO> _helper;
  TSIDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, TSIInfo, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  virtual size_t clear() override;

  TSIInfo* create(IntKey key) override;
  void destroy(IntKey key, TSIInfo* rec) override;

private:
  struct groupByKey;
  static TSIDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
