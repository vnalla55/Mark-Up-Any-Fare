#pragma once

#include "Common/CabinType.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MinFareCxrFltRestr.h"

#include <vector>

namespace tse
{
class MinFareAppl
{
public:
  MinFareAppl()
    : _textTblItemNo(0),
      _seqNo(0),
      _routingTariff1(0),
      _routingTariff2(0),
      _ruleTariff(0),
      _tariffCat(0),
      _nmlHipTariffCatInd(0),
      _nmlCtmTariffCatInd(0),
      _userApplType(' '),
      _fareTypeAppl(' '),
      _mpmInd(' '),
      _routingInd(' '),
      _globalDir(GlobalDirection::NO_DIR),
      _directionalInd(FROM),
      _viaDirectionalInd(FROM),
      _intermDirectionalInd(FROM),
      _viaExceptInd(' '),
      _stopConnectRestr(' '),
      _hipCheckAppl(' '),
      _hipStopTktInd(' '),
      _ctmCheckAppl(' '),
      _ctmStopTktInd(' '),
      _backhaulCheckAppl(' '),
      _backhaulStopTktInd(' '),
      _dmcCheckAppl(' '),
      _dmcStopTktInd(' '),
      _comCheckAppl(' '),
      _comStopTktInd(' '),
      _cpmCheckAppl(' '),
      _cpmStopTktInd(' '),
      _serviceRestr(' '),
      _nonStopInd(' '),
      _directInd(' '),
      _constructPointInd(' '),
      _exceptCxrFltRestr(' '),
      _exceptSecondaryCxr(' '),
      _betwDirectionalInd(FROM),
      _posExceptInd(' '),
      _poiExceptInd(' '),
      _sotiInd(' '),
      _sotoInd(' '),
      _sitiInd(' '),
      _sitoInd(' '),
      _applyDefaultLogic(' '),
      _domAppl(' '),
      _domExceptInd(' '),
      _nmlFareCompareInd(' '),
      _nmlMpmBeforeRtgInd(' '),
      _nmlRtgBeforeMpmInd(' '),
      _nmlHipRestrCompInd(' '),
      _nmlHipUnrestrCompInd(' '),
      _nmlHipRbdCompInd(' '),
      _nmlHipStopCompInd(' '),
      _nmlHipOrigInd(' '),
      _nmlHipOrigNationInd(' '),
      _nmlHipFromInterInd(' '),
      _nmlHipDestInd(' '),
      _nmlHipDestNationInd(' '),
      _nmlHipToInterInd(' '),
      _nmlHipExemptInterToInter(' '),
      _spclHipTariffCatInd(' '),
      _spclHipRuleTrfInd(' '),
      _spclHipFareClassInd(' '),
      _spclHip1stCharInd(' '),
      _spclHipStopCompInd(' '),
      _spclHipSpclOnlyInd(' '),
      _spclHipOrigInd(' '),
      _spclHipOrigNationInd(' '),
      _spclHipFromInterInd(' '),
      _spclHipDestInd(' '),
      _spclHipDestNationInd(' '),
      _spclHipToInterInd(' '),
      _spclHipExemptInterToInter(' '),
      _nmlCtmRestrCompInd(' '),
      _nmlCtmUnrestrCompInd(' '),
      _nmlCtmRbdCompInd(' '),
      _nmlCtmStopCompInd(' '),
      _nmlCtmOrigInd(' '),
      _nmlCtmDestNationInd(' '),
      _nmlCtmToInterInd(' '),
      _spclCtmTariffCatInd(' '),
      _spclCtmRuleTrfInd(' '),
      _spclCtmFareClassInd(' '),
      _spclSame1stCharFBInd2(' '),
      _spclCtmStopCompInd(' '),
      _spclCtmMktComp(' '),
      _spclCtmOrigInd(' '),
      _spclCtmDestNationInd(' '),
      _spclCtmToInterInd(' '),
      _cpmExcl(' '),
      _intHipCheckAppl(' '),
      _intHipStopTktInd(' '),
      _intCtmCheckAppl(' '),
      _intCtmStopTktInd(' '),
      _intBackhaulChkAppl(' '),
      _intBackhaulStopTktInd(' '),
      _intDmcCheckAppl(' '),
      _intDmcStopTktInd(' '),
      _intComCheckAppl(' '),
      _intComStopTktInd(' '),
      _intCpmCheckAppl(' '),
      _intCpmStopTktInd(' '),
      _domFareTypeExcept(' '),
      _nmlHipOrigNationTktPt(' '),
      _nmlHipOrigNationStopPt(' '),
      _nmlHipStopoverPt(' '),
      _nmlHipTicketedPt(' '),
      _spclHipOrigNationTktPt(' '),
      _spclHipOrigNationStopPt(' '),
      _spclHipStopoverPt(' '),
      _spclHipTicketedPt(' ')
  {
  }

  ~MinFareAppl()
  {
    std::vector<MinFareCxrFltRestr*>::iterator MFCFRIt;
    for (MFCFRIt = _cxrFltRestrs.begin(); MFCFRIt != _cxrFltRestrs.end(); MFCFRIt++)
    { // Get Children ... First the ExcptCxrs ...
      delete *MFCFRIt;
    }
  }

  VendorCode& textTblVendor() { return _textTblVendor; }
  const VendorCode& textTblVendor() const { return _textTblVendor; }

