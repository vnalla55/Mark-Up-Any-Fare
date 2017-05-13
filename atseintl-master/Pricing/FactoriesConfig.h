// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         18-11-2011
//! \file         FactoriesConfig.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include <boost/noncopyable.hpp>

#include <stdint.h>

namespace tse
{

class ConfigMan;

class FactoriesConfig : private boost::noncopyable
{
public:
  explicit FactoriesConfig(const tse::ConfigMan& config);

  ~FactoriesConfig() = default;

  int32_t multiPaxShortCktTimeOut() const { return _multiPaxShortCktTimeOut; }

  bool puScopeValidationEnabled() const { return _puScopeValidationEnabled; }

  bool eoeTuningEnabled() const { return _eoeTuningEnabled; }

  bool searchAlwaysLowToHigh() const { return _searchAlwaysLowToHigh; }

  bool enableCxrFareSearchTuning() const { return _enableCxrFareSearchTuning; }

  bool avoidStaticObjectPool() const { return _avoidStaticObjectPool; }

  uint32_t maxNbrCombMsgThreshold() const { return _maxNbrCombMsgThreshold; }

  int32_t globalPushBackMax() const { return _globalPushBackMax; }

  int32_t plusUpPushBackMax() const { return _plusUpPushBackMax; }

  int32_t plusUpPushBackThreshold() const { return _plusUpPushBackThreshold; }

protected:
  FactoriesConfig(); // for unit tests

  void setSearchAlwaysLowToHighForUnitTestsOnly(const bool value)
  {
    _searchAlwaysLowToHigh = value;
  }

private:
  uint32_t _maxNbrCombMsgThreshold;
  int32_t _multiPaxShortCktTimeOut; // In case of Multi-Pax Trx, short-ckt after 30 sec
  int32_t _plusUpPushBackThreshold; // negative value means ignore it
  int32_t _plusUpPushBackMax; // negative value means ignore it
  int32_t _globalPushBackMax; // for all FPFactory of a PaxType combined limit,
  // negative value means ignore it

  bool _puScopeValidationEnabled;
  bool _eoeTuningEnabled;
  bool _searchAlwaysLowToHigh;
  bool _enableCxrFareSearchTuning;
  bool _avoidStaticObjectPool;
};

} /* namespace tse */
