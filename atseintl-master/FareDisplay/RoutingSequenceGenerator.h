//----------------------------------------------------------------------------
//   File : RoutingSequenceGenerator.cpp
//   Author: Abu
//   Copyright Sabre 2004
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "Routing/RoutingConsts.h"


#include <list>
#include <map>

namespace tse
{
/**
 *@class RoutingSequenceGenerator.
 * generates RoutingSequence for a given fare group
 *
 */

class PaxTypeFare;

class RoutingSequenceGenerator
{
  friend class RoutingSequenceGeneratorTest;

public:
  RoutingSequenceGenerator();
  virtual ~RoutingSequenceGenerator();
  bool generateRoutingSequence(std::list<PaxTypeFare*>& fares);

  // TODO:: We need to remove this method after restructuring of AddOn
  void generate(std::vector<AddonFareInfo*>& addOnFareInfoList,
                std::map<const AddonFareInfo*, std::string>& addOnRoutingSeq,
                const RecordScope& crossRefType) const;

  void
  generate(std::vector<FDAddOnFareInfo*>& addOnFareInfoList, const RecordScope& crossRefType) const;

  std::string getRTGSeq(const GlobalDirection& curentGlobal, uint16_t counter = 1) const;

private:
  static const uint16_t FILLER_ZERO = 0;
  static const uint16_t TWO_DIGIT = 10;

  class RoutingSequence
  {
  public:
    RoutingSequence(GlobalDirection g,
                    const RoutingNumber& r,
                    const RoutingNumber& origAddonRtgNum,
                    const RoutingNumber& destAddonRtgNum,
                    const CarrierCode& c,
                    const VendorCode& v)
      : _globalDirection(g),
        _routingNumber(r),
        _origAddonRtgNum(origAddonRtgNum),
        _destAddonRtgNum(destAddonRtgNum),
        _carrier(c),
        _vendor(v)
    {
    }
    //~RoutingSequence() {};

    GlobalDirection _globalDirection;
    RoutingNumber _routingNumber;
    RoutingNumber _origAddonRtgNum;
    RoutingNumber _destAddonRtgNum;
    CarrierCode _carrier;
    VendorCode _vendor;

    bool operator<(const RoutingSequence& key) const
    {
      if (this->_globalDirection < key._globalDirection)
        return true;
      if (this->_globalDirection > key._globalDirection)
        return false;
      if (this->_routingNumber < key._routingNumber)
        return true;
      if (this->_routingNumber > key._routingNumber)
        return false;
      if (this->_origAddonRtgNum < key._origAddonRtgNum)
        return true;
      if (this->_origAddonRtgNum > key._origAddonRtgNum)
        return false;
      if (this->_destAddonRtgNum < key._destAddonRtgNum)
        return true;
      if (this->_destAddonRtgNum > key._destAddonRtgNum)
        return false;
      if (key.isMileage())
      {
        return false;
      }
      else
      {
        if (this->_carrier < key._carrier)
          return true;
        if (this->_carrier > key._carrier)
          return false;
        if (this->_vendor < key._vendor)
          return true;
        if (this->_vendor > key._vendor)
          return false;
      }
      return false;
    }

    bool isMileage() const { return _routingNumber == MILEAGE_ROUTING; }
  };

  void generate(std::list<PaxTypeFare*>& fares,
                std::map<RoutingSequence, std::string>&,
                std::map<GlobalDirection, uint16_t>&);

  void getRoutingOrder(std::string& order, uint16_t count) const;

  std::map<RoutingSequence, std::string> _generatedSequence;
  std::map<GlobalDirection, uint16_t> _processedGlobals;
};
} // namespace tse

