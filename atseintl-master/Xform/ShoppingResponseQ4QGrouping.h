// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         04-03-2013
//! \file         ShoppingResponseQ4QGrouping.h
//! \brief
//!
//!  Copyright (C) Sabre 2013
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{
class ShoppingTrx;
class ConfigMan;
class Logger;

namespace details
{
class ConfigQ4Q
{
public:
  bool isGroupItin() const { return _groupItin; }
  bool isGroupInterlineItin() const { return _groupInterlineItin; }
  int getMaxGroup() const { return _maxGroup; }
  int getGroupMaxItin() const { return _groupMaxItin; }

  ConfigQ4Q();

  void initialize(const ConfigMan& config, const ShoppingTrx& shoppingTrx, Logger& logger);

private:
  bool _initialized;
  bool _groupItin;
  bool _groupInterlineItin;
  int _maxGroup;
  int _groupMaxItin;
};

struct GroupItn
{
  GroupItn() : _groupNumber(0), _groupSize(0) {}
  int _groupNumber;
  int _groupSize;
};
}

class ShoppingResponseQ4QGrouping
{
public:
  typedef const std::vector<int> SopIndexVector;

  explicit ShoppingResponseQ4QGrouping(const ShoppingTrx* const shoppingTrx);

  int16_t generateQ4Q(const SopIndexVector& sops, std::string& mapKey);

private:
  void updateMapKeyForAltDateTrx(const SopIndexVector& sops, std::string& /* in-out*/ mapKey) const;

  void updateMapKeyForGuaranteedTicketingCarriers(const SopIndexVector& sops,
                                                  std::string& /* in-out*/ mapKey) const;

  bool isInterlineGroupingForcedForAltDates() const;

  uint32_t getFirstInterlineSopIndex(const CarrierCode& carrier, const SopIndexVector& sops) const;

  int16_t findOrCreateGroupForKey(const SopIndexVector& sops, const std::string& mapKey);

  typedef details::GroupItn GroupItn;
  typedef details::ConfigQ4Q ConfigQ4Q;
  typedef std::map<std::string, GroupItn> CarrierGroupMap;

  const ShoppingTrx* const _shoppingTrx;
  ConfigQ4Q _configQ4Q;
  int16_t _carrierGroupIndex;
  CarrierGroupMap _carrierGroupMap;

  static Logger _logger;
};

} /* namespace tse */
