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
class FareDisplayPref;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, TJRGroup> FareDisplayPrefKey;

class FareDisplayPrefDAO
    : public DataAccessObject<FareDisplayPrefKey, std::vector<FareDisplayPref*>, false>
{
public:
  static FareDisplayPrefDAO& instance();

  const std::vector<FareDisplayPref*>& get(DeleteList& del,
                                           const Indicator& userApplType,
                                           const UserApplCode& userAppl,
                                           const Indicator& pseudoCityType,
                                           const PseudoCityCode& pseudoCity,
                                           const TJRGroup& tjrGroup);

  bool translateKey(const ObjectKey& objectKey, FareDisplayPrefKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FareDisplayPrefKey createKey(FareDisplayPref* info);

  void translateKey(const FareDisplayPrefKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDisplayPref, FareDisplayPrefDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDisplayPrefDAO>;

  static DAOHelper<FareDisplayPrefDAO> _helper;

  FareDisplayPrefDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDisplayPrefKey, std::vector<FareDisplayPref*>, false>(
          cacheSize, cacheType, 2)
  {
  }

  std::vector<FareDisplayPref*>* create(FareDisplayPrefKey key) override;
  void destroy(FareDisplayPrefKey key, std::vector<FareDisplayPref*>* recs) override;

  virtual void load() override;

private:
  static FareDisplayPrefDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

