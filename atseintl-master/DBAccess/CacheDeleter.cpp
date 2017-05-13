// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DBAccess/CacheDeleter.h"

#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheRegistry.h"

namespace tse
{

CacheDeleterBase::CacheDeleterBase() { TrxCounter::registerObserver(*this); }
CacheDeleterBase::~CacheDeleterBase() { TrxCounter::unregisterObserver(*this); }

namespace
{
class EmptyTrash
{
public:
  void operator()(std::pair<const std::string, CacheControl*>& p)
  {
    CacheControl* ctl(p.second);
    if (LIKELY(ctl != nullptr))
    {
      ctl->emptyTrash();
    }
  }
};
}

void
CacheDeleterBase::emptyTrash()
{
  CacheRegistry& registry(CacheRegistry::instance());
  EmptyTrash emptyTrash;
  registry.forEach(emptyTrash);
}

}
