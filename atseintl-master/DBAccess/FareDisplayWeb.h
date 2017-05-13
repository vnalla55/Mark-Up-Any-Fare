//----------------------------------------------------------------------------
// � 2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareDisplayWeb
{
public:
  FareDisplayWeb() : _displayInd(' '), _ruleTariff(0) {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& displayInd() { return _displayInd; }
  const Indicator& displayInd() const { return _displayInd; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  TktCode& tktDesignator() { return _tktDesignator; }
  const TktCode& tktDesignator() const { return _tktDesignator; }

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  FareDisplayWeb& operator=(FareDisplayWeb& web)
  {
    if (&web != this)
    {
      _carrier = web._carrier;
      _displayInd = web._displayInd;
      _vendor = web._vendor;
      _ruleTariff = web._ruleTariff;
      _rule = web._rule;
      _fareClass = web._fareClass;
      _tktDesignator = web._tktDesignator;
      _paxType = web._paxType;
    }
    return *this;
  }

  bool operator==(const FareDisplayWeb& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_displayInd == rhs._displayInd) &&
            (_vendor == rhs._vendor) && (_ruleTariff == rhs._ruleTariff) && (_rule == rhs._rule) &&
            (_fareClass == rhs._fareClass) && (_tktDesignator == rhs._tktDesignator) &&
            (_paxType == rhs._paxType));
  }

  static void dummyData(FareDisplayWeb& obj)
  {
    obj._carrier = "ABC";
    obj._displayInd = 'D';
    obj._vendor = "EFGH";
    obj._ruleTariff = 1;
    obj._rule = "IJKL";
    obj._fareClass = "aaaaaaaa";
    obj._tktDesignator = "bbbbbbbb";
    obj._paxType = "MNO";
  }

private:
  CarrierCode _carrier;
  Indicator _displayInd;
  VendorCode _vendor;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  FareClassCode _fareClass;
  TktCode _tktDesignator;
  PaxTypeCode _paxType; // From FareDisplayWebSegment table

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _displayInd);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _paxType);
  }

};
}

