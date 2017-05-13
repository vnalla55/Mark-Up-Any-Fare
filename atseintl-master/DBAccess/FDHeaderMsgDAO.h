//----------------------------------------------------------------------------
//  File: FDHeaderMsgDAO.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDHeaderMsg.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
// TODO: remove any mention of inclusionCode in this class.
// It was once a key but not anymore. Change FDHeaderMsgController as well.
typedef HashKey<PseudoCityCode, PseudoCityType, Indicator, std::string, TJRGroup> FDHeaderMsgKey;

class FDHeaderMsgDAO : public DataAccessObject<FDHeaderMsgKey, std::vector<const FDHeaderMsg*> >
{
public:
  static FDHeaderMsgDAO& instance();

  std::vector<const tse::FDHeaderMsg*>& get(DeleteList& del,
                                            const PseudoCityCode& pseudoCityCode,
                                            const PseudoCityType& pseudoCityType,
                                            const Indicator& userApplType,
                                            const std::string& userAppl,
                                            const TJRGroup& tjrGroup,
                                            const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, FDHeaderMsgKey& key) const override
  {
    // note: the cache notification keys are written in the db order
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a) &&
                             objectKey.getValue("PSEUDOCITYTYPE", key._b) &&
                             objectKey.getValue("USERAPPL", key._d) &&
                             objectKey.getValue("USERAPPLTYPE", key._c) &&
                             objectKey.getValue("SSGGROUPNO", key._e);
  }

  FDHeaderMsgKey createKey(const FDHeaderMsg* info);

  void translateKey(const FDHeaderMsgKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITY", key._a);
    objectKey.setValue("PSEUDOCITYTYPE", key._b);
    objectKey.setValue("USERAPPL", key._d);
    objectKey.setValue("USERAPPLTYPE", key._c);
    objectKey.setValue("SSGGROUPNO", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FDHeaderMsg, FDHeaderMsgDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDHeaderMsgDAO>;

  static DAOHelper<FDHeaderMsgDAO> _helper;

  FDHeaderMsgDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDHeaderMsgKey, std::vector<const FDHeaderMsg*> >(cacheSize, cacheType)
  {
  }

  void load() override;

  std::vector<const tse::FDHeaderMsg*>* create(FDHeaderMsgKey key) override;

  void destroy(FDHeaderMsgKey key, std::vector<const tse::FDHeaderMsg*>* t) override;

private:
  struct isEffective;
  static FDHeaderMsgDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // End of namespace tse
