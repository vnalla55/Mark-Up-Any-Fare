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

#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <algorithm>
#include <set>

namespace tse
{

template <typename T>
class HandlesHolder : private std::set<T*>
{
public:
  void insert(T* value)
  {
    Guard guard(_mutex);
    Set::insert(value);
  }

  void erase(T* value)
  {
    Guard guard(_mutex);
    Set::erase(value);
  }

  template <typename O>
  void forEach(O operation) const
  {
    Guard guard(_mutex);
    std::for_each(this->begin(), this->end(), operation);
  }

private:
  typedef std::set<T*> Set;
  typedef boost::lock_guard<boost::mutex> Guard;

  mutable boost::mutex _mutex;
};

} // tse


