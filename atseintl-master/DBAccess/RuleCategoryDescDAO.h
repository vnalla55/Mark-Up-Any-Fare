//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"

namespace tse
{
class RuleCategoryDescInfo;
class DeleteList;

class RuleCategoryDescDAO : public DataAccessObject<CatKey, RuleCategoryDescInfo, false>
{
public:
  static RuleCategoryDescDAO& instance();

  const RuleCategoryDescInfo* get(DeleteList& del, const CatNumber& key);

  bool translateKey(const ObjectKey& objectKey, CatKey& key) const override
  {
    return key.initialized = objectKey.getValue("CATNUMBER", key._a);
  }

  CatKey createKey(RuleCategoryDescInfo* info);

  void translateKey(const CatKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CATNUMBER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<RuleCategoryDescInfo, RuleCategoryDescDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RuleCategoryDescDAO>;
  static DAOHelper<RuleCategoryDescDAO> _helper;

  RuleCategoryDescDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CatKey, RuleCategoryDescInfo, false>(cacheSize, cacheType)
  {
  }

  RuleCategoryDescInfo* create(CatKey key) override;
  void destroy(CatKey key, RuleCategoryDescInfo* rec) override;
  virtual void load() override;

private:
  struct groupByKey;
  static RuleCategoryDescDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

