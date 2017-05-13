//----------------------------------------------------------------------------
//  File: MsgTextDAO.cpp
//
//  Author:
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
#include "DBAccess/MsgTextDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetMsgText.h"

namespace tse
{
log4cxx::LoggerPtr
MsgTextDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MsgTextDAO"));

std::string
MsgTextDAO::_name("MsgText");
std::string
MsgTextDAO::_cacheClass("FareCalc");

DAOHelper<MsgTextDAO>
MsgTextDAO::_helper(_name);

MsgTextDAO* MsgTextDAO::_instance = nullptr;

// ----------------------------------------------------------------
//   @method instance
//
//   Description: Gets the reference to MsgTextDAO object.
//
//   @return Reference to MsgTextDAO object.
//
// ----------------------------------------------------------------

MsgTextDAO&
MsgTextDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const FareCalcConfigText&
getMsgTextData(const Indicator userApplType,
               const UserApplCode& userAppl,
               const PseudoCityCode& pseudoCity,
               DeleteList& deleteList)
{
  MsgTextDAO& dao = MsgTextDAO::instance();
  return dao.get(deleteList, userApplType, userAppl, pseudoCity);
}

// ----------------------------------------------------------------
//   @method	Get
//   @param del								- Delete list
//
//   @return Vector of MsgText objects which matches criteria.
//
// ----------------------------------------------------------------

FareCalcConfigText&
MsgTextDAO::get(DeleteList& del,
                const Indicator userApplType,
                const UserApplCode& userAppl,
                const PseudoCityCode& pseudoCity)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MsgTextKey key(userApplType, userAppl, pseudoCity);

  DAOCache::pointer_type ptr = cache().get(key);

  del.copy(ptr);

  return *ptr;
}

// ----------------------------------------------------------------
//   @method create
//   @param key				- Key value ( MsgItemNo )
//
//   Description:
//
//   @return Vector of std::string objects
//
// ----------------------------------------------------------------

FareCalcConfigText*
MsgTextDAO::create(MsgTextKey key)
{

  LOG4CXX_INFO(_logger, "Entering create");

  FareCalcConfigText* ret = new FareCalcConfigText;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMsgText mt(dbAdapter->getAdapter());
    mt.findMsgText(ret->fccTextMap(), key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MsgTextDAO::Get");
    throw;
  }

  LOG4CXX_INFO(_logger, "Leaving create with ret:" << ret);
  return ret;
}

void
MsgTextDAO::destroy(MsgTextKey key, FareCalcConfigText* rec)
{
  delete rec;
}

} // namespace tse
