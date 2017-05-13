//----------------------------------------------------------------------------
//  File: MsgTextDAO.h
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
#include "DBAccess/FareCalcConfigText.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

typedef HashKey<Indicator, UserApplCode, PseudoCityCode> MsgTextKey;

class MsgTextDAO : public DataAccessObject<MsgTextKey, FareCalcConfigText>
{
public:
  static MsgTextDAO& instance();

  FareCalcConfigText& get(DeleteList& del,
                          const Indicator msgType,
                          const UserApplCode& keyword1,
                          const PseudoCityCode& keyword2);

  bool translateKey(const ObjectKey& objectKey, MsgTextKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("PSEUDOCITY", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<MsgTextDAO>;

  static DAOHelper<MsgTextDAO> _helper;

  MsgTextDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MsgTextKey, FareCalcConfigText>(cacheSize, cacheType)
  {
  }

  FareCalcConfigText* create(MsgTextKey key) override;
  void destroy(MsgTextKey key, FareCalcConfigText* rec) override;

private:
  // struct isEffective;
  static MsgTextDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // End of namespace tse

