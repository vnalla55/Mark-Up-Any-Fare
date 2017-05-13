//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/FareCalcConfigSeg.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class FareCalcConfig
{
public:
  FareCalcConfig()
    : _userApplType(' '),
      _itinDisplayInd(' '),
      _wpPsgTypDisplay(' '),
      _itinHeaderTextInd(' '),
      _wpConnectionInd(' '),
      _wpChildInfantFareBasis(' '),
      _fareBasisTktDesLng(' '),
      _truePsgrTypeInd(' '),
      _fareTaxTotalInd(' '),
      _noofTaxBoxes(' '),
      _baseTaxEquivTotalLength(0),
      _taxPlacementInd(' '),
      _taxCurCodeDisplayInd(' '),
      _zpAmountDisplayInd(' '),
      _taxExemptionInd(' '),
      _taxExemptBreakdownInd(' '),
      _fcPsgTypDisplay(' '),
      _fcChildInfantFareBasis(' '),
      _fcConnectionInd(' '),
      _domesticNUC(' '),
      _tvlCommencementDate(' '),
      _wrapAround(' '),
      _multiSurchargeSpacing(' '),
      _domesticISI(' '),
      _internationalISI(' '),
      _displayBSR(' '),
      _endorsements(' '),
      _warningMessages(' '),
      _lastDayTicketDisplay(' '),
      _lastDayTicketOutput(' '),
      _reservationOverride(' '),
      _fareBasisDisplayOption(' '),
      _globalSidetripInd(' '),
      _wpaPermitted(' '),
      _wpaPsgDtlFormat(' '),
      _wpaFareLinePsgType(' '),
      _wpaFareLineHdr(' '),
      _wpaPrimePsgRefNo(' '),
      _wpa2ndPsgRefNo(' '),
      _wpaFareOptionMaxNo(0),
      _wpaSort(' '),
      _wpaShowDupAmounts(' '),
      _wpaPsgLineBreak(' '),
      _wpaPsgMultiLineBreak(' '),
      _wpaNoMatchHigherCabinFare(' '),
      _wpaStoreWithoutRebook(' '),
      _wpaAccTvlOption(' '),
      _wpaAccTvlCat13(' '),
      _wpaRoInd(' '),
      _wpNoMatchPermitted(' '),
      _wpPsgDtlFormat(' '),
      _wpFareLinePsgType(' '),
      _wpFareLineHdr(' '),
      _wpPrimePsgRefNo(' '),
      _wp2ndPsgRefNo(' '),
      _wpFareOptionMaxNo(0),
      _wpSort(' '),
      _wpShowDupAmounts(' '),
      _wpPsgLineBreak(' '),
      _wpPsgMultiLineBreak(' '),
      _wpNoMatchHigherCabinFare(' '),
      _wpStoreWithoutRebook(' '),
      _wpAccTvlOption(' '),
      _wpAccTvlCat13(' '),
      _wpRoInd(' '),
      _valCxrDisplayOpt(' '),
      _negPermitted(' '),
      _noMatchAvail(' '),
      _wpWpaTrailerMsg(' '),
      _interlineTktPermitted(' '),
      _participatingAgreement(' '),
      _applyDomesticMultiCurrency(' '),
      _applyIntlMultiCurrency(' '),
      _ietPriceInterlineActive(' '),
      _valueCodeBase(' ')
  {
  }

  ~FareCalcConfig()
  {
    std::vector<FareCalcConfigSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    { // Nuke 'em!
      delete *SegIt;
    }
  }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  Indicator& itinDisplayInd() { return _itinDisplayInd; }
  const Indicator& itinDisplayInd() const { return _itinDisplayInd; }

  Indicator& wpPsgTypDisplay() { return _wpPsgTypDisplay; }
  const Indicator& wpPsgTypDisplay() const { return _wpPsgTypDisplay; }

  Indicator& itinHeaderTextInd() { return _itinHeaderTextInd; }
  const Indicator& itinHeaderTextInd() const { return _itinHeaderTextInd; }

  Indicator& wpConnectionInd() { return _wpConnectionInd; }
  const Indicator& wpConnectionInd() const { return _wpConnectionInd; }

  Indicator& wpChildInfantFareBasis() { return _wpChildInfantFareBasis; }
  const Indicator& wpChildInfantFareBasis() const { return _wpChildInfantFareBasis; }

  Indicator& fareBasisTktDesLng() { return _fareBasisTktDesLng; }
  const Indicator& fareBasisTktDesLng() const { return _fareBasisTktDesLng; }

  Indicator& truePsgrTypeInd() { return _truePsgrTypeInd; }
  const Indicator& truePsgrTypeInd() const { return _truePsgrTypeInd; }

  Indicator& fareTaxTotalInd() { return _fareTaxTotalInd; }
  const Indicator& fareTaxTotalInd() const { return _fareTaxTotalInd; }

  Indicator& noofTaxBoxes() { return _noofTaxBoxes; }
  const Indicator& noofTaxBoxes() const { return _noofTaxBoxes; }

  int& baseTaxEquivTotalLength() { return _baseTaxEquivTotalLength; }
  const int& baseTaxEquivTotalLength() const { return _baseTaxEquivTotalLength; }

  Indicator& taxPlacementInd() { return _taxPlacementInd; }
  const Indicator& taxPlacementInd() const { return _taxPlacementInd; }

  Indicator& taxCurCodeDisplayInd() { return _taxCurCodeDisplayInd; }
  const Indicator& taxCurCodeDisplayInd() const { return _taxCurCodeDisplayInd; }

  Indicator& zpAmountDisplayInd() { return _zpAmountDisplayInd; }
  const Indicator& zpAmountDisplayInd() const { return _zpAmountDisplayInd; }

  Indicator& taxExemptionInd() { return _taxExemptionInd; }
  const Indicator& taxExemptionInd() const { return _taxExemptionInd; }

  Indicator& taxExemptBreakdownInd() { return _taxExemptBreakdownInd; }
  const Indicator& taxExemptBreakdownInd() const { return _taxExemptBreakdownInd; }

  Indicator& fcPsgTypDisplay() { return _fcPsgTypDisplay; }
  const Indicator& fcPsgTypDisplay() const { return _fcPsgTypDisplay; }

  Indicator& fcChildInfantFareBasis() { return _fcChildInfantFareBasis; }
  const Indicator& fcChildInfantFareBasis() const { return _fcChildInfantFareBasis; }

  Indicator& fcConnectionInd() { return _fcConnectionInd; }
  const Indicator& fcConnectionInd() const { return _fcConnectionInd; }

  Indicator& domesticNUC() { return _domesticNUC; }
  const Indicator& domesticNUC() const { return _domesticNUC; }

  Indicator& tvlCommencementDate() { return _tvlCommencementDate; }
  const Indicator& tvlCommencementDate() const { return _tvlCommencementDate; }

  Indicator& wrapAround() { return _wrapAround; }
  const Indicator& wrapAround() const { return _wrapAround; }

  Indicator& multiSurchargeSpacing() { return _multiSurchargeSpacing; }
  const Indicator& multiSurchargeSpacing() const { return _multiSurchargeSpacing; }

  Indicator& domesticISI() { return _domesticISI; }
  const Indicator& domesticISI() const { return _domesticISI; }

  Indicator& internationalISI() { return _internationalISI; }
  const Indicator& internationalISI() const { return _internationalISI; }

  Indicator& displayBSR() { return _displayBSR; }
  const Indicator& displayBSR() const { return _displayBSR; }

  Indicator& endorsements() { return _endorsements; }
  const Indicator& endorsements() const { return _endorsements; }

  Indicator& warningMessages() { return _warningMessages; }
  const Indicator& warningMessages() const { return _warningMessages; }

  Indicator& lastDayTicketDisplay() { return _lastDayTicketDisplay; }
  const Indicator& lastDayTicketDisplay() const { return _lastDayTicketDisplay; }

  Indicator& lastDayTicketOutput() { return _lastDayTicketOutput; }
  const Indicator& lastDayTicketOutput() const { return _lastDayTicketOutput; }

  Indicator& reservationOverride() { return _reservationOverride; }
  const Indicator& reservationOverride() const { return _reservationOverride; }

  Indicator& fareBasisDisplayOption() { return _fareBasisDisplayOption; }
  const Indicator& fareBasisDisplayOption() const { return _fareBasisDisplayOption; }

  Indicator& globalSidetripInd() { return _globalSidetripInd; }
  const Indicator& globalSidetripInd() const { return _globalSidetripInd; }

  Indicator& wpaPermitted() { return _wpaPermitted; }
  const Indicator& wpaPermitted() const { return _wpaPermitted; }

  Indicator& wpaPsgDtlFormat() { return _wpaPsgDtlFormat; }
  const Indicator& wpaPsgDtlFormat() const { return _wpaPsgDtlFormat; }

  Indicator& wpaFareLinePsgType() { return _wpaFareLinePsgType; }
  const Indicator& wpaFareLinePsgType() const { return _wpaFareLinePsgType; }

  Indicator& wpaFareLineHdr() { return _wpaFareLineHdr; }
  const Indicator& wpaFareLineHdr() const { return _wpaFareLineHdr; }

  Indicator& wpaPrimePsgRefNo() { return _wpaPrimePsgRefNo; }
  const Indicator& wpaPrimePsgRefNo() const { return _wpaPrimePsgRefNo; }

  Indicator& wpa2ndPsgRefNo() { return _wpa2ndPsgRefNo; }
  const Indicator& wpa2ndPsgRefNo() const { return _wpa2ndPsgRefNo; }

  int& wpaFareOptionMaxNo() { return _wpaFareOptionMaxNo; }
  const int& wpaFareOptionMaxNo() const { return _wpaFareOptionMaxNo; }

  Indicator& wpaSort() { return _wpaSort; }
  const Indicator& wpaSort() const { return _wpaSort; }

  Indicator& wpaShowDupAmounts() { return _wpaShowDupAmounts; }
  const Indicator& wpaShowDupAmounts() const { return _wpaShowDupAmounts; }

  Indicator& wpaPsgLineBreak() { return _wpaPsgLineBreak; }
  const Indicator& wpaPsgLineBreak() const { return _wpaPsgLineBreak; }

  Indicator& wpaPsgMultiLineBreak() { return _wpaPsgMultiLineBreak; }
  const Indicator& wpaPsgMultiLineBreak() const { return _wpaPsgMultiLineBreak; }

  Indicator& wpaNoMatchHigherCabinFare() { return _wpaNoMatchHigherCabinFare; }
  const Indicator& wpaNoMatchHigherCabinFare() const { return _wpaNoMatchHigherCabinFare; }

  Indicator& wpaStoreWithoutRebook() { return _wpaStoreWithoutRebook; }
  const Indicator& wpaStoreWithoutRebook() const { return _wpaStoreWithoutRebook; }

  Indicator& wpaAccTvlOption() { return _wpaAccTvlOption; }
  const Indicator& wpaAccTvlOption() const { return _wpaAccTvlOption; }

  Indicator& wpaAccTvlCat13() { return _wpaAccTvlCat13; }
  const Indicator& wpaAccTvlCat13() const { return _wpaAccTvlCat13; }

  Indicator& wpaRoInd() { return _wpaRoInd; }
  const Indicator& wpaRoInd() const { return _wpaRoInd; }

  Indicator& wpNoMatchPermitted() { return _wpNoMatchPermitted; }
  const Indicator& wpNoMatchPermitted() const { return _wpNoMatchPermitted; }

  Indicator& wpPsgDtlFormat() { return _wpPsgDtlFormat; }
  const Indicator& wpPsgDtlFormat() const { return _wpPsgDtlFormat; }

  Indicator& wpFareLinePsgType() { return _wpFareLinePsgType; }
  const Indicator& wpFareLinePsgType() const { return _wpFareLinePsgType; }

  Indicator& wpFareLineHdr() { return _wpFareLineHdr; }
  const Indicator& wpFareLineHdr() const { return _wpFareLineHdr; }

  Indicator& wpPrimePsgRefNo() { return _wpPrimePsgRefNo; }
  const Indicator& wpPrimePsgRefNo() const { return _wpPrimePsgRefNo; }

  Indicator& wp2ndPsgRefNo() { return _wp2ndPsgRefNo; }
  const Indicator& wp2ndPsgRefNo() const { return _wp2ndPsgRefNo; }

  int& wpFareOptionMaxNo() { return _wpFareOptionMaxNo; }
  const int& wpFareOptionMaxNo() const { return _wpFareOptionMaxNo; }

  Indicator& wpSort() { return _wpSort; }
  const Indicator& wpSort() const { return _wpSort; }

  Indicator& wpShowDupAmounts() { return _wpShowDupAmounts; }
  const Indicator& wpShowDupAmounts() const { return _wpShowDupAmounts; }

  Indicator& wpPsgLineBreak() { return _wpPsgLineBreak; }
  const Indicator& wpPsgLineBreak() const { return _wpPsgLineBreak; }

  Indicator& wpPsgMultiLineBreak() { return _wpPsgMultiLineBreak; }
  const Indicator& wpPsgMultiLineBreak() const { return _wpPsgMultiLineBreak; }

  Indicator& wpNoMatchHigherCabinFare() { return _wpNoMatchHigherCabinFare; }
  const Indicator& wpNoMatchHigherCabinFare() const { return _wpNoMatchHigherCabinFare; }

  Indicator& wpStoreWithoutRebook() { return _wpStoreWithoutRebook; }
  const Indicator& wpStoreWithoutRebook() const { return _wpStoreWithoutRebook; }

  Indicator& wpAccTvlOption() { return _wpAccTvlOption; }
  const Indicator& wpAccTvlOption() const { return _wpAccTvlOption; }

  Indicator& wpAccTvlCat13() { return _wpAccTvlCat13; }
  const Indicator& wpAccTvlCat13() const { return _wpAccTvlCat13; }

  Indicator& wpRoInd() { return _wpRoInd; }
  const Indicator& wpRoInd() const { return _wpRoInd; }

  Indicator& valCxrDisplayOpt() { return _valCxrDisplayOpt; }
  const Indicator& valCxrDisplayOpt() const { return _valCxrDisplayOpt; }

  Indicator& negPermitted() { return _negPermitted; }
  const Indicator& negPermitted() const { return _negPermitted; }

  Indicator& noMatchAvail() { return _noMatchAvail; }
  const Indicator& noMatchAvail() const { return _noMatchAvail; }

  Indicator& wpWpaTrailerMsg() { return _wpWpaTrailerMsg; }
  const Indicator& wpWpaTrailerMsg() const { return _wpWpaTrailerMsg; }

  Indicator& interlineTktPermitted() { return _interlineTktPermitted; }
  const Indicator& interlineTktPermitted() const { return _interlineTktPermitted; }

  Indicator& participatingAgreement() { return _participatingAgreement; }
  const Indicator& participatingAgreement() const { return _participatingAgreement; }

  Indicator& applyDomesticMultiCurrency() { return _applyDomesticMultiCurrency; }
  const Indicator& applyDomesticMultiCurrency() const { return _applyDomesticMultiCurrency; }

  Indicator& applyIntlMultiCurrency() { return _applyIntlMultiCurrency; }
  const Indicator& applyIntlMultiCurrency() const { return _applyIntlMultiCurrency; }

  Indicator& ietPriceInterlineActive() { return _ietPriceInterlineActive; }
  const Indicator& ietPriceInterlineActive() const { return _ietPriceInterlineActive; }

  Indicator& valueCodeBase() { return _valueCodeBase; }
  const Indicator& valueCodeBase() const { return _valueCodeBase; }

  std::vector<FareCalcConfigSeg*>& segs() { return _segs; }
  const std::vector<FareCalcConfigSeg*>& segs() const { return _segs; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  bool operator==(const FareCalcConfig& rhs) const
  {
    bool eq =
        ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
         (_pseudoCity == rhs._pseudoCity) && (_loc1 == rhs._loc1) &&
         (_itinDisplayInd == rhs._itinDisplayInd) && (_wpPsgTypDisplay == rhs._wpPsgTypDisplay) &&
         (_itinHeaderTextInd == rhs._itinHeaderTextInd) &&
         (_wpConnectionInd == rhs._wpConnectionInd) &&
         (_wpChildInfantFareBasis == rhs._wpChildInfantFareBasis) &&
         (_fareBasisTktDesLng == rhs._fareBasisTktDesLng) &&
         (_truePsgrTypeInd == rhs._truePsgrTypeInd) && (_fareTaxTotalInd == rhs._fareTaxTotalInd) &&
         (_noofTaxBoxes == rhs._noofTaxBoxes) &&
         (_baseTaxEquivTotalLength == rhs._baseTaxEquivTotalLength) &&
         (_taxPlacementInd == rhs._taxPlacementInd) &&
         (_taxCurCodeDisplayInd == rhs._taxCurCodeDisplayInd) &&
         (_zpAmountDisplayInd == rhs._zpAmountDisplayInd) &&
         (_taxExemptionInd == rhs._taxExemptionInd) &&
         (_taxExemptBreakdownInd == rhs._taxExemptBreakdownInd) &&
         (_fcPsgTypDisplay == rhs._fcPsgTypDisplay) &&
         (_fcChildInfantFareBasis == rhs._fcChildInfantFareBasis) &&
         (_fcConnectionInd == rhs._fcConnectionInd) && (_domesticNUC == rhs._domesticNUC) &&
         (_tvlCommencementDate == rhs._tvlCommencementDate) && (_wrapAround == rhs._wrapAround) &&
         (_multiSurchargeSpacing == rhs._multiSurchargeSpacing) &&
         (_domesticISI == rhs._domesticISI) && (_internationalISI == rhs._internationalISI) &&
         (_displayBSR == rhs._displayBSR) && (_endorsements == rhs._endorsements) &&
         (_warningMessages == rhs._warningMessages) &&
         (_lastDayTicketDisplay == rhs._lastDayTicketDisplay) &&
         (_lastDayTicketOutput == rhs._lastDayTicketOutput) &&
         (_reservationOverride == rhs._reservationOverride) &&
         (_fareBasisDisplayOption == rhs._fareBasisDisplayOption) &&
         (_globalSidetripInd == rhs._globalSidetripInd) && (_wpaPermitted == rhs._wpaPermitted) &&
         (_wpaPsgDtlFormat == rhs._wpaPsgDtlFormat) &&
         (_wpaFareLinePsgType == rhs._wpaFareLinePsgType) &&
         (_wpaFareLineHdr == rhs._wpaFareLineHdr) && (_wpaPrimePsgRefNo == rhs._wpaPrimePsgRefNo) &&
         (_wpa2ndPsgRefNo == rhs._wpa2ndPsgRefNo) &&
         (_wpaFareOptionMaxNo == rhs._wpaFareOptionMaxNo) && (_wpaSort == rhs._wpaSort) &&
         (_wpaShowDupAmounts == rhs._wpaShowDupAmounts) &&
         (_wpaPsgLineBreak == rhs._wpaPsgLineBreak) &&
         (_wpaPsgMultiLineBreak == rhs._wpaPsgMultiLineBreak) &&
         (_wpaNoMatchHigherCabinFare == rhs._wpaNoMatchHigherCabinFare) &&
         (_wpaStoreWithoutRebook == rhs._wpaStoreWithoutRebook) &&
         (_wpaAccTvlOption == rhs._wpaAccTvlOption) && (_wpaAccTvlCat13 == rhs._wpaAccTvlCat13) &&
         (_wpaRoInd == rhs._wpaRoInd) && (_wpNoMatchPermitted == rhs._wpNoMatchPermitted) &&
         (_wpPsgDtlFormat == rhs._wpPsgDtlFormat) &&
         (_wpFareLinePsgType == rhs._wpFareLinePsgType) && (_wpFareLineHdr == rhs._wpFareLineHdr) &&
         (_wpPrimePsgRefNo == rhs._wpPrimePsgRefNo) && (_wp2ndPsgRefNo == rhs._wp2ndPsgRefNo) &&
         (_wpFareOptionMaxNo == rhs._wpFareOptionMaxNo) && (_wpSort == rhs._wpSort) &&
         (_wpShowDupAmounts == rhs._wpShowDupAmounts) && (_wpPsgLineBreak == rhs._wpPsgLineBreak) &&
         (_wpPsgMultiLineBreak == rhs._wpPsgMultiLineBreak) &&
         (_wpNoMatchHigherCabinFare == rhs._wpNoMatchHigherCabinFare) &&
         (_wpStoreWithoutRebook == rhs._wpStoreWithoutRebook) &&
         (_wpAccTvlOption == rhs._wpAccTvlOption) && (_wpAccTvlCat13 == rhs._wpAccTvlCat13) &&
         (_wpRoInd == rhs._wpRoInd) && (_valCxrDisplayOpt == rhs._valCxrDisplayOpt) &&
         (_negPermitted == rhs._negPermitted) && (_noMatchAvail == rhs._noMatchAvail) &&
         (_wpWpaTrailerMsg == rhs._wpWpaTrailerMsg) &&
         (_interlineTktPermitted == rhs._interlineTktPermitted) &&
         (_participatingAgreement == rhs._participatingAgreement) &&
         (_applyDomesticMultiCurrency == rhs._applyDomesticMultiCurrency) &&
         (_applyIntlMultiCurrency == rhs._applyIntlMultiCurrency) &&
         (_ietPriceInterlineActive == rhs._ietPriceInterlineActive) &&
         (_valueCodeBase == rhs._valueCodeBase) && (_createDate == rhs._createDate) &&
         (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(FareCalcConfig& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "DEFG";
    obj._pseudoCity = "HIJKL";
    LocKey::dummyData(obj._loc1);
    obj._itinDisplayInd = 'M';
    obj._wpPsgTypDisplay = 'N';
    obj._itinHeaderTextInd = 'O';
    obj._wpConnectionInd = 'P';
    obj._wpChildInfantFareBasis = 'Q';
    obj._fareBasisTktDesLng = 'R';
    obj._truePsgrTypeInd = 'S';
    obj._fareTaxTotalInd = 'T';
    obj._noofTaxBoxes = 1;
    obj._baseTaxEquivTotalLength = 'U';
    obj._taxPlacementInd = 'V';
    obj._taxCurCodeDisplayInd = 'W';
    obj._zpAmountDisplayInd = 'X';
    obj._taxExemptionInd = 'Y';
    obj._taxExemptBreakdownInd = 'Z';
    obj._fcPsgTypDisplay = 'a';
    obj._fcChildInfantFareBasis = 'b';
    obj._fcConnectionInd = 'c';
    obj._domesticNUC = 'd';
    obj._tvlCommencementDate = 'e';
    obj._wrapAround = 'f';
    obj._multiSurchargeSpacing = 'g';
    obj._domesticISI = 'h';
    obj._internationalISI = 'i';
    obj._displayBSR = 'j';
    obj._endorsements = 'k';
    obj._warningMessages = 'l';
    obj._lastDayTicketDisplay = 'm';
    obj._lastDayTicketOutput = 'n';
    obj._reservationOverride = 'o';
    obj._fareBasisDisplayOption = 'p';
    obj._globalSidetripInd = 'q';
    obj._wpaPermitted = 'r';
    obj._wpaPsgDtlFormat = 's';
    obj._wpaFareLinePsgType = 't';
    obj._wpaFareLineHdr = 'u';
    obj._wpaPrimePsgRefNo = 'v';
    obj._wpa2ndPsgRefNo = 'w';
    obj._wpaFareOptionMaxNo = 2;
    obj._wpaSort = 'x';
    obj._wpaShowDupAmounts = 'y';
    obj._wpaPsgLineBreak = 'z';
    obj._wpaPsgMultiLineBreak = '0';
    obj._wpaNoMatchHigherCabinFare = '1';
    obj._wpaStoreWithoutRebook = '2';
    obj._wpaAccTvlOption = '3';
    obj._wpaAccTvlCat13 = '4';
    obj._wpaRoInd = '5';
    obj._wpNoMatchPermitted = '6';
    obj._wpPsgDtlFormat = '7';
    obj._wpFareLinePsgType = '8';
    obj._wpFareLineHdr = '9';
    obj._wpPrimePsgRefNo = 'A';
    obj._wp2ndPsgRefNo = 'B';
    obj._wpFareOptionMaxNo = 3;
    obj._wpSort = 'C';
    obj._wpShowDupAmounts = 'D';
    obj._wpPsgLineBreak = 'E';
    obj._wpPsgMultiLineBreak = 'F';
    obj._wpNoMatchHigherCabinFare = 'G';
    obj._wpStoreWithoutRebook = 'H';
    obj._wpAccTvlOption = 'I';
    obj._wpAccTvlCat13 = 'J';
    obj._wpRoInd = 'K';
    obj._valCxrDisplayOpt = 'L';
    obj._negPermitted = 'N';
    obj._noMatchAvail = 'M';
    obj._wpWpaTrailerMsg = 'O';
    obj._interlineTktPermitted = 'P';
    obj._participatingAgreement = 'Q';
    obj._applyDomesticMultiCurrency = 'R';
    obj._applyIntlMultiCurrency = 'S';
    obj._ietPriceInterlineActive = 'T';
    obj._valueCodeBase = 'U';
    obj._createDate = time(nullptr);

    FareCalcConfigSeg* fccs1 = new FareCalcConfigSeg;
    FareCalcConfigSeg* fccs2 = new FareCalcConfigSeg;

    FareCalcConfigSeg::dummyData(*fccs1);
    FareCalcConfigSeg::dummyData(*fccs2);

    obj._segs.push_back(fccs1);
    obj._segs.push_back(fccs2);
  }

protected:
  // Join fields (w/Child: FARECALCCONFIGSEG)
  Indicator _userApplType;
  UserApplCode _userAppl;
  PseudoCityCode _pseudoCity;
  LocKey _loc1;
  Indicator _itinDisplayInd;
  Indicator _wpPsgTypDisplay;
  Indicator _itinHeaderTextInd;
  Indicator _wpConnectionInd;
  Indicator _wpChildInfantFareBasis;
  Indicator _fareBasisTktDesLng;
  Indicator _truePsgrTypeInd;
  Indicator _fareTaxTotalInd;
  Indicator _noofTaxBoxes;
  int _baseTaxEquivTotalLength;
  Indicator _taxPlacementInd;
  Indicator _taxCurCodeDisplayInd;
  Indicator _zpAmountDisplayInd;
  Indicator _taxExemptionInd;
  Indicator _taxExemptBreakdownInd;
  Indicator _fcPsgTypDisplay;
  Indicator _fcChildInfantFareBasis;
  Indicator _fcConnectionInd;
  Indicator _domesticNUC;
  Indicator _tvlCommencementDate;
  Indicator _wrapAround;
  Indicator _multiSurchargeSpacing;
  Indicator _domesticISI;
  Indicator _internationalISI;
  Indicator _displayBSR;
  Indicator _endorsements;
  Indicator _warningMessages;
  Indicator _lastDayTicketDisplay;
  Indicator _lastDayTicketOutput;
  Indicator _reservationOverride;
  Indicator _fareBasisDisplayOption;
  Indicator _globalSidetripInd;
  Indicator _wpaPermitted;
  Indicator _wpaPsgDtlFormat;
  Indicator _wpaFareLinePsgType;
  Indicator _wpaFareLineHdr;
  Indicator _wpaPrimePsgRefNo;
  Indicator _wpa2ndPsgRefNo;
  int _wpaFareOptionMaxNo;
  Indicator _wpaSort;
  Indicator _wpaShowDupAmounts;
  Indicator _wpaPsgLineBreak;
  Indicator _wpaPsgMultiLineBreak;
  Indicator _wpaNoMatchHigherCabinFare;
  Indicator _wpaStoreWithoutRebook;
  Indicator _wpaAccTvlOption;
  Indicator _wpaAccTvlCat13;
  Indicator _wpaRoInd;
  Indicator _wpNoMatchPermitted;
  Indicator _wpPsgDtlFormat;
  Indicator _wpFareLinePsgType;
  Indicator _wpFareLineHdr;
  Indicator _wpPrimePsgRefNo;
  Indicator _wp2ndPsgRefNo;
  int _wpFareOptionMaxNo;
  Indicator _wpSort;
  Indicator _wpShowDupAmounts;
  Indicator _wpPsgLineBreak;
  Indicator _wpPsgMultiLineBreak;
  Indicator _wpNoMatchHigherCabinFare;
  Indicator _wpStoreWithoutRebook;
  Indicator _wpAccTvlOption;
  Indicator _wpAccTvlCat13;
  Indicator _wpRoInd;
  Indicator _valCxrDisplayOpt;
  Indicator _negPermitted;
  Indicator _noMatchAvail;
  Indicator _wpWpaTrailerMsg;
  Indicator _interlineTktPermitted;
  Indicator _participatingAgreement;
  Indicator _applyDomesticMultiCurrency;
  Indicator _applyIntlMultiCurrency;
  Indicator _ietPriceInterlineActive;
  Indicator _valueCodeBase;
  DateTime _createDate;
  std::vector<FareCalcConfigSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _itinDisplayInd);
    FLATTENIZE(archive, _wpPsgTypDisplay);
    FLATTENIZE(archive, _itinHeaderTextInd);
    FLATTENIZE(archive, _wpConnectionInd);
    FLATTENIZE(archive, _wpChildInfantFareBasis);
    FLATTENIZE(archive, _fareBasisTktDesLng);
    FLATTENIZE(archive, _truePsgrTypeInd);
    FLATTENIZE(archive, _fareTaxTotalInd);
    FLATTENIZE(archive, _noofTaxBoxes);
    FLATTENIZE(archive, _baseTaxEquivTotalLength);
    FLATTENIZE(archive, _taxPlacementInd);
    FLATTENIZE(archive, _taxCurCodeDisplayInd);
    FLATTENIZE(archive, _zpAmountDisplayInd);
    FLATTENIZE(archive, _taxExemptionInd);
    FLATTENIZE(archive, _taxExemptBreakdownInd);
    FLATTENIZE(archive, _fcPsgTypDisplay);
    FLATTENIZE(archive, _fcChildInfantFareBasis);
    FLATTENIZE(archive, _fcConnectionInd);
    FLATTENIZE(archive, _domesticNUC);
    FLATTENIZE(archive, _tvlCommencementDate);
    FLATTENIZE(archive, _wrapAround);
    FLATTENIZE(archive, _multiSurchargeSpacing);
    FLATTENIZE(archive, _domesticISI);
    FLATTENIZE(archive, _internationalISI);
    FLATTENIZE(archive, _displayBSR);
    FLATTENIZE(archive, _endorsements);
    FLATTENIZE(archive, _warningMessages);
    FLATTENIZE(archive, _lastDayTicketDisplay);
    FLATTENIZE(archive, _lastDayTicketOutput);
    FLATTENIZE(archive, _reservationOverride);
    FLATTENIZE(archive, _fareBasisDisplayOption);
    FLATTENIZE(archive, _globalSidetripInd);
    FLATTENIZE(archive, _wpaPermitted);
    FLATTENIZE(archive, _wpaPsgDtlFormat);
    FLATTENIZE(archive, _wpaFareLinePsgType);
    FLATTENIZE(archive, _wpaFareLineHdr);
    FLATTENIZE(archive, _wpaPrimePsgRefNo);
    FLATTENIZE(archive, _wpa2ndPsgRefNo);
    FLATTENIZE(archive, _wpaFareOptionMaxNo);
    FLATTENIZE(archive, _wpaSort);
    FLATTENIZE(archive, _wpaShowDupAmounts);
    FLATTENIZE(archive, _wpaPsgLineBreak);
    FLATTENIZE(archive, _wpaPsgMultiLineBreak);
    FLATTENIZE(archive, _wpaNoMatchHigherCabinFare);
    FLATTENIZE(archive, _wpaStoreWithoutRebook);
    FLATTENIZE(archive, _wpaAccTvlOption);
    FLATTENIZE(archive, _wpaAccTvlCat13);
    FLATTENIZE(archive, _wpaRoInd);
    FLATTENIZE(archive, _wpNoMatchPermitted);
    FLATTENIZE(archive, _wpPsgDtlFormat);
    FLATTENIZE(archive, _wpFareLinePsgType);
    FLATTENIZE(archive, _wpFareLineHdr);
    FLATTENIZE(archive, _wpPrimePsgRefNo);
    FLATTENIZE(archive, _wp2ndPsgRefNo);
    FLATTENIZE(archive, _wpFareOptionMaxNo);
    FLATTENIZE(archive, _wpSort);
    FLATTENIZE(archive, _wpShowDupAmounts);
    FLATTENIZE(archive, _wpPsgLineBreak);
    FLATTENIZE(archive, _wpPsgMultiLineBreak);
    FLATTENIZE(archive, _wpNoMatchHigherCabinFare);
    FLATTENIZE(archive, _wpStoreWithoutRebook);
    FLATTENIZE(archive, _wpAccTvlOption);
    FLATTENIZE(archive, _wpAccTvlCat13);
    FLATTENIZE(archive, _wpRoInd);
    FLATTENIZE(archive, _valCxrDisplayOpt);
    FLATTENIZE(archive, _negPermitted);
    FLATTENIZE(archive, _noMatchAvail);
    FLATTENIZE(archive, _wpWpaTrailerMsg);
    FLATTENIZE(archive, _interlineTktPermitted);
    FLATTENIZE(archive, _participatingAgreement);
    FLATTENIZE(archive, _applyDomesticMultiCurrency);
    FLATTENIZE(archive, _applyIntlMultiCurrency);
    FLATTENIZE(archive, _ietPriceInterlineActive);
    FLATTENIZE(archive, _valueCodeBase);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _segs);
  }

protected:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_userApplType & ptr->_userAppl & ptr->_pseudoCity & ptr->_loc1 &
           ptr->_itinDisplayInd & ptr->_wpPsgTypDisplay & ptr->_itinHeaderTextInd &
           ptr->_wpConnectionInd & ptr->_wpChildInfantFareBasis & ptr->_fareBasisTktDesLng &
           ptr->_truePsgrTypeInd & ptr->_fareTaxTotalInd & ptr->_noofTaxBoxes &
           ptr->_baseTaxEquivTotalLength & ptr->_taxPlacementInd & ptr->_taxCurCodeDisplayInd &
           ptr->_zpAmountDisplayInd & ptr->_taxExemptionInd & ptr->_taxExemptBreakdownInd &
           ptr->_fcPsgTypDisplay & ptr->_fcChildInfantFareBasis & ptr->_fcConnectionInd &
           ptr->_domesticNUC & ptr->_tvlCommencementDate & ptr->_wrapAround &
           ptr->_multiSurchargeSpacing & ptr->_domesticISI & ptr->_internationalISI &
           ptr->_displayBSR & ptr->_endorsements & ptr->_warningMessages &
           ptr->_lastDayTicketDisplay & ptr->_lastDayTicketOutput & ptr->_reservationOverride &
           ptr->_fareBasisDisplayOption & ptr->_globalSidetripInd & ptr->_wpaPermitted &
           ptr->_wpaPsgDtlFormat & ptr->_wpaFareLinePsgType & ptr->_wpaFareLineHdr &
           ptr->_wpaPrimePsgRefNo & ptr->_wpa2ndPsgRefNo & ptr->_wpaFareOptionMaxNo &
           ptr->_wpaSort & ptr->_wpaShowDupAmounts & ptr->_wpaPsgLineBreak &
           ptr->_wpaPsgMultiLineBreak & ptr->_wpaNoMatchHigherCabinFare &
           ptr->_wpaStoreWithoutRebook & ptr->_wpaAccTvlOption & ptr->_wpaAccTvlCat13 &
           ptr->_wpaRoInd & ptr->_wpNoMatchPermitted & ptr->_wpPsgDtlFormat &
           ptr->_wpFareLinePsgType & ptr->_wpFareLineHdr & ptr->_wpPrimePsgRefNo &
           ptr->_wp2ndPsgRefNo & ptr->_wpFareOptionMaxNo & ptr->_wpSort & ptr->_wpShowDupAmounts &
           ptr->_wpPsgLineBreak & ptr->_wpPsgMultiLineBreak & ptr->_wpNoMatchHigherCabinFare &
           ptr->_wpStoreWithoutRebook & ptr->_wpAccTvlOption & ptr->_wpAccTvlCat13 & ptr->_wpRoInd &
           ptr->_valCxrDisplayOpt & ptr->_negPermitted & ptr->_noMatchAvail &
           ptr->_wpWpaTrailerMsg & ptr->_interlineTktPermitted & ptr->_participatingAgreement &
           ptr->_applyDomesticMultiCurrency & ptr->_applyIntlMultiCurrency &
           ptr->_ietPriceInterlineActive & ptr->_valueCodeBase & ptr->_createDate & ptr->_segs;
  }

private:
  FareCalcConfig(const FareCalcConfig&);
  FareCalcConfig& operator=(const FareCalcConfig&);
};
}

