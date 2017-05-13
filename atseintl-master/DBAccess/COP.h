#pragma once
//----------------------------------------------------------------------------
// COP.h
//
// Copyright Sabre 2004
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class COP
{
public:
  COP()
    : _copLocType(' '),
      _travelAppl(' '),
      _participationind(' '),
      _puNormalSpecialType(' '),
      _puTripType(' '),
      _puAppl(' '),
      _puOrigLocType(' '),
      _puWithinLocType(' '),
      _consPointLocType(' ')
  {
  }

  Indicator& travelAppl() { return _travelAppl; }
  const Indicator& travelAppl() const { return _travelAppl; }

  CarrierCode& copCarrier() { return _copCarrier; }
  const CarrierCode& copCarrier() const { return _copCarrier; }

  Indicator& puNormalSpecialType() { return _puNormalSpecialType; }
  const Indicator& puNormalSpecialType() const { return _puNormalSpecialType; }

  Indicator& puTripType() { return _puTripType; }
  const Indicator& puTripType() const { return _puTripType; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  Indicator& puAppl() { return _puAppl; }
  const Indicator& puAppl() const { return _puAppl; }

  LocCode& puOrigLoc() { return _puOrigLoc; }
  const LocCode& puOrigLoc() const { return _puOrigLoc; }

  LocCode& puWithinLoc() { return _puWithinLoc; }
  const LocCode& puWithinLoc() const { return _puWithinLoc; }

  std::vector<CarrierCode>& tktgCarrier() { return _tktgCarrier; }
  const std::vector<CarrierCode>& tktgCarrier() const { return _tktgCarrier; }

  bool operator==(const COP& rhs) const
  {
    return ((_copLocType == rhs._copLocType) && (_copLoc == rhs._copLoc) &&
            (_travelAppl == rhs._travelAppl) && (_copCarrier == rhs._copCarrier) &&
            (_participationind == rhs._participationind) &&
            (_puNormalSpecialType == rhs._puNormalSpecialType) &&
            (_puTripType == rhs._puTripType) && (_fareType == rhs._fareType) &&
            (_puAppl == rhs._puAppl) && (_puOrigLocType == rhs._puOrigLocType) &&
            (_puOrigLoc == rhs._puOrigLoc) && (_puWithinLocType == rhs._puWithinLocType) &&
            (_puWithinLoc == rhs._puWithinLoc) && (_consPointLocType == rhs._consPointLocType) &&
            (_consPointLoc == rhs._consPointLoc) && (_tktgCarrier == rhs._tktgCarrier));
  }

private:
  Indicator _copLocType;
  LocCode _copLoc;
  Indicator _travelAppl;
  CarrierCode _copCarrier;
  Indicator _participationind;
  Indicator _puNormalSpecialType;
  Indicator _puTripType;
  FareType _fareType;
  Indicator _puAppl;
  Indicator _puOrigLocType;
  LocCode _puOrigLoc;
  Indicator _puWithinLocType;
  LocCode _puWithinLoc;
  Indicator _consPointLocType;
  LocCode _consPointLoc;
  std::vector<CarrierCode> _tktgCarrier;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _copLocType);
    FLATTENIZE(archive, _copLoc);
    FLATTENIZE(archive, _travelAppl);
    FLATTENIZE(archive, _copCarrier);
    FLATTENIZE(archive, _participationind);
    FLATTENIZE(archive, _puNormalSpecialType);
    FLATTENIZE(archive, _puTripType);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _puAppl);
    FLATTENIZE(archive, _puOrigLocType);
    FLATTENIZE(archive, _puOrigLoc);
    FLATTENIZE(archive, _puWithinLocType);
    FLATTENIZE(archive, _puWithinLoc);
    FLATTENIZE(archive, _consPointLocType);
    FLATTENIZE(archive, _consPointLoc);
    FLATTENIZE(archive, _tktgCarrier);
  }

private:
};


} // namespace tse

