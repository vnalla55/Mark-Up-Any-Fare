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
class FareDisplaySort;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, TJRGroup, Indicator, Indicator>
FareDisplaySortKey;

class FareDisplaySortDAO
    : public DataAccessObject<FareDisplaySortKey, std::vector<FareDisplaySort*>, false>
{
public:
  static FareDisplaySortDAO& instance();

  const std::vector<FareDisplaySort*>& get(DeleteList& del,
                                           const Indicator& userApplType,
                                           const UserApplCode& userAppl,
                                           const Indicator& pseudoCityType,
                                           const PseudoCityCode& pseudoCity,
                                           const TJRGroup& tjrGroup);

  bool translateKey(const ObjectKey& objectKey, FareDisplaySortKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FareDisplaySortKey createKey(FareDisplaySort* info);

  void translateKey(const FareDisplaySortKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDisplaySort, FareDisplaySortDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDisplaySortDAO>;

  static DAOHelper<FareDisplaySortDAO> _helper;

  FareDisplaySortDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDisplaySortKey, std::vector<FareDisplaySort*>, false>(cacheSize,
                                                                                 cacheType)
  {
  }
  virtual void load() override;
  std::vector<FareDisplaySort*>* create(FareDisplaySortKey key) override;
  void destroy(FareDisplaySortKey key, std::vector<FareDisplaySort*>* recs) override;

private:
  static FareDisplaySortDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

