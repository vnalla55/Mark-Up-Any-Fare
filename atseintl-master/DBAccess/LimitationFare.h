#pragma once
//----------------------------------------------------------------------------
// LimitationFare.h
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/LimitFareCxrLoc.h"

#include <vector>

namespace tse
{

class LimitationFare : public LimitationCmn
{
public:
  LimitationFare() = default;

  ~LimitationFare()
  {
    std::vector<LimitFareCxrLoc*>::iterator LocIt;
    for (LocIt = _exceptViaCxrs.begin(); LocIt != _exceptViaCxrs.end(); LocIt++)
    { // Nuke 'em!
      delete *LocIt;
    }
  }

  Indicator& govCarrierAppl() { return _govCarrierAppl; }
  const Indicator& govCarrierAppl() const { return _govCarrierAppl; }

  std::vector<CarrierCode>& govCarriers() { return _govCarriers; }
  const std::vector<CarrierCode>& govCarriers() const { return _govCarriers; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  Indicator& fareComponentAppl() { return _fareComponentAppl; }
  const Indicator& fareComponentAppl() const { return _fareComponentAppl; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Indicator& mustNotViaHip() { return _mustNotViaHip; }
  const Indicator& mustNotViaHip() const { return _mustNotViaHip; }

  Indicator& confirmedAppl() { return _confirmedAppl; }
  const Indicator& confirmedAppl() const { return _confirmedAppl; }

  Indicator& maxDomSegments() { return _maxDomSegments; }
  const Indicator& maxDomSegments() const { return _maxDomSegments; }

  Indicator& exceptViaCxrLocInd() { return _exceptViaCxrLocInd; }
  const Indicator& exceptViaCxrLocInd() const { return _exceptViaCxrLocInd; }

  std::vector<LimitFareCxrLoc*>& exceptViaCxrs() { return _exceptViaCxrs; }
  const std::vector<LimitFareCxrLoc*>& exceptViaCxrs() const { return _exceptViaCxrs; }

  LocKey& notViaLoc() { return _notViaLoc; }
  const LocKey& notViaLoc() const { return _notViaLoc; }

  Indicator& viaGovCarrierInd() { return _viaGovCarrierInd; }
  const Indicator& viaGovCarrierInd() const { return _viaGovCarrierInd; }

  LocKey& notViaToFromLoc() { return _notViaToFromLoc; }
  const LocKey& notViaToFromLoc() const { return _notViaToFromLoc; }

  Indicator& retransitGovCxrAppl() { return _retransitGovCxrAppl; }
  const Indicator& retransitGovCxrAppl() const { return _retransitGovCxrAppl; }

  std::vector<RoutingNumber>& routings() { return _routings; }
  const std::vector<RoutingNumber>& routings() const { return _routings; }

  bool operator==(const LimitationFare& rhs) const
  {
    bool eq = ((LimitationCmn::operator==(rhs)) && (_govCarrierAppl == rhs._govCarrierAppl) &&
               (_govCarriers == rhs._govCarriers) && (_directionality == rhs._directionality) &&
               (_fareComponentAppl == rhs._fareComponentAppl) && (_loc1 == rhs._loc1) &&
               (_loc2 == rhs._loc2) && (_globalDir == rhs._globalDir) &&
               (_mustNotViaHip == rhs._mustNotViaHip) && (_confirmedAppl == rhs._confirmedAppl) &&
               (_maxDomSegments == rhs._maxDomSegments) &&
               (_exceptViaCxrLocInd == rhs._exceptViaCxrLocInd) &&
               (_exceptViaCxrs.size() == rhs._exceptViaCxrs.size()) &&
               (_notViaLoc == rhs._notViaLoc) && (_viaGovCarrierInd == rhs._viaGovCarrierInd) &&
               (_notViaToFromLoc == rhs._notViaToFromLoc) &&
               (_retransitGovCxrAppl == rhs._retransitGovCxrAppl) && (_routings == rhs._routings));

    for (size_t i = 0; (eq && (i < _exceptViaCxrs.size())); ++i)
    {
      eq = (*(_exceptViaCxrs[i]) == *(rhs._exceptViaCxrs[i]));
    }

    return eq;
  }

  static void dummyData(LimitationFare& obj)
  {
    LimitationCmn::dummyData(obj);
    obj._govCarrierAppl = 'A';
    obj._govCarriers.push_back("BCD");
    obj._govCarriers.push_back("EFG");
    obj._directionality = 'H';
    obj._fareComponentAppl = 'I';
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
    obj._globalDir = GlobalDirection::US;
    obj._mustNotViaHip = 'J';
    obj._confirmedAppl = 'K';
    obj._maxDomSegments = 'L';
    obj._exceptViaCxrLocInd = 'M';

    LimitFareCxrLoc* lfcl1 = new LimitFareCxrLoc;
    LimitFareCxrLoc* lfcl2 = new LimitFareCxrLoc;

    LimitFareCxrLoc::dummyData(*lfcl1);
    LimitFareCxrLoc::dummyData(*lfcl2);

    obj._exceptViaCxrs.push_back(lfcl1);
    obj._exceptViaCxrs.push_back(lfcl2);

    LocKey::dummyData(obj._notViaLoc);
    obj._viaGovCarrierInd = 'N';
    LocKey::dummyData(obj._notViaToFromLoc);
    obj._retransitGovCxrAppl = 'O';

    obj._routings.push_back("PQRS");
    obj._routings.push_back("TUVW");
  }

private:
  Indicator _govCarrierAppl = ' ';
  std::vector<CarrierCode> _govCarriers;
  Indicator _directionality = ' ';
  Indicator _fareComponentAppl = ' ';
  LocKey _loc1;
  LocKey _loc2;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  Indicator _mustNotViaHip = ' ';
  Indicator _confirmedAppl = ' ';
  Indicator _maxDomSegments = ' ';
  Indicator _exceptViaCxrLocInd = ' ';
  std::vector<LimitFareCxrLoc*> _exceptViaCxrs;
  LocKey _notViaLoc;
  Indicator _viaGovCarrierInd = ' ';
  LocKey _notViaToFromLoc;
  Indicator _retransitGovCxrAppl = ' ';
  std::vector<RoutingNumber> _routings;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, LimitationCmn);
    FLATTENIZE(archive, _govCarrierAppl);
    FLATTENIZE(archive, _govCarriers);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _fareComponentAppl);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _mustNotViaHip);
    FLATTENIZE(archive, _confirmedAppl);
    FLATTENIZE(archive, _maxDomSegments);
    FLATTENIZE(archive, _exceptViaCxrLocInd);
    FLATTENIZE(archive, _exceptViaCxrs);
    FLATTENIZE(archive, _notViaLoc);
    FLATTENIZE(archive, _viaGovCarrierInd);
    FLATTENIZE(archive, _notViaToFromLoc);
    FLATTENIZE(archive, _retransitGovCxrAppl);
    FLATTENIZE(archive, _routings);
  }

private:
  LimitationFare(const LimitationFare&);
  LimitationFare& operator=(const LimitationFare&);
};
} // namespace tse