  int& textTblItemNo() { return _textTblItemNo; }
  const int& textTblItemNo() const { return _textTblItemNo; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  TariffNumber& routingTariff1() { return _routingTariff1; }
  const TariffNumber& routingTariff1() const { return _routingTariff1; }

  TariffNumber& routingTariff2() { return _routingTariff2; }
  const TariffNumber& routingTariff2() const { return _routingTariff2; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  TariffCategory& tariffCat() { return _tariffCat; }
  const TariffCategory& tariffCat() const { return _tariffCat; }

  int& nmlHipTariffCatInd() { return _nmlHipTariffCatInd; }
  const int& nmlHipTariffCatInd() const { return _nmlHipTariffCatInd; }

  int& nmlCtmTariffCatInd() { return _nmlCtmTariffCatInd; }
  const int& nmlCtmTariffCatInd() const { return _nmlCtmTariffCatInd; }

  CarrierCode& tktgCarrier() { return _tktgCarrier; }
  const CarrierCode& tktgCarrier() const { return _tktgCarrier; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  Indicator& fareTypeAppl() { return _fareTypeAppl; }
  const Indicator& fareTypeAppl() const { return _fareTypeAppl; }

  Indicator& mpmInd() { return _mpmInd; }
  const Indicator& mpmInd() const { return _mpmInd; }

  Indicator& routingInd() { return _routingInd; }
  const Indicator& routingInd() const { return _routingInd; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Directionality& directionalInd() { return _directionalInd; }
  const Directionality& directionalInd() const { return _directionalInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& viaExceptInd() { return _viaExceptInd; }
  const Indicator& viaExceptInd() const { return _viaExceptInd; }

  Directionality& viaDirectionalInd() { return _viaDirectionalInd; }
  const Directionality& viaDirectionalInd() const { return _viaDirectionalInd; }

  LocKey& viaLoc1() { return _viaLoc1; }
  const LocKey& viaLoc1() const { return _viaLoc1; }

  LocKey& viaLoc2() { return _viaLoc2; }
  const LocKey& viaLoc2() const { return _viaLoc2; }

  Directionality& intermDirectionalInd() { return _intermDirectionalInd; }
  const Directionality& intermDirectionalInd() const { return _intermDirectionalInd; }

  LocKey& intermediateLoc1() { return _intermediateLoc1; }
  const LocKey& intermediateLoc1() const { return _intermediateLoc1; }

  LocKey& intermediateLoc2() { return _intermediateLoc2; }
  const LocKey& intermediateLoc2() const { return _intermediateLoc2; }

  Indicator& stopConnectRestr() { return _stopConnectRestr; }
  const Indicator& stopConnectRestr() const { return _stopConnectRestr; }

  Indicator& hipCheckAppl() { return _hipCheckAppl; }
  const Indicator& hipCheckAppl() const { return _hipCheckAppl; }

  Indicator& hipStopTktInd() { return _hipStopTktInd; }
  const Indicator& hipStopTktInd() const { return _hipStopTktInd; }

  Indicator& ctmCheckAppl() { return _ctmCheckAppl; }
  const Indicator& ctmCheckAppl() const { return _ctmCheckAppl; }

  Indicator& ctmStopTktInd() { return _ctmStopTktInd; }
  const Indicator& ctmStopTktInd() const { return _ctmStopTktInd; }

  Indicator& backhaulCheckAppl() { return _backhaulCheckAppl; }
  const Indicator& backhaulCheckAppl() const { return _backhaulCheckAppl; }

  Indicator& backhaulStopTktInd() { return _backhaulStopTktInd; }
  const Indicator& backhaulStopTktInd() const { return _backhaulStopTktInd; }

  Indicator& dmcCheckAppl() { return _dmcCheckAppl; }
  const Indicator& dmcCheckAppl() const { return _dmcCheckAppl; }

  Indicator& dmcStopTktInd() { return _dmcStopTktInd; }
  const Indicator& dmcStopTktInd() const { return _dmcStopTktInd; }

  Indicator& comCheckAppl() { return _comCheckAppl; }
  const Indicator& comCheckAppl() const { return _comCheckAppl; }

  Indicator& comStopTktInd() { return _comStopTktInd; }
  const Indicator& comStopTktInd() const { return _comStopTktInd; }

  Indicator& cpmCheckAppl() { return _cpmCheckAppl; }
  const Indicator& cpmCheckAppl() const { return _cpmCheckAppl; }

  Indicator& cpmStopTktInd() { return _cpmStopTktInd; }
  const Indicator& cpmStopTktInd() const { return _cpmStopTktInd; }

  Directionality& betwDirectionalInd() { return _betwDirectionalInd; }
  const Directionality& betwDirectionalInd() const { return _betwDirectionalInd; }

  LocKey& betwLoc1() { return _betwLoc1; }
  const LocKey& betwLoc1() const { return _betwLoc1; }

  LocKey& betwLoc2() { return _betwLoc2; }
  const LocKey& betwLoc2() const { return _betwLoc2; }

  Indicator& serviceRestr() { return _serviceRestr; }
  const Indicator& serviceRestr() const { return _serviceRestr; }

  Indicator& nonStopInd() { return _nonStopInd; }
  const Indicator& nonStopInd() const { return _nonStopInd; }

  Indicator& directInd() { return _directInd; }
  const Indicator& directInd() const { return _directInd; }

  Indicator& constructPointInd() { return _constructPointInd; }
  const Indicator& constructPointInd() const { return _constructPointInd; }

  Indicator& exceptCxrFltRestr() { return _exceptCxrFltRestr; }
  const Indicator& exceptCxrFltRestr() const { return _exceptCxrFltRestr; }

  Indicator& exceptSecondaryCxr() { return _exceptSecondaryCxr; }
  const Indicator& exceptSecondaryCxr() const { return _exceptSecondaryCxr; }

  Indicator& posExceptInd() { return _posExceptInd; }
  const Indicator& posExceptInd() const { return _posExceptInd; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  Indicator& poiExceptInd() { return _poiExceptInd; }
  const Indicator& poiExceptInd() const { return _poiExceptInd; }

  LocKey& poiLoc() { return _poiLoc; }
  const LocKey& poiLoc() const { return _poiLoc; }

  Indicator& sotiInd() { return _sotiInd; }
  const Indicator& sotiInd() const { return _sotiInd; }

  Indicator& sotoInd() { return _sotoInd; }
  const Indicator& sotoInd() const { return _sotoInd; }

  Indicator& sitiInd() { return _sitiInd; }
  const Indicator& sitiInd() const { return _sitiInd; }

  Indicator& sitoInd() { return _sitoInd; }
  const Indicator& sitoInd() const { return _sitoInd; }

  // Children
  std::vector<CarrierCode>& excptCxrs() { return _excptCxrs; }
  const std::vector<CarrierCode>& excptCxrs() const { return _excptCxrs; }

  std::vector<RuleNumber>& rules() { return _rules; }
  const std::vector<RuleNumber>& rules() const { return _rules; }

  std::vector<std::pair<RuleNumber, Footnote> >& ruleFootnotes() { return _ruleFootnotes; }
  const std::vector<std::pair<RuleNumber, Footnote> >&
  ruleFootnotes() const { return _ruleFootnotes; }

  std::vector<FareClassCode>& fareClasses() { return _fareClasses; }
  const std::vector<FareClassCode>& fareClasses() const { return _fareClasses; }

  std::vector<FareTypeAbbrev>& fareTypes() { return _fareTypes; }
  const std::vector<FareTypeAbbrev>& fareTypes() const { return _fareTypes; }

  std::vector<MinFareCxrFltRestr*>& cxrFltRestrs() { return _cxrFltRestrs; }
  const std::vector<MinFareCxrFltRestr*>& cxrFltRestrs() const { return _cxrFltRestrs; }

  std::vector<CarrierCode>& secondaryCxrs() { return _secondaryCxrs; }
  const std::vector<CarrierCode>& secondaryCxrs() const { return _secondaryCxrs; }

  std::vector<FareTypeAbbrev>& domFareTypes() { return _domFareTypes; }
  const std::vector<FareTypeAbbrev>& domFareTypes() const { return _domFareTypes; }

  // Override (From MinimumFares if _applyDefaultLogic == 'N')
  Indicator& applyDefaultLogic() { return _applyDefaultLogic; }
  const Indicator& applyDefaultLogic() const { return _applyDefaultLogic; }

  Indicator& domAppl() { return _domAppl; }
  const Indicator& domAppl() const { return _domAppl; }

  Indicator& domExceptInd() { return _domExceptInd; }
  const Indicator& domExceptInd() const { return _domExceptInd; }

  LocKey& domLoc() { return _domLoc; }
  const LocKey& domLoc() const { return _domLoc; }

  Indicator& nmlFareCompareInd() { return _nmlFareCompareInd; }
  const Indicator& nmlFareCompareInd() const { return _nmlFareCompareInd; }

  Indicator& nmlMpmBeforeRtgInd() { return _nmlMpmBeforeRtgInd; }
  const Indicator& nmlMpmBeforeRtgInd() const { return _nmlMpmBeforeRtgInd; }

  Indicator& nmlRtgBeforeMpmInd() { return _nmlRtgBeforeMpmInd; }
  const Indicator& nmlRtgBeforeMpmInd() const { return _nmlRtgBeforeMpmInd; }

  Indicator& nmlHipRestrCompInd() { return _nmlHipRestrCompInd; }
  const Indicator& nmlHipRestrCompInd() const { return _nmlHipRestrCompInd; }

  Indicator& nmlHipUnrestrCompInd() { return _nmlHipUnrestrCompInd; }
  const Indicator& nmlHipUnrestrCompInd() const { return _nmlHipUnrestrCompInd; }

  Indicator& nmlHipRbdCompInd() { return _nmlHipRbdCompInd; }
  const Indicator& nmlHipRbdCompInd() const { return _nmlHipRbdCompInd; }

  Indicator& nmlHipStopCompInd() { return _nmlHipStopCompInd; }
  const Indicator& nmlHipStopCompInd() const { return _nmlHipStopCompInd; }

  Indicator& nmlHipOrigInd() { return _nmlHipOrigInd; }
  const Indicator& nmlHipOrigInd() const { return _nmlHipOrigInd; }

  Indicator& nmlHipOrigNationInd() { return _nmlHipOrigNationInd; }
  const Indicator& nmlHipOrigNationInd() const { return _nmlHipOrigNationInd; }

  Indicator& nmlHipFromInterInd() { return _nmlHipFromInterInd; }
  const Indicator& nmlHipFromInterInd() const { return _nmlHipFromInterInd; }

  Indicator& nmlHipDestInd() { return _nmlHipDestInd; }
  const Indicator& nmlHipDestInd() const { return _nmlHipDestInd; }

  Indicator& nmlHipDestNationInd() { return _nmlHipDestNationInd; }
  const Indicator& nmlHipDestNationInd() const { return _nmlHipDestNationInd; }

  Indicator& nmlHipToInterInd() { return _nmlHipToInterInd; }
  const Indicator& nmlHipToInterInd() const { return _nmlHipToInterInd; }

  Indicator& nmlHipExemptInterToInter() { return _nmlHipExemptInterToInter; }
  const Indicator& nmlHipExemptInterToInter() const { return _nmlHipExemptInterToInter; }

  Indicator& spclHipTariffCatInd() { return _spclHipTariffCatInd; }
  const Indicator& spclHipTariffCatInd() const { return _spclHipTariffCatInd; }

  Indicator& spclHipRuleTrfInd() { return _spclHipRuleTrfInd; }
  const Indicator& spclHipRuleTrfInd() const { return _spclHipRuleTrfInd; }

  Indicator& spclHipFareClassInd() { return _spclHipFareClassInd; }
  const Indicator& spclHipFareClassInd() const { return _spclHipFareClassInd; }

  Indicator& spclHip1stCharInd() { return _spclHip1stCharInd; }
  const Indicator& spclHip1stCharInd() const { return _spclHip1stCharInd; }

  Indicator& spclHipStopCompInd() { return _spclHipStopCompInd; }
  const Indicator& spclHipStopCompInd() const { return _spclHipStopCompInd; }

  Indicator& spclHipSpclOnlyInd() { return _spclHipSpclOnlyInd; }
  const Indicator& spclHipSpclOnlyInd() const { return _spclHipSpclOnlyInd; }

  LocKey& spclHipLoc() { return _spclHipLoc; }
  const LocKey& spclHipLoc() const { return _spclHipLoc; }

  Indicator& spclHipOrigInd() { return _spclHipOrigInd; }
  const Indicator& spclHipOrigInd() const { return _spclHipOrigInd; }

  Indicator& spclHipOrigNationInd() { return _spclHipOrigNationInd; }
  const Indicator& spclHipOrigNationInd() const { return _spclHipOrigNationInd; }

  Indicator& spclHipFromInterInd() { return _spclHipFromInterInd; }
  const Indicator& spclHipFromInterInd() const { return _spclHipFromInterInd; }

  Indicator& spclHipDestInd() { return _spclHipDestInd; }
  const Indicator& spclHipDestInd() const { return _spclHipDestInd; }

  Indicator& spclHipDestNationInd() { return _spclHipDestNationInd; }
  const Indicator& spclHipDestNationInd() const { return _spclHipDestNationInd; }

  std::string& specialProcessName() { return _specialProcessName; }
  const std::string& specialProcessName() const { return _specialProcessName; }

  Indicator& spclHipToInterInd() { return _spclHipToInterInd; }
  const Indicator& spclHipToInterInd() const { return _spclHipToInterInd; }

  Indicator& spclHipExemptInterToInter() { return _spclHipExemptInterToInter; }
  const Indicator& spclHipExemptInterToInter() const { return _spclHipExemptInterToInter; }

  Indicator& nmlCtmRestrCompInd() { return _nmlCtmRestrCompInd; }
  const Indicator& nmlCtmRestrCompInd() const { return _nmlCtmRestrCompInd; }

  Indicator& nmlCtmUnrestrCompInd() { return _nmlCtmUnrestrCompInd; }
  const Indicator& nmlCtmUnrestrCompInd() const { return _nmlCtmUnrestrCompInd; }

  Indicator& nmlCtmRbdCompInd() { return _nmlCtmRbdCompInd; }
  const Indicator& nmlCtmRbdCompInd() const { return _nmlCtmRbdCompInd; }

  Indicator& nmlCtmStopCompInd() { return _nmlCtmStopCompInd; }
  const Indicator& nmlCtmStopCompInd() const { return _nmlCtmStopCompInd; }

  Indicator& nmlCtmOrigInd() { return _nmlCtmOrigInd; }
  const Indicator& nmlCtmOrigInd() const { return _nmlCtmOrigInd; }

  Indicator& nmlCtmDestNationInd() { return _nmlCtmDestNationInd; }
  const Indicator& nmlCtmDestNationInd() const { return _nmlCtmDestNationInd; }

  Indicator& nmlCtmToInterInd() { return _nmlCtmToInterInd; }
  const Indicator& nmlCtmToInterInd() const { return _nmlCtmToInterInd; }

  Indicator& spclCtmTariffCatInd() { return _spclCtmTariffCatInd; }
  const Indicator& spclCtmTariffCatInd() const { return _spclCtmTariffCatInd; }

  Indicator& spclCtmRuleTrfInd() { return _spclCtmRuleTrfInd; }
  const Indicator& spclCtmRuleTrfInd() const { return _spclCtmRuleTrfInd; }

  Indicator& spclCtmFareClassInd() { return _spclCtmFareClassInd; }
  const Indicator& spclCtmFareClassInd() const { return _spclCtmFareClassInd; }

  Indicator& spclSame1stCharFBInd2() { return _spclSame1stCharFBInd2; }
  const Indicator& spclSame1stCharFBInd2() const { return _spclSame1stCharFBInd2; }

  Indicator& spclCtmStopCompInd() { return _spclCtmStopCompInd; }
  const Indicator& spclCtmStopCompInd() const { return _spclCtmStopCompInd; }

  Indicator& spclCtmMktComp() { return _spclCtmMktComp; }
  const Indicator& spclCtmMktComp() const { return _spclCtmMktComp; }

  Indicator& spclCtmOrigInd() { return _spclCtmOrigInd; }
  const Indicator& spclCtmOrigInd() const { return _spclCtmOrigInd; }

  Indicator& spclCtmDestNationInd() { return _spclCtmDestNationInd; }
  const Indicator& spclCtmDestNationInd() const { return _spclCtmDestNationInd; }

  Indicator& spclCtmToInterInd() { return _spclCtmToInterInd; }
  const Indicator& spclCtmToInterInd() const { return _spclCtmToInterInd; }

  Indicator& cpmExcl() { return _cpmExcl; }
  const Indicator& cpmExcl() const { return _cpmExcl; }

  Indicator& intHipCheckAppl() { return _intHipCheckAppl; }
  const Indicator& intHipCheckAppl() const { return _intHipCheckAppl; }

  Indicator& intHipStopTktInd() { return _intHipStopTktInd; }
  const Indicator& intHipStopTktInd() const { return _intHipStopTktInd; }

  Indicator& intCtmCheckAppl() { return _intCtmCheckAppl; }
  const Indicator& intCtmCheckAppl() const { return _intCtmCheckAppl; }

  Indicator& intCtmStopTktInd() { return _intCtmStopTktInd; }
  const Indicator& intCtmStopTktInd() const { return _intCtmStopTktInd; }

  Indicator& intBackhaulChkAppl() { return _intBackhaulChkAppl; }
  const Indicator& intBackhaulChkAppl() const { return _intBackhaulChkAppl; }

  Indicator& intBackhaulStopTktInd() { return _intBackhaulStopTktInd; }
  const Indicator& intBackhaulStopTktInd() const { return _intBackhaulStopTktInd; }

  Indicator& intDmcCheckAppl() { return _intDmcCheckAppl; }
  const Indicator& intDmcCheckAppl() const { return _intDmcCheckAppl; }

  Indicator& intDmcStopTktInd() { return _intDmcStopTktInd; }
  const Indicator& intDmcStopTktInd() const { return _intDmcStopTktInd; }

  Indicator& intComCheckAppl() { return _intComCheckAppl; }
  const Indicator& intComCheckAppl() const { return _intComCheckAppl; }

  Indicator& intComStopTktInd() { return _intComStopTktInd; }
  const Indicator& intComStopTktInd() const { return _intComStopTktInd; }

  Indicator& intCpmCheckAppl() { return _intCpmCheckAppl; }
  const Indicator& intCpmCheckAppl() const { return _intCpmCheckAppl; }

  Indicator& intCpmStopTktInd() { return _intCpmStopTktInd; }
  const Indicator& intCpmStopTktInd() const { return _intCpmStopTktInd; }

  Indicator& domFareTypeExcept() { return _domFareTypeExcept; }
  const Indicator& domFareTypeExcept() const { return _domFareTypeExcept; }

  Indicator& nmlHipOrigNationTktPt() { return _nmlHipOrigNationTktPt; }
  const Indicator& nmlHipOrigNationTktPt() const { return _nmlHipOrigNationTktPt; }

  Indicator& nmlHipOrigNationStopPt() { return _nmlHipOrigNationStopPt; }
  const Indicator& nmlHipOrigNationStopPt() const { return _nmlHipOrigNationStopPt; }

  Indicator& nmlHipStopoverPt() { return _nmlHipStopoverPt; }
  const Indicator& nmlHipStopoverPt() const { return _nmlHipStopoverPt; }

  Indicator& nmlHipTicketedPt() { return _nmlHipTicketedPt; }
  const Indicator& nmlHipTicketedPt() const { return _nmlHipTicketedPt; }

  Indicator& spclHipOrigNationTktPt() { return _spclHipOrigNationTktPt; }
  const Indicator& spclHipOrigNationTktPt() const { return _spclHipOrigNationTktPt; }

  Indicator& spclHipOrigNationStopPt() { return _spclHipOrigNationStopPt; }
  const Indicator& spclHipOrigNationStopPt() const { return _spclHipOrigNationStopPt; }

  Indicator& spclHipStopoverPt() { return _spclHipStopoverPt; }
  const Indicator& spclHipStopoverPt() const { return _spclHipStopoverPt; }

  Indicator& spclHipTicketedPt() { return _spclHipTicketedPt; }
  const Indicator& spclHipTicketedPt() const { return _spclHipTicketedPt; }

  bool operator==(const MinFareAppl& rhs) const
  {
    bool
    eq((_textTblVendor == rhs._textTblVendor) && (_textTblItemNo == rhs._textTblItemNo) &&
       (_governingCarrier == rhs._governingCarrier) && (_versionDate == rhs._versionDate) &&
       (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
       (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
       (_discDate == rhs._discDate) && (_vendor == rhs._vendor) &&
       (_routingTariff1 == rhs._routingTariff1) && (_routingTariff2 == rhs._routingTariff2) &&
       (_ruleTariff == rhs._ruleTariff) && (_tariffCat == rhs._tariffCat) &&
       (_nmlHipTariffCatInd == rhs._nmlHipTariffCatInd) &&
       (_nmlCtmTariffCatInd == rhs._nmlCtmTariffCatInd) && (_tktgCarrier == rhs._tktgCarrier) &&
       (_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
       (_ruleTariffCode == rhs._ruleTariffCode) && (_fareTypeAppl == rhs._fareTypeAppl) &&
       (_mpmInd == rhs._mpmInd) && (_routingInd == rhs._routingInd) &&
       (_globalDir == rhs._globalDir) && (_directionalInd == rhs._directionalInd) &&
       (_routing == rhs._routing) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
       (_viaExceptInd == rhs._viaExceptInd) && (_viaDirectionalInd == rhs._viaDirectionalInd) &&
       (_viaLoc1 == rhs._viaLoc1) && (_viaLoc2 == rhs._viaLoc2) &&
       (_intermDirectionalInd == rhs._intermDirectionalInd) &&
       (_intermediateLoc1 == rhs._intermediateLoc1) &&
       (_intermediateLoc2 == rhs._intermediateLoc2) &&
       (_stopConnectRestr == rhs._stopConnectRestr) && (_hipCheckAppl == rhs._hipCheckAppl) &&
       (_hipStopTktInd == rhs._hipStopTktInd) && (_ctmCheckAppl == rhs._ctmCheckAppl) &&
       (_ctmStopTktInd == rhs._ctmStopTktInd) && (_backhaulCheckAppl == rhs._backhaulCheckAppl) &&
       (_backhaulStopTktInd == rhs._backhaulStopTktInd) && (_dmcCheckAppl == rhs._dmcCheckAppl) &&
       (_dmcStopTktInd == rhs._dmcStopTktInd) && (_comCheckAppl == rhs._comCheckAppl) &&
       (_comStopTktInd == rhs._comStopTktInd) && (_cpmCheckAppl == rhs._cpmCheckAppl) &&
       (_cpmStopTktInd == rhs._cpmStopTktInd) && (_betwDirectionalInd == rhs._betwDirectionalInd) &&
       (_betwLoc1 == rhs._betwLoc1) && (_betwLoc2 == rhs._betwLoc2) &&
       (_serviceRestr == rhs._serviceRestr) && (_nonStopInd == rhs._nonStopInd) &&
       (_directInd == rhs._directInd) && (_constructPointInd == rhs._constructPointInd) &&
       (_exceptCxrFltRestr == rhs._exceptCxrFltRestr) &&
       (_exceptSecondaryCxr == rhs._exceptSecondaryCxr) && (_posExceptInd == rhs._posExceptInd) &&
       (_posLoc == rhs._posLoc) && (_poiExceptInd == rhs._poiExceptInd) &&
       (_poiLoc == rhs._poiLoc) && (_sotiInd == rhs._sotiInd) && (_sotoInd == rhs._sotoInd) &&
       (_sitiInd == rhs._sitiInd) && (_sitoInd == rhs._sitoInd) && (_excptCxrs == rhs._excptCxrs) &&
       (_rules == rhs._rules) && (_ruleFootnotes == rhs._ruleFootnotes) &&
       (_fareClasses == rhs._fareClasses) &&
       (_fareTypes == rhs._fareTypes) && (_secondaryCxrs == rhs._secondaryCxrs) &&
       (_domFareTypes == rhs._domFareTypes) && (_applyDefaultLogic == rhs._applyDefaultLogic) &&
       (_domAppl == rhs._domAppl) && (_domExceptInd == rhs._domExceptInd) &&
       (_domLoc == rhs._domLoc) && (_nmlFareCompareInd == rhs._nmlFareCompareInd) &&
       (_nmlMpmBeforeRtgInd == rhs._nmlMpmBeforeRtgInd) &&
       (_nmlRtgBeforeMpmInd == rhs._nmlRtgBeforeMpmInd) &&
       (_nmlHipRestrCompInd == rhs._nmlHipRestrCompInd) &&
       (_nmlHipUnrestrCompInd == rhs._nmlHipUnrestrCompInd) &&
       (_nmlHipRbdCompInd == rhs._nmlHipRbdCompInd) &&
       (_nmlHipStopCompInd == rhs._nmlHipStopCompInd) && (_nmlHipOrigInd == rhs._nmlHipOrigInd) &&
       (_nmlHipOrigNationInd == rhs._nmlHipOrigNationInd) &&
       (_nmlHipFromInterInd == rhs._nmlHipFromInterInd) && (_nmlHipDestInd == rhs._nmlHipDestInd) &&
       (_nmlHipDestNationInd == rhs._nmlHipDestNationInd) &&
       (_nmlHipToInterInd == rhs._nmlHipToInterInd) &&
       (_nmlHipExemptInterToInter == rhs._nmlHipExemptInterToInter) &&
       (_spclHipTariffCatInd == rhs._spclHipTariffCatInd) &&
       (_spclHipRuleTrfInd == rhs._spclHipRuleTrfInd) &&
       (_spclHipFareClassInd == rhs._spclHipFareClassInd) &&
       (_spclHip1stCharInd == rhs._spclHip1stCharInd) &&
       (_spclHipStopCompInd == rhs._spclHipStopCompInd) &&
       (_spclHipSpclOnlyInd == rhs._spclHipSpclOnlyInd) && (_spclHipLoc == rhs._spclHipLoc) &&
       (_spclHipOrigInd == rhs._spclHipOrigInd) &&
       (_spclHipOrigNationInd == rhs._spclHipOrigNationInd) &&
       (_spclHipFromInterInd == rhs._spclHipFromInterInd) &&
       (_spclHipDestInd == rhs._spclHipDestInd) &&
       (_spclHipDestNationInd == rhs._spclHipDestNationInd) &&
       (_specialProcessName == rhs._specialProcessName) &&
       (_spclHipToInterInd == rhs._spclHipToInterInd) &&
       (_spclHipExemptInterToInter == rhs._spclHipExemptInterToInter) &&
       (_nmlCtmRestrCompInd == rhs._nmlCtmRestrCompInd) &&
       (_nmlCtmUnrestrCompInd == rhs._nmlCtmUnrestrCompInd) &&
       (_nmlCtmRbdCompInd == rhs._nmlCtmRbdCompInd) &&
       (_nmlCtmStopCompInd == rhs._nmlCtmStopCompInd) && (_nmlCtmOrigInd == rhs._nmlCtmOrigInd) &&
       (_nmlCtmDestNationInd == rhs._nmlCtmDestNationInd) &&
       (_nmlCtmToInterInd == rhs._nmlCtmToInterInd) &&
       (_spclCtmTariffCatInd == rhs._spclCtmTariffCatInd) &&
       (_spclCtmRuleTrfInd == rhs._spclCtmRuleTrfInd) &&
       (_spclCtmFareClassInd == rhs._spclCtmFareClassInd) &&
       (_spclSame1stCharFBInd2 == rhs._spclSame1stCharFBInd2) &&
       (_spclCtmStopCompInd == rhs._spclCtmStopCompInd) &&
       (_spclCtmMktComp == rhs._spclCtmMktComp) && (_spclCtmOrigInd == rhs._spclCtmOrigInd) &&
       (_spclCtmDestNationInd == rhs._spclCtmDestNationInd) &&
       (_spclCtmToInterInd == rhs._spclCtmToInterInd) && (_cpmExcl == rhs._cpmExcl) &&
       (_intHipCheckAppl == rhs._intHipCheckAppl) && (_intHipStopTktInd == rhs._intHipStopTktInd) &&
       (_intCtmCheckAppl == rhs._intCtmCheckAppl) && (_intCtmStopTktInd == rhs._intCtmStopTktInd) &&
       (_intBackhaulChkAppl == rhs._intBackhaulChkAppl) &&
       (_intBackhaulStopTktInd == rhs._intBackhaulStopTktInd) &&
       (_intDmcCheckAppl == rhs._intDmcCheckAppl) && (_intDmcStopTktInd == rhs._intDmcStopTktInd) &&
       (_intComCheckAppl == rhs._intComCheckAppl) && (_intComStopTktInd == rhs._intComStopTktInd) &&
       (_intCpmCheckAppl == rhs._intCpmCheckAppl) && (_intCpmStopTktInd == rhs._intCpmStopTktInd) &&
       (_domFareTypeExcept == rhs._domFareTypeExcept) &&
       (_nmlHipOrigNationTktPt == rhs._nmlHipOrigNationTktPt) &&
       (_nmlHipOrigNationStopPt == rhs._nmlHipOrigNationStopPt) &&
       (_nmlHipStopoverPt == rhs._nmlHipStopoverPt) &&
       (_nmlHipTicketedPt == rhs._nmlHipTicketedPt) &&
       (_spclHipOrigNationTktPt == rhs._spclHipOrigNationTktPt) &&
       (_spclHipOrigNationStopPt == rhs._spclHipOrigNationStopPt) &&
       (_spclHipStopoverPt == rhs._spclHipStopoverPt) &&
       (_spclHipTicketedPt == rhs._spclHipTicketedPt) &&
       (_cxrFltRestrs.size() == rhs._cxrFltRestrs.size()));

    for (size_t i = 0; (eq && (i < _cxrFltRestrs.size())); ++i)
    {
      eq = (*(_cxrFltRestrs[i]) == *(rhs._cxrFltRestrs[i]));
    }

    return eq;
  }

  static void dummyData(MinFareAppl& obj)
  {
    obj._textTblVendor = "ABCD";
    obj._textTblItemNo = 1;
    obj._governingCarrier = "EFG";
    obj._versionDate = time(nullptr);
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._vendor = "HIJK";
    obj._routingTariff1 = 3;
    obj._routingTariff2 = 4;
    obj._ruleTariff = 5;
    obj._tariffCat = 6;
    obj._nmlHipTariffCatInd = 7;
    obj._nmlCtmTariffCatInd = 8;
    obj._tktgCarrier = "LMN";
    obj._userApplType = 'O';
    obj._userAppl = "PQRS";
    obj._ruleTariffCode = "TUVWXYZ";
    obj._fareTypeAppl = 'a';

    obj._mpmInd = 'b';
    obj._routingInd = 'c';
    obj._routing = "defh";
    obj._globalDir = GlobalDirection::US;
    obj._directionalInd = FROM;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._viaExceptInd = 'i';
    obj._viaDirectionalInd = BETWEEN;

    LocKey::dummyData(obj._viaLoc1);
    LocKey::dummyData(obj._viaLoc2);

    obj._intermDirectionalInd = TO;

    LocKey::dummyData(obj._intermediateLoc1);
    LocKey::dummyData(obj._intermediateLoc2);

    obj._stopConnectRestr = 'j';
    obj._hipCheckAppl = 'k';
    obj._hipStopTktInd = 'l';
    obj._ctmCheckAppl = 'm';
    obj._ctmStopTktInd = 'n';
    obj._backhaulCheckAppl = 'o';
    obj._backhaulStopTktInd = 'p';
    obj._dmcCheckAppl = 'q';
    obj._dmcStopTktInd = 'r';
    obj._comCheckAppl = 's';
    obj._comStopTktInd = 't';
    obj._cpmCheckAppl = 'u';
    obj._cpmStopTktInd = 'v';
    obj._betwDirectionalInd = BOTH;

    LocKey::dummyData(obj._betwLoc1);
    LocKey::dummyData(obj._betwLoc2);

    obj._serviceRestr = 'w';
    obj._nonStopInd = 'x';
    obj._directInd = 'y';
    obj._constructPointInd = 'z';
    obj._exceptCxrFltRestr = '1';
    obj._exceptSecondaryCxr = '2';
    obj._posExceptInd = '3';

    LocKey::dummyData(obj._posLoc);

    obj._poiExceptInd = '4';

    LocKey::dummyData(obj._poiLoc);

    obj._sotiInd = '5';
    obj._sotoInd = '6';
    obj._sitiInd = '7';
    obj._sitoInd = '8';
    obj._applyDefaultLogic = '9';
    obj._domAppl = '0';
    obj._domExceptInd = 'A';

    LocKey::dummyData(obj._domLoc);

    obj._nmlFareCompareInd = 'B';
    obj._nmlMpmBeforeRtgInd = 'C';
    obj._nmlRtgBeforeMpmInd = 'D';
    obj._nmlHipRestrCompInd = 'E';
    obj._nmlHipUnrestrCompInd = 'F';
    obj._nmlHipRbdCompInd = 'G';
    obj._nmlHipStopCompInd = 'H';
    obj._nmlHipOrigInd = 'I';
    obj._nmlHipOrigNationInd = 'J';
    obj._nmlHipFromInterInd = 'K';
    obj._nmlHipDestInd = 'L';
    obj._nmlHipDestNationInd = 'M';
    obj._nmlHipToInterInd = 'N';
    obj._nmlHipExemptInterToInter = 'O';
    obj._spclHipTariffCatInd = 'P';
    obj._spclHipRuleTrfInd = 'Q';
    obj._spclHipFareClassInd = 'R';
    obj._spclHip1stCharInd = 'S';
    obj._spclHipStopCompInd = 'T';
    obj._spclHipSpclOnlyInd = 'U';

    LocKey::dummyData(obj._spclHipLoc);

    obj._spclHipOrigInd = 'V';
    obj._spclHipOrigNationInd = 'W';
    obj._spclHipFromInterInd = 'X';
    obj._spclHipDestInd = 'Y';
    obj._spclHipDestNationInd = 'Z';
    obj._specialProcessName = "bbbbbbbb";
    obj._spclHipToInterInd = 'a';
    obj._spclHipExemptInterToInter = 'b';
    obj._nmlCtmRestrCompInd = 'c';
    obj._nmlCtmUnrestrCompInd = 'd';
    obj._nmlCtmRbdCompInd = 'e';
    obj._nmlCtmStopCompInd = 'f';
    obj._nmlCtmOrigInd = 'g';
    obj._nmlCtmDestNationInd = 'h';
    obj._nmlCtmToInterInd = 'i';
    obj._spclCtmTariffCatInd = 'j';
    obj._spclCtmRuleTrfInd = 'k';
    obj._spclCtmFareClassInd = 'l';
    obj._spclSame1stCharFBInd2 = 'm';
    obj._spclCtmStopCompInd = 'n';
    obj._spclCtmMktComp = 'o';
    obj._spclCtmOrigInd = 'p';
    obj._spclCtmDestNationInd = 'q';
    obj._spclCtmToInterInd = 'r';
    obj._cpmExcl = 's';
    obj._intHipCheckAppl = 't';
    obj._intHipStopTktInd = 'u';
    obj._intCtmCheckAppl = 'v';
    obj._intCtmStopTktInd = 'w';
    obj._intBackhaulChkAppl = 'x';
    obj._intBackhaulStopTktInd = 'y';
    obj._intDmcCheckAppl = 'z';
    obj._intDmcStopTktInd = '1';
    obj._intComCheckAppl = '2';
    obj._intComStopTktInd = '3';
    obj._intCpmCheckAppl = '4';
    obj._intCpmStopTktInd = '5';
    obj._domFareTypeExcept = '6';
    obj._nmlHipOrigNationTktPt = '7';
    obj._nmlHipOrigNationStopPt = '8';
    obj._nmlHipStopoverPt = '9';
    obj._nmlHipTicketedPt = '0';
    obj._spclHipOrigNationTktPt = 'A';
    obj._spclHipOrigNationStopPt = 'B';
    obj._spclHipStopoverPt = 'C';
    obj._spclHipTicketedPt = 'D';

    obj._excptCxrs.push_back("EFG");
    obj._excptCxrs.push_back("HIJ");
    obj._rules.push_back("KLMN");
    obj._rules.push_back("OPQR");
    obj._fareClasses.push_back("cccccccc");
    obj._fareClasses.push_back("dddddddd");
    obj._fareTypes.push_back("STU");
    obj._fareTypes.push_back("VWX");
    obj._secondaryCxrs.push_back("ZYa");
    obj._secondaryCxrs.push_back("bcd");
    obj._domFareTypes.push_back("efg");
    obj._domFareTypes.push_back("hij");

    MinFareCxrFltRestr* mfcfr1 = new MinFareCxrFltRestr;
    MinFareCxrFltRestr* mfcfr2 = new MinFareCxrFltRestr;

    MinFareCxrFltRestr::dummyData(*mfcfr1);
    MinFareCxrFltRestr::dummyData(*mfcfr2);

    obj._cxrFltRestrs.push_back(mfcfr1);
    obj._cxrFltRestrs.push_back(mfcfr2);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  VendorCode _textTblVendor;
  int _textTblItemNo;
  CarrierCode _governingCarrier;
  DateTime _versionDate;
  int _seqNo;
  TariffNumber _routingTariff1;
  TariffNumber _routingTariff2;
  TariffNumber _ruleTariff;
  TariffCategory _tariffCat;
  int _nmlHipTariffCatInd;
  int _nmlCtmTariffCatInd;
  CarrierCode _tktgCarrier;
  Indicator _userApplType;
  Indicator _fareTypeAppl;
  Indicator _mpmInd;
  Indicator _routingInd;
  GlobalDirection _globalDir;
  Directionality _directionalInd;
  Directionality _viaDirectionalInd;
  Directionality _intermDirectionalInd;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  VendorCode _vendor;
  UserApplCode _userAppl;
  TariffCode _ruleTariffCode;
  RoutingNumber _routing;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _viaExceptInd;
  Indicator _stopConnectRestr;
  Indicator _hipCheckAppl;
  Indicator _hipStopTktInd;
  LocKey _viaLoc1;
  LocKey _viaLoc2;

  LocKey _intermediateLoc1;
  LocKey _intermediateLoc2;
  Indicator _ctmCheckAppl;
  Indicator _ctmStopTktInd;
  Indicator _backhaulCheckAppl;
  Indicator _backhaulStopTktInd;
  Indicator _dmcCheckAppl;
  Indicator _dmcStopTktInd;
  Indicator _comCheckAppl;
  Indicator _comStopTktInd;
  Indicator _cpmCheckAppl;
  Indicator _cpmStopTktInd;
  Indicator _serviceRestr;
  Indicator _nonStopInd;
  Indicator _directInd;
  Indicator _constructPointInd;
  Indicator _exceptCxrFltRestr;
  Indicator _exceptSecondaryCxr;
  Directionality _betwDirectionalInd;

  Indicator _posExceptInd;
  Indicator _poiExceptInd;
  Indicator _sotiInd;

  LocKey _betwLoc1;
  LocKey _betwLoc2;
  LocKey _posLoc;
  LocKey _poiLoc;
  Indicator _sotoInd;
  Indicator _sitiInd;
  Indicator _sitoInd;
  // Override (From MinimumFares if _applyDefaultLogic == 'N')
  Indicator _applyDefaultLogic;
  Indicator _domAppl;
  Indicator _domExceptInd;
  Indicator _nmlFareCompareInd;
  Indicator _nmlMpmBeforeRtgInd;
  // Children
  std::vector<CarrierCode> _excptCxrs;
  std::vector<RuleNumber> _rules;
  std::vector<std::pair<RuleNumber, Footnote> > _ruleFootnotes;
  std::vector<FareClassCode> _fareClasses;
  std::vector<FareTypeAbbrev> _fareTypes;
  std::vector<MinFareCxrFltRestr*> _cxrFltRestrs;
  std::vector<CarrierCode> _secondaryCxrs;
  std::vector<FareTypeAbbrev> _domFareTypes;

  LocKey _domLoc;

  Indicator _nmlRtgBeforeMpmInd;
  Indicator _nmlHipRestrCompInd;
  Indicator _nmlHipUnrestrCompInd;
  Indicator _nmlHipRbdCompInd;
  Indicator _nmlHipStopCompInd;
  Indicator _nmlHipOrigInd;
  Indicator _nmlHipOrigNationInd;
  Indicator _nmlHipFromInterInd;
  Indicator _nmlHipDestInd;
  Indicator _nmlHipDestNationInd;
  Indicator _nmlHipToInterInd;
  Indicator _nmlHipExemptInterToInter;
  Indicator _spclHipTariffCatInd;
  Indicator _spclHipRuleTrfInd;
  Indicator _spclHipFareClassInd;
  Indicator _spclHip1stCharInd;
  LocKey _spclHipLoc;
  std::string _specialProcessName;
  Indicator _spclHipStopCompInd;
  Indicator _spclHipSpclOnlyInd;
  Indicator _spclHipOrigInd;
  Indicator _spclHipOrigNationInd;
  Indicator _spclHipFromInterInd;
  Indicator _spclHipDestInd;
  Indicator _spclHipDestNationInd;
  Indicator _spclHipToInterInd;
  Indicator _spclHipExemptInterToInter;
  Indicator _nmlCtmRestrCompInd;
  Indicator _nmlCtmUnrestrCompInd;
  Indicator _nmlCtmRbdCompInd;
  Indicator _nmlCtmStopCompInd;
  Indicator _nmlCtmOrigInd;
  Indicator _nmlCtmDestNationInd;
  Indicator _nmlCtmToInterInd;
  Indicator _spclCtmTariffCatInd;
  Indicator _spclCtmRuleTrfInd;
  Indicator _spclCtmFareClassInd;
  Indicator _spclSame1stCharFBInd2;
  Indicator _spclCtmStopCompInd;
  Indicator _spclCtmMktComp;
  Indicator _spclCtmOrigInd;
  Indicator _spclCtmDestNationInd;
  Indicator _spclCtmToInterInd;
  Indicator _cpmExcl;
  Indicator _intHipCheckAppl;
  Indicator _intHipStopTktInd;
  Indicator _intCtmCheckAppl;
  Indicator _intCtmStopTktInd;
  Indicator _intBackhaulChkAppl;
  Indicator _intBackhaulStopTktInd;
  Indicator _intDmcCheckAppl;
  Indicator _intDmcStopTktInd;
  Indicator _intComCheckAppl;
  Indicator _intComStopTktInd;
  Indicator _intCpmCheckAppl;
  Indicator _intCpmStopTktInd;
  Indicator _domFareTypeExcept;
  Indicator _nmlHipOrigNationTktPt;
  Indicator _nmlHipOrigNationStopPt;
  Indicator _nmlHipStopoverPt;
  Indicator _nmlHipTicketedPt;
  Indicator _spclHipOrigNationTktPt;
  Indicator _spclHipOrigNationStopPt;
  Indicator _spclHipStopoverPt;
  Indicator _spclHipTicketedPt;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _textTblVendor);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _governingCarrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _routingTariff1);
    FLATTENIZE(archive, _routingTariff2);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _tariffCat);
    FLATTENIZE(archive, _nmlHipTariffCatInd);
    FLATTENIZE(archive, _nmlCtmTariffCatInd);
    FLATTENIZE(archive, _tktgCarrier);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _ruleTariffCode);
    FLATTENIZE(archive, _fareTypeAppl);
    FLATTENIZE(archive, _mpmInd);
    FLATTENIZE(archive, _routingInd);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _directionalInd);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _viaExceptInd);
    FLATTENIZE(archive, _viaDirectionalInd);
    FLATTENIZE(archive, _viaLoc1);
    FLATTENIZE(archive, _viaLoc2);
    FLATTENIZE(archive, _intermDirectionalInd);
    FLATTENIZE(archive, _intermediateLoc1);
    FLATTENIZE(archive, _intermediateLoc2);
    FLATTENIZE(archive, _stopConnectRestr);
    FLATTENIZE(archive, _hipCheckAppl);
    FLATTENIZE(archive, _hipStopTktInd);
    FLATTENIZE(archive, _ctmCheckAppl);
    FLATTENIZE(archive, _ctmStopTktInd);
    FLATTENIZE(archive, _backhaulCheckAppl);
    FLATTENIZE(archive, _backhaulStopTktInd);
    FLATTENIZE(archive, _dmcCheckAppl);
    FLATTENIZE(archive, _dmcStopTktInd);
    FLATTENIZE(archive, _comCheckAppl);
    FLATTENIZE(archive, _comStopTktInd);
    FLATTENIZE(archive, _cpmCheckAppl);
    FLATTENIZE(archive, _cpmStopTktInd);
    FLATTENIZE(archive, _betwDirectionalInd);
    FLATTENIZE(archive, _betwLoc1);
    FLATTENIZE(archive, _betwLoc2);
    FLATTENIZE(archive, _serviceRestr);
    FLATTENIZE(archive, _nonStopInd);
    FLATTENIZE(archive, _directInd);
    FLATTENIZE(archive, _constructPointInd);
    FLATTENIZE(archive, _exceptCxrFltRestr);
    FLATTENIZE(archive, _exceptSecondaryCxr);
    FLATTENIZE(archive, _posExceptInd);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _poiExceptInd);
    FLATTENIZE(archive, _poiLoc);
    FLATTENIZE(archive, _sotiInd);
    FLATTENIZE(archive, _sotoInd);
    FLATTENIZE(archive, _sitiInd);
    FLATTENIZE(archive, _sitoInd);
    FLATTENIZE(archive, _excptCxrs);
    FLATTENIZE(archive, _rules);
    FLATTENIZE(archive, _ruleFootnotes);
    FLATTENIZE(archive, _fareClasses);
    FLATTENIZE(archive, _fareTypes);
    FLATTENIZE(archive, _cxrFltRestrs);
    FLATTENIZE(archive, _secondaryCxrs);
    FLATTENIZE(archive, _domFareTypes);
    FLATTENIZE(archive, _applyDefaultLogic);
    FLATTENIZE(archive, _domAppl);
    FLATTENIZE(archive, _domExceptInd);
    FLATTENIZE(archive, _domLoc);
    FLATTENIZE(archive, _nmlFareCompareInd);
    FLATTENIZE(archive, _nmlMpmBeforeRtgInd);
    FLATTENIZE(archive, _nmlRtgBeforeMpmInd);
    FLATTENIZE(archive, _nmlHipRestrCompInd);
    FLATTENIZE(archive, _nmlHipUnrestrCompInd);
    FLATTENIZE(archive, _nmlHipRbdCompInd);
    FLATTENIZE(archive, _nmlHipStopCompInd);
    FLATTENIZE(archive, _nmlHipOrigInd);
    FLATTENIZE(archive, _nmlHipOrigNationInd);
    FLATTENIZE(archive, _nmlHipFromInterInd);
    FLATTENIZE(archive, _nmlHipDestInd);
    FLATTENIZE(archive, _nmlHipDestNationInd);
    FLATTENIZE(archive, _nmlHipToInterInd);
    FLATTENIZE(archive, _nmlHipExemptInterToInter);
    FLATTENIZE(archive, _spclHipTariffCatInd);
    FLATTENIZE(archive, _spclHipRuleTrfInd);
    FLATTENIZE(archive, _spclHipFareClassInd);
    FLATTENIZE(archive, _spclHip1stCharInd);
    FLATTENIZE(archive, _spclHipStopCompInd);
    FLATTENIZE(archive, _spclHipSpclOnlyInd);
    FLATTENIZE(archive, _spclHipLoc);
    FLATTENIZE(archive, _spclHipOrigInd);
    FLATTENIZE(archive, _spclHipOrigNationInd);
    FLATTENIZE(archive, _spclHipFromInterInd);
    FLATTENIZE(archive, _spclHipDestInd);
    FLATTENIZE(archive, _spclHipDestNationInd);
    FLATTENIZE(archive, _specialProcessName);
    FLATTENIZE(archive, _spclHipToInterInd);
    FLATTENIZE(archive, _spclHipExemptInterToInter);
    FLATTENIZE(archive, _nmlCtmRestrCompInd);
    FLATTENIZE(archive, _nmlCtmUnrestrCompInd);
    FLATTENIZE(archive, _nmlCtmRbdCompInd);
    FLATTENIZE(archive, _nmlCtmStopCompInd);
    FLATTENIZE(archive, _nmlCtmOrigInd);
    FLATTENIZE(archive, _nmlCtmDestNationInd);
    FLATTENIZE(archive, _nmlCtmToInterInd);
    FLATTENIZE(archive, _spclCtmTariffCatInd);
    FLATTENIZE(archive, _spclCtmRuleTrfInd);
    FLATTENIZE(archive, _spclCtmFareClassInd);
    FLATTENIZE(archive, _spclSame1stCharFBInd2);
    FLATTENIZE(archive, _spclCtmStopCompInd);
    FLATTENIZE(archive, _spclCtmMktComp);
    FLATTENIZE(archive, _spclCtmOrigInd);
    FLATTENIZE(archive, _spclCtmDestNationInd);
    FLATTENIZE(archive, _spclCtmToInterInd);
    FLATTENIZE(archive, _cpmExcl);
    FLATTENIZE(archive, _intHipCheckAppl);
    FLATTENIZE(archive, _intHipStopTktInd);
    FLATTENIZE(archive, _intCtmCheckAppl);
    FLATTENIZE(archive, _intCtmStopTktInd);
    FLATTENIZE(archive, _intBackhaulChkAppl);
    FLATTENIZE(archive, _intBackhaulStopTktInd);
    FLATTENIZE(archive, _intDmcCheckAppl);
    FLATTENIZE(archive, _intDmcStopTktInd);
    FLATTENIZE(archive, _intComCheckAppl);
    FLATTENIZE(archive, _intComStopTktInd);
    FLATTENIZE(archive, _intCpmCheckAppl);
    FLATTENIZE(archive, _intCpmStopTktInd);
    FLATTENIZE(archive, _domFareTypeExcept);
    FLATTENIZE(archive, _nmlHipOrigNationTktPt);
    FLATTENIZE(archive, _nmlHipOrigNationStopPt);
    FLATTENIZE(archive, _nmlHipStopoverPt);
    FLATTENIZE(archive, _nmlHipTicketedPt);
    FLATTENIZE(archive, _spclHipOrigNationTktPt);
    FLATTENIZE(archive, _spclHipOrigNationStopPt);
    FLATTENIZE(archive, _spclHipStopoverPt);
    FLATTENIZE(archive, _spclHipTicketedPt);
  }

protected:
private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer & ptr->_textTblVendor & ptr->_textTblItemNo & ptr->_governingCarrier &
           ptr->_versionDate & ptr->_seqNo & ptr->_createDate & ptr->_expireDate & ptr->_effDate &
           ptr->_discDate & ptr->_vendor & ptr->_routingTariff1 & ptr->_routingTariff2 &
           ptr->_ruleTariff & ptr->_tariffCat & ptr->_nmlHipTariffCatInd &
           ptr->_nmlCtmTariffCatInd & ptr->_tktgCarrier & ptr->_userApplType & ptr->_userAppl &
           ptr->_ruleTariffCode & ptr->_fareTypeAppl & ptr->_mpmInd & ptr->_routingInd &
           ptr->_routing & ptr->_globalDir & ptr->_directionalInd & ptr->_loc1 & ptr->_loc2 &
           ptr->_viaExceptInd & ptr->_viaDirectionalInd & ptr->_viaLoc1 & ptr->_viaLoc2 &
           ptr->_intermDirectionalInd & ptr->_intermediateLoc1 & ptr->_intermediateLoc2 &
           ptr->_stopConnectRestr & ptr->_hipCheckAppl & ptr->_hipStopTktInd & ptr->_ctmCheckAppl &
           ptr->_ctmStopTktInd & ptr->_backhaulCheckAppl & ptr->_backhaulStopTktInd &
           ptr->_dmcCheckAppl & ptr->_dmcStopTktInd & ptr->_comCheckAppl & ptr->_comStopTktInd &
           ptr->_cpmCheckAppl & ptr->_cpmStopTktInd & ptr->_betwDirectionalInd & ptr->_betwLoc1 &
           ptr->_betwLoc2 & ptr->_serviceRestr & ptr->_nonStopInd & ptr->_directInd &
           ptr->_constructPointInd & ptr->_exceptCxrFltRestr & ptr->_exceptSecondaryCxr &
           ptr->_posExceptInd & ptr->_posLoc & ptr->_poiExceptInd & ptr->_poiLoc & ptr->_sotiInd &
           ptr->_sotoInd & ptr->_sitiInd & ptr->_sitoInd & ptr->_excptCxrs & ptr->_rules &
           ptr->_ruleFootnotes &
           ptr->_fareClasses & ptr->_fareTypes & ptr->_cxrFltRestrs & ptr->_secondaryCxrs &
           ptr->_domFareTypes & ptr->_applyDefaultLogic & ptr->_domAppl & ptr->_domExceptInd &
           ptr->_domLoc & ptr->_nmlFareCompareInd & ptr->_nmlMpmBeforeRtgInd &
           ptr->_nmlRtgBeforeMpmInd & ptr->_nmlHipRestrCompInd & ptr->_nmlHipUnrestrCompInd &
           ptr->_nmlHipRbdCompInd & ptr->_nmlHipStopCompInd & ptr->_nmlHipOrigInd &
           ptr->_nmlHipOrigNationInd & ptr->_nmlHipFromInterInd & ptr->_nmlHipDestInd &
           ptr->_nmlHipDestNationInd & ptr->_nmlHipToInterInd & ptr->_nmlHipExemptInterToInter &
           ptr->_spclHipTariffCatInd & ptr->_spclHipRuleTrfInd & ptr->_spclHipFareClassInd &
           ptr->_spclHip1stCharInd & ptr->_spclHipStopCompInd & ptr->_spclHipSpclOnlyInd &
           ptr->_spclHipLoc & ptr->_spclHipOrigInd & ptr->_spclHipOrigNationInd &
           ptr->_spclHipFromInterInd & ptr->_spclHipDestInd & ptr->_spclHipDestNationInd &
           ptr->_specialProcessName & ptr->_spclHipToInterInd & ptr->_spclHipExemptInterToInter &
           ptr->_nmlCtmRestrCompInd & ptr->_nmlCtmUnrestrCompInd & ptr->_nmlCtmRbdCompInd &
           ptr->_nmlCtmStopCompInd & ptr->_nmlCtmOrigInd & ptr->_nmlCtmDestNationInd &
           ptr->_nmlCtmToInterInd & ptr->_spclCtmTariffCatInd & ptr->_spclCtmRuleTrfInd &
           ptr->_spclCtmFareClassInd & ptr->_spclSame1stCharFBInd2 & ptr->_spclCtmStopCompInd &
           ptr->_spclCtmMktComp & ptr->_spclCtmOrigInd & ptr->_spclCtmDestNationInd &
           ptr->_spclCtmToInterInd & ptr->_cpmExcl & ptr->_intHipCheckAppl &
           ptr->_intHipStopTktInd & ptr->_intCtmCheckAppl & ptr->_intCtmStopTktInd &
           ptr->_intBackhaulChkAppl & ptr->_intBackhaulStopTktInd & ptr->_intDmcCheckAppl &
           ptr->_intDmcStopTktInd & ptr->_intComCheckAppl & ptr->_intComStopTktInd &
           ptr->_intCpmCheckAppl & ptr->_intCpmStopTktInd & ptr->_domFareTypeExcept &
           ptr->_nmlHipOrigNationTktPt & ptr->_nmlHipOrigNationStopPt & ptr->_nmlHipStopoverPt &
           ptr->_nmlHipTicketedPt & ptr->_spclHipOrigNationTktPt & ptr->_spclHipOrigNationStopPt &
           ptr->_spclHipStopoverPt & ptr->_spclHipTicketedPt;
  }

  MinFareAppl(const MinFareAppl&);
  MinFareAppl& operator=(const MinFareAppl&);
};
} // namespace tse
