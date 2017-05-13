//-------------------------------------------------------------------
//
//  Description: Boolean Flag Resetter
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <boost/noncopyable.hpp>

namespace tse
{

class BooleanFlagResetter
{
  bool& _flag;

public:
  BooleanFlagResetter(bool& flag) : _flag(flag) {}
  ~BooleanFlagResetter()
  {
    if (_flag)
    {
      _flag = false;
    }
  }
};

struct TurnFlagOn
{
  void operator()(bool& flag)
  {
    if (!flag)
      flag = true;
  }
};

struct TurnFlagOff
{
  void operator()(bool& flag)
  {
    if (flag)
      flag = false;
  }
};

template <typename T, typename P>
class RAIIImpl : boost::noncopyable
{
  T& _resource;
  P _predicate;

public:
  explicit RAIIImpl(T& resource) : _resource(resource) {}
  ~RAIIImpl() { _predicate(_resource); }
};

typedef RAIIImpl<bool, TurnFlagOn> BoolRAIISwitchTrue;
typedef RAIIImpl<bool, TurnFlagOff> BoolRAIISwitchFalse;

struct ConditionalTurnFlagOn
{
  bool shouldSwitch(bool& flag) { return !flag; }

  void switchValue(bool& flag) { flag = true; }

  void restoreValue(bool& flag) { flag = false; }
};

template <typename T, typename P>
class ConditionalRAIIImpl : boost::noncopyable
{
  T& _resource;
  P _predicate;
  bool _shouldRestore;

public:
  explicit ConditionalRAIIImpl(T& resource) : _resource(resource), _shouldRestore(false)
  {
    if (_predicate.shouldSwitch(_resource))
    {
      _shouldRestore = true;
      _predicate.switchValue(_resource);
    }
  }
  ~ConditionalRAIIImpl()
  {
    if (_shouldRestore)
      _predicate.restoreValue(_resource);
  }
};

typedef ConditionalRAIIImpl<bool, ConditionalTurnFlagOn> BoolConditionalRAIISwitchTrue;

template <typename T>
class RaiiWithMemory : boost::noncopyable
{
  T& _resource;
  T _memory;

public:
 RaiiWithMemory(T& resource, const T& target) :
  _resource(resource), _memory(resource)
  {
    _resource = target;
  }
  ~RaiiWithMemory() { _resource = _memory; }
};

} // End of namespace tse

