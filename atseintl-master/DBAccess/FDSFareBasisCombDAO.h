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
class FDSFareBasisComb;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, Indicator, PseudoCityCode, TJRGroup> FDSFareBasisCombKey;

class FDSFareBasisCombDAO
    : public DataAccessObject<FDSFareBasisCombKey, std::vector<FDSFareBasisComb*>, false>
{
public:
  static FDSFareBasisCombDAO& instance();

  const std::vector<FDSFareBasisComb*>& get(DeleteList& del,
                                            const Indicator& userApplType,
                                            const UserApplCode& userAppl,
                                            const Indicator& pseudoCityType,
                                            const PseudoCityCode& pseudoCity,
                                            const TJRGroup& tjrGroup,
                                            const Indicator& fareDisplayType,
                                            const Indicator& domIntlAppl,
                                            const uint64_t& seqno);

  bool translateKey(const ObjectKey& objectKey, FDSFareBasisCombKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._c) &&
                             objectKey.getValue("PSEUDOCITY", key._d) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FDSFareBasisCombKey createKey(FDSFareBasisComb* info);

  void translateKey(const FDSFareBasisCombKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("PSEUDOCITYTYPE", key._c);
    objectKey.setValue("PSEUDOCITY", key._d);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FDSFareBasisComb, FDSFareBasisCombDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDSFareBasisCombDAO>;

  static DAOHelper<FDSFareBasisCombDAO> _helper;

  FDSFareBasisCombDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDSFareBasisCombKey, std::vector<FDSFareBasisComb*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<FDSFareBasisComb*>* create(FDSFareBasisCombKey key) override;
  void destroy(FDSFareBasisCombKey key, std::vector<FDSFareBasisComb*>* recs) override;

  virtual void load() override;

private:
  static FDSFareBasisCombDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

