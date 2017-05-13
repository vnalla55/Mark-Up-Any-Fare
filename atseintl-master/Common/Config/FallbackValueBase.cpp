//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#ifndef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/FallbackValueBase.h"

#include "Common/Config/ConfigMan.h" // sfc
#include "Common/HandlesHolder.h"
#include "Common/Logger.h"

#include <cstdio> // should be in log4cxx
#include <functional>

namespace tse
{

namespace fallback
{

namespace
{

typedef HandlesHolder<FallbackValueBase> Pool;

Pool&
getPool()
{
  static Pool pool;
  return pool;
}

} // namespace

Logger
FallbackValueBase::_logger("atseintl.Server.TseServer");

FallbackValueBase::FallbackValueBase() { getPool().insert(this); }

FallbackValueBase::~FallbackValueBase() { getPool().erase(this); }

void
FallbackValueBase::loggerMessage(const std::string& option, bool value) const
{
  LOG4CXX_INFO(_logger, "Setting " << option << " - " << (value ? "On" : "Off"));
}

void
configure(const tse::ConfigMan& config)
{
  typedef std::mem_fun1_t<void, FallbackValueBase, const tse::ConfigMan*> Method;
  getPool().forEach(std::bind2nd(Method(&FallbackValueBase::set), &config));
}

} // fallback

} // tse

#endif
