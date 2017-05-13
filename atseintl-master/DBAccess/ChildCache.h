// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

namespace tse {

template <class Key>
class ChildCache
{
public:
  virtual void keyRemoved(const Key& key) = 0;
  virtual void cacheCleared() = 0;
  virtual void notifierDestroyed() {}
  virtual ~ChildCache() {};
};

}

