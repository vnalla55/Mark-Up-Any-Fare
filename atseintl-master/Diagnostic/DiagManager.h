//----------------------------------------------------------------------------
//  File:         DiagManager.h
//  Description:  Diagnostic manager object, used to encapsulate the work
//                of creating and destroying DiagCollector objects based
//                on whether or not they are needed
//  Authors:      David White
//  Created:      Mar 2005
//
//  Copyright Sabre 2005
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

#include "Common/Assert.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"

namespace tse
{
// An object of this class should be created at the start of a block of code
// which requires diagnostic output. The activate() function should be called
// for each diagnostic number that is used in the function.
//
// When a section of code is entered that uses a specific diagnostic, the
// isActive() function should be used to see if a diagnostic should actually
// be output. If isActive() is true, then the collector may be retrieved
// using collector() and the diagnostic output.
//
// Note that this object is inexpensive to create and destroy unless a
// diagnostic is actually activated. It is thus efficient to use it in cases
// where a diagnostic will be used only infrequently.

class DiagManager final
{
public:
  DiagManager() = delete;
  DiagManager(const DiagManager& other) = delete;
  explicit DiagManager(Trx& trx) : _trx(trx) {}
  DiagManager(Trx& trx, DiagnosticTypes type) : _trx(trx) { activate(type); }
  DiagManager(Trx& trx, DiagnosticTypes type, const std::string& diagParam)
    : _trx(trx) { activate(type, diagParam); }
  DiagManager(DiagManager&& other) noexcept
    : _trx(other._trx), _collector(other._collector), _active(other._active)
  {
    other._collector = nullptr;
    other._active = false;
  }

  ~DiagManager()
  {
    if (_collector)
      _collector->flushMsg();
  }

  DiagManager& operator=(const DiagManager& other) = delete;
  DiagManager& operator=(DiagManager&& other) = delete;

  DiagManager& activate(DiagnosticTypes type)
  {
    if (_trx.diagnostic().diagnosticType() == type)
      _active = true;
    return *this;
  }

  DiagManager& activate(DiagnosticTypes type, const std::string& diagParam)
  {
    if (_trx.diagnostic().diagParamMapItemPresent(diagParam))
      return activate(type);
    return *this;
  }

  void deActivate() { _active = false; }
  bool isActive() const { return UNLIKELY(_active); }

  DiagCollector& collector();

private:
  Trx& _trx;
  DiagCollector* _collector = nullptr;
  bool _active = false;
};

inline DiagCollector&
DiagManager::collector()
{
  TSE_ASSERT(_active);
  if (!_collector)
  {
    _collector = DCFactory::instance()->create(_trx);
    _collector->enable(_trx.diagnostic().diagnosticType());
  }
  return *_collector;
}

template <typename T>
inline DiagManager& operator<<(DiagManager& man, const T& t)
{
  if (UNLIKELY(man.isActive()))
    man.collector() << t;
  return man;
}

// for std::endl and std::flush
inline DiagManager& operator<<(DiagManager& man, std::ostream&(*os)(std::ostream&))
{
  if (UNLIKELY(man.isActive()))
  {
    man.collector() << os;
    man.collector().flushMsg();
  }
  return man;
}

}
