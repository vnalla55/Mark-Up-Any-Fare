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
class FDSGlobalDir;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, TJRGroup> FDSGlobalDirKey;

class FDSGlobalDirDAO : public DataAccessObject<FDSGlobalDirKey, std::vector<FDSGlobalDir*>, false>
{
public:
  static FDSGlobalDirDAO& instance();

  const std::vector<FDSGlobalDir*>& get(DeleteList& del,
                                        const Indicator& userApplType,
                                        const UserApplCode& userAppl,
                                        const Indicator& pseudoCityType,
                                        const PseudoCityCode& pseudoCity,
                                        const TJRGroup& tjrGroup,
                                        const Indicator& fareDisplayType,
                                        const Indicator& domIntlAppl,
                                        const DateTime& versionDate,
                                        const uint64_t& seqno,
                                        const DateTime& createDate);

  bool translateKey(const ObjectKey& objectKey, FDSGlobalDirKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FDSGlobalDirKey createKey(FDSGlobalDir* info);

  void translateKey(const FDSGlobalDirKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FDSGlobalDir, FDSGlobalDirDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDSGlobalDirDAO>;

  static DAOHelper<FDSGlobalDirDAO> _helper;

  FDSGlobalDirDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDSGlobalDirKey, std::vector<FDSGlobalDir*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<FDSGlobalDir*>* create(FDSGlobalDirKey key) override;
  void destroy(FDSGlobalDirKey key, std::vector<FDSGlobalDir*>* recs) override;

  virtual void load() override;

private:
  static FDSGlobalDirDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

