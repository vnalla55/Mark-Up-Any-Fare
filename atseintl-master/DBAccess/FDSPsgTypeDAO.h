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
class FDSPsgType;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, TJRGroup> FDSPsgTypeKey;

class FDSPsgTypeDAO : public DataAccessObject<FDSPsgTypeKey, std::vector<FDSPsgType*>, false>
{
public:
  static FDSPsgTypeDAO& instance();

  const std::vector<FDSPsgType*>& get(DeleteList& del,
                                      const Indicator& userApplType,
                                      const UserApplCode& userAppl,
                                      const Indicator& pseudoCityType,
                                      const PseudoCityCode& pseudoCity,
                                      const TJRGroup& tjrGroup,
                                      const Indicator& fareDisplayType,
                                      const Indicator& domIntlAppl,
                                      const uint64_t& seqno);

  bool translateKey(const ObjectKey& objectKey, FDSPsgTypeKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FDSPsgTypeKey createKey(FDSPsgType* info);

  void translateKey(const FDSPsgTypeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FDSPsgType, FDSPsgTypeDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDSPsgTypeDAO>;

  static DAOHelper<FDSPsgTypeDAO> _helper;

  FDSPsgTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDSPsgTypeKey, std::vector<FDSPsgType*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<FDSPsgType*>* create(FDSPsgTypeKey key) override;
  void destroy(FDSPsgTypeKey key, std::vector<FDSPsgType*>* recs) override;

  virtual void load() override;

private:
  static FDSPsgTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

