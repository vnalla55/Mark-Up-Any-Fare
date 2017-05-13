//----------------------------------------------------------------------------
//       � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CarrierPreference
{
public:
  CarrierPreference()
    : _memoNo(0),
      _freebaggageexempt(' '),
      _availabilityApplyrul2st(' '),
      _availabilityApplyrul3st(' '),
      _bypassosc(' '),
      _bypassrsc(' '),
      _applysamenuctort(' '),
      _applyrtevaltoterminalpt(' '),
      _noApplydrvexceptus(' '),
      _applyleastRestrStOptopu(' '),
      _applyleastRestrtrnsftopu(' '),
      _noApplycombtag1and3(' '),
      _applysingleaddonconstr(' '),
      _applyspecoveraddon(' '),
      _noApplynigeriaCuradj(' '),
      _noSurfaceAtFareBreak(' '),
      _flowMktJourneyType(' '),
      _localMktJourneyType(' '),
      _activateSoloPricing(' '),
      _activateSoloShopping(' '),
      _activateJourneyPricing(' '),
      _activateJourneyShopping(' '),
      _applyUS2TaxOnFreeTkt(' '),
      _privateFareInd(' '),
      _freeBaggageCarrierExempt(' '),
      _applyPremBusCabinDiffCalc(' '),
      _applyPremEconCabinDiffCalc(' '),
      _noApplySlideToNonPremium(' '),
      _noApplyBagExceptOnCombArea1_3(' '),
      _ovrideFreeBagFareCompLogic(' '),
      _applyNormalFareOJInDiffCntrys(' '),
      _applySingleTOJBetwAreasShorterFC(' '),
      _applySingleTOJBetwAreasLongerFC(' '),
      _applySpclDOJEurope(' '),
      _applyHigherRTOJ(' '),
      _applyHigherRT(' '),
      _applyFBCinFC(' '),
      _applyBrandedFaresPerFc(' ')
  {
  }

  virtual ~CarrierPreference() {}

  struct CombPref
  {

    CombPref() : _publicPrivate1(' '), _publicPrivate2(' ') {}

    virtual bool operator==(const CombPref& rhs) const
    {
      return ((_publicPrivate1 == rhs._publicPrivate1) && (_vendor1 == rhs._vendor1) &&
              (_publicPrivate2 == rhs._publicPrivate2) && (_vendor2 == rhs._vendor2));
    }

    static void dummyData(CombPref& obj)
    {
      obj._publicPrivate1 = 'A';
      obj._vendor1 = "BCDE";
      obj._publicPrivate2 = 'F';
      obj._vendor2 = "GHIJ";
    }

  private:
    Indicator _publicPrivate1;
    VendorCode _vendor1;
    Indicator _publicPrivate2;
    VendorCode _vendor2;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _publicPrivate1);
      FLATTENIZE(archive, _vendor1);
      FLATTENIZE(archive, _publicPrivate2);
      FLATTENIZE(archive, _vendor2);
    }

    // so called accessors i.e. complete waste of time

    virtual ~CombPref() {}

    Indicator& publicPrivate1() { return _publicPrivate1; }
    const Indicator& publicPrivate1() const { return _publicPrivate1; }

    VendorCode& vendor1() { return _vendor1; }
    const VendorCode& vendor1() const { return _vendor1; }

    Indicator& publicPrivate2() { return _publicPrivate2; }
    const Indicator& publicPrivate2() const { return _publicPrivate2; }

    VendorCode& vendor2() { return _vendor2; }
    const VendorCode& vendor2() const { return _vendor2; }
  };

  virtual bool operator==(const CarrierPreference& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_firstTvlDate == rhs._firstTvlDate) &&
            (_lastTvlDate == rhs._lastTvlDate) && (_memoNo == rhs._memoNo) &&
            (_freebaggageexempt == rhs._freebaggageexempt) &&
            (_availabilityApplyrul2st == rhs._availabilityApplyrul2st) &&
            (_availabilityApplyrul3st == rhs._availabilityApplyrul3st) &&
            (_bypassosc == rhs._bypassosc) && (_bypassrsc == rhs._bypassrsc) &&
            (_applysamenuctort == rhs._applysamenuctort) &&
            (_applyrtevaltoterminalpt == rhs._applyrtevaltoterminalpt) &&
            (_noApplydrvexceptus == rhs._noApplydrvexceptus) &&
            (_applyleastRestrStOptopu == rhs._applyleastRestrStOptopu) &&
            (_applyleastRestrtrnsftopu == rhs._applyleastRestrtrnsftopu) &&
            (_noApplycombtag1and3 == rhs._noApplycombtag1and3) &&
            (_applysingleaddonconstr == rhs._applysingleaddonconstr) &&
            (_applyspecoveraddon == rhs._applyspecoveraddon) &&
            (_noApplynigeriaCuradj == rhs._noApplynigeriaCuradj) &&
            (_noSurfaceAtFareBreak == rhs._noSurfaceAtFareBreak) &&
            (_carrierbasenation == rhs._carrierbasenation) && (_description == rhs._description) &&
            (_flowMktJourneyType == rhs._flowMktJourneyType) &&
            (_localMktJourneyType == rhs._localMktJourneyType) &&
            (_activateSoloPricing == rhs._activateSoloPricing) &&
            (_activateSoloShopping == rhs._activateSoloShopping) &&
            (_activateJourneyPricing == rhs._activateJourneyPricing) &&
            (_activateJourneyShopping == rhs._activateJourneyShopping) &&
            (_applyUS2TaxOnFreeTkt == rhs._applyUS2TaxOnFreeTkt) &&
            (_combPrefs == rhs._combPrefs) && (_fbrPrefs == rhs._fbrPrefs) &&
            (_privateFareInd == rhs._privateFareInd) &&
            (_freeBaggageCarrierExempt == rhs._freeBaggageCarrierExempt) &&
            (_applyPremBusCabinDiffCalc == rhs._applyPremBusCabinDiffCalc) &&
            (_applyPremEconCabinDiffCalc == rhs._applyPremEconCabinDiffCalc) &&
            (_noApplySlideToNonPremium == rhs._noApplySlideToNonPremium) &&
            (_noApplyBagExceptOnCombArea1_3 == rhs._noApplyBagExceptOnCombArea1_3) &&
            (_ovrideFreeBagFareCompLogic == rhs._ovrideFreeBagFareCompLogic) &&
            (_applyNormalFareOJInDiffCntrys == rhs._applyNormalFareOJInDiffCntrys) &&
            (_applySingleTOJBetwAreasShorterFC == rhs._applySingleTOJBetwAreasShorterFC) &&
            (_applySingleTOJBetwAreasLongerFC == rhs._applySingleTOJBetwAreasLongerFC) &&
            (_applySpclDOJEurope == rhs._applySpclDOJEurope) &&
            (_applyHigherRTOJ == rhs._applyHigherRTOJ) && (_applyHigherRT == rhs._applyHigherRT) &&
            (_applyFBCinFC == rhs._applyFBCinFC) &&
            (_applyBrandedFaresPerFc == rhs._applyBrandedFaresPerFc) &&
            (_nonRefundableYQCode == rhs._nonRefundableYQCode) &&
            (_nonRefundableYRCode == rhs._nonRefundableYRCode));
  }

  static void dummyData(CarrierPreference& obj)
  {
    obj._carrier = "ABC";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._firstTvlDate = time(nullptr);
    obj._lastTvlDate = time(nullptr);
    obj._memoNo = 1;
    obj._freebaggageexempt = 'D';
    obj._availabilityApplyrul2st = 'E';
    obj._availabilityApplyrul3st = 'F';
    obj._bypassosc = 'G';
    obj._bypassrsc = 'H';
    obj._applysamenuctort = 'I';
    obj._applyrtevaltoterminalpt = 'J';
    obj._noApplydrvexceptus = 'K';
    obj._applyleastRestrStOptopu = 'L';
    obj._applyleastRestrtrnsftopu = 'M';
    obj._noApplycombtag1and3 = 'N';
    obj._applysingleaddonconstr = 'O';
    obj._applyspecoveraddon = 'P';
    obj._noApplynigeriaCuradj = 'Q';
    obj._noSurfaceAtFareBreak = 'R';
    obj._carrierbasenation = "STU";
    obj._description = "Description";
    obj._flowMktJourneyType = 'V';
    obj._localMktJourneyType = 'W';
    obj._activateSoloPricing = 'X';
    obj._activateSoloShopping = 'Y';
    obj._activateJourneyPricing = 'Z';
    obj._activateJourneyShopping = 'a';
    obj._applyUS2TaxOnFreeTkt = 'b';
    obj._applyPremBusCabinDiffCalc = 'c';
    obj._applyPremEconCabinDiffCalc = 'd';
    obj._noApplySlideToNonPremium = 'e';
    obj._noApplyBagExceptOnCombArea1_3 = 'f';
    obj._ovrideFreeBagFareCompLogic = 'g';
    obj._applyNormalFareOJInDiffCntrys = 'Y';
    obj._applySingleTOJBetwAreasShorterFC = 'Y';
    obj._applySingleTOJBetwAreasLongerFC = 'Y';
    obj._applySpclDOJEurope = 'Y';
    obj._applyHigherRTOJ = 'Y';
    obj._applyHigherRT = 'Y';
    obj._applyFBCinFC = 'N';
    obj._applyBrandedFaresPerFc = 'N';
    obj._nonRefundableYQCode = "YQI";
    obj._nonRefundableYRCode = "YRF";

    CombPref cb1;
    CombPref cb2;
    CombPref::dummyData(cb1);
    CombPref::dummyData(cb2);
    obj._combPrefs.push_back(cb1);
    obj._combPrefs.push_back(cb2);

    obj._fbrPrefs.push_back("efgh");
    obj._fbrPrefs.push_back("ijkl");

    obj._privateFareInd = 'c';
    obj._freeBaggageCarrierExempt = 'd';
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& firstTvlDate() { return _firstTvlDate; }
  const DateTime& firstTvlDate() const { return _firstTvlDate; }

  DateTime& lastTvlDate() { return _lastTvlDate; }
  const DateTime& lastTvlDate() const { return _lastTvlDate; }

  int& memoNo() { return _memoNo; }
  const int& memoNo() const { return _memoNo; }

  Indicator& freebaggageexempt() { return _freebaggageexempt; }
  const Indicator& freebaggageexempt() const { return _freebaggageexempt; }

  Indicator& availabilityApplyrul2st() { return _availabilityApplyrul2st; }
  const Indicator& availabilityApplyrul2st() const { return _availabilityApplyrul2st; }

  Indicator& availabilityApplyrul3st() { return _availabilityApplyrul3st; }
  const Indicator& availabilityApplyrul3st() const { return _availabilityApplyrul3st; }

  Indicator& bypassosc() { return _bypassosc; }
  const Indicator& bypassosc() const { return _bypassosc; }

  Indicator& bypassrsc() { return _bypassrsc; }
  const Indicator& bypassrsc() const { return _bypassrsc; }

  Indicator& applysamenuctort() { return _applysamenuctort; }
  const Indicator& applysamenuctort() const { return _applysamenuctort; }

  Indicator& applyrtevaltoterminalpt() { return _applyrtevaltoterminalpt; }
  const Indicator& applyrtevaltoterminalpt() const { return _applyrtevaltoterminalpt; }

  Indicator& noApplydrvexceptus() { return _noApplydrvexceptus; }
  const Indicator& noApplydrvexceptus() const { return _noApplydrvexceptus; }

  Indicator& applyleastRestrStOptopu() { return _applyleastRestrStOptopu; }
  const Indicator& applyleastRestrStOptopu() const { return _applyleastRestrStOptopu; }

  Indicator& applyleastRestrtrnsftopu() { return _applyleastRestrtrnsftopu; }
  const Indicator& applyleastRestrtrnsftopu() const { return _applyleastRestrtrnsftopu; }

  Indicator& noApplycombtag1and3() { return _noApplycombtag1and3; }
  const Indicator& noApplycombtag1and3() const { return _noApplycombtag1and3; }

  Indicator& applysingleaddonconstr() { return _applysingleaddonconstr; }
  const Indicator& applysingleaddonconstr() const { return _applysingleaddonconstr; }

  Indicator& applyspecoveraddon() { return _applyspecoveraddon; }
  const Indicator& applyspecoveraddon() const { return _applyspecoveraddon; }

  Indicator& noApplynigeriaCuradj() { return _noApplynigeriaCuradj; }
  const Indicator& noApplynigeriaCuradj() const { return _noApplynigeriaCuradj; }

  Indicator& noSurfaceAtFareBreak() { return _noSurfaceAtFareBreak; }
  const Indicator& noSurfaceAtFareBreak() const { return _noSurfaceAtFareBreak; }

  CarrierCode& carrierbasenation() { return _carrierbasenation; }
  const CarrierCode& carrierbasenation() const { return _carrierbasenation; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  Indicator& flowMktJourneyType() { return _flowMktJourneyType; }
  const Indicator& flowMktJourneyType() const { return _flowMktJourneyType; }

  Indicator& localMktJourneyType() { return _localMktJourneyType; }
  const Indicator& localMktJourneyType() const { return _localMktJourneyType; }

  Indicator& activateSoloPricing() { return _activateSoloPricing; }
  const Indicator& activateSoloPricing() const { return _activateSoloPricing; }

  Indicator& activateSoloShopping() { return _activateSoloShopping; }
  const Indicator& activateSoloShopping() const { return _activateSoloShopping; }

  Indicator& activateJourneyPricing() { return _activateJourneyPricing; }
  const Indicator& activateJourneyPricing() const { return _activateJourneyPricing; }

  Indicator& activateJourneyShopping() { return _activateJourneyShopping; }
  const Indicator& activateJourneyShopping() const { return _activateJourneyShopping; }

  Indicator& applyUS2TaxOnFreeTkt() { return _applyUS2TaxOnFreeTkt; }
  const Indicator& applyUS2TaxOnFreeTkt() const { return _applyUS2TaxOnFreeTkt; }

  std::vector<CombPref>& combPrefs() { return _combPrefs; }
  const std::vector<CombPref>& combPrefs() const { return _combPrefs; }

  std::vector<VendorCode>& fbrPrefs() { return _fbrPrefs; }
  const std::vector<VendorCode>& fbrPrefs() const { return _fbrPrefs; }

  Indicator& privateFareInd() { return _privateFareInd; }
  const Indicator& privateFareInd() const { return _privateFareInd; }

  Indicator& freeBaggageCarrierExempt() { return _freeBaggageCarrierExempt; }
  const Indicator& freeBaggageCarrierExempt() const { return _freeBaggageCarrierExempt; }

  Indicator& applyPremBusCabinDiffCalc() { return _applyPremBusCabinDiffCalc; }
  const Indicator& applyPremBusCabinDiffCalc() const { return _applyPremBusCabinDiffCalc; }

  Indicator& applyPremEconCabinDiffCalc() { return _applyPremEconCabinDiffCalc; }
  const Indicator& applyPremEconCabinDiffCalc() const { return _applyPremEconCabinDiffCalc; }

  Indicator& noApplySlideToNonPremium() { return _noApplySlideToNonPremium; }
  const Indicator& noApplySlideToNonPremium() const { return _noApplySlideToNonPremium; }

  Indicator& noApplyBagExceptOnCombArea1_3() { return _noApplyBagExceptOnCombArea1_3; }
  const Indicator& noApplyBagExceptOnCombArea1_3() const { return _noApplyBagExceptOnCombArea1_3; }

  Indicator& ovrideFreeBagFareCompLogic() { return _ovrideFreeBagFareCompLogic; }
  const Indicator& ovrideFreeBagFareCompLogic() const { return _ovrideFreeBagFareCompLogic; }

  Indicator& applyNormalFareOJInDiffCntrys() { return _applyNormalFareOJInDiffCntrys; }
  const Indicator& applyNormalFareOJInDiffCntrys() const { return _applyNormalFareOJInDiffCntrys; }

  Indicator& applySingleTOJBetwAreasShorterFC() { return _applySingleTOJBetwAreasShorterFC; }
  const Indicator& applySingleTOJBetwAreasShorterFC() const
  {
    return _applySingleTOJBetwAreasShorterFC;
  }

  Indicator& applySingleTOJBetwAreasLongerFC() { return _applySingleTOJBetwAreasLongerFC; }
  const Indicator& applySingleTOJBetwAreasLongerFC() const
  {
    return _applySingleTOJBetwAreasLongerFC;
  }

  Indicator& applySpclDOJEurope() { return _applySpclDOJEurope; }
  const Indicator& applySpclDOJEurope() const { return _applySpclDOJEurope; }

  Indicator& applyHigherRTOJ() { return _applyHigherRTOJ; }
  const Indicator& applyHigherRTOJ() const { return _applyHigherRTOJ; }

  Indicator& applyHigherRT() { return _applyHigherRT; }
  const Indicator& applyHigherRT() const { return _applyHigherRT; }

  Indicator& applyFBCinFC() { return _applyFBCinFC; }
  const Indicator& applyFBCinFC() const { return _applyFBCinFC; }

  void setApplyBrandedFaresPerFc(Indicator applyBrandedFaresPerFc)
  {
    _applyBrandedFaresPerFc = applyBrandedFaresPerFc;
  }
  Indicator getApplyBrandedFaresPerFc() const
  {
    return _applyBrandedFaresPerFc;
  }

  void setNonRefundableYQCode(const TaxCode& nonRefundableYQCode)
  {
    _nonRefundableYQCode = nonRefundableYQCode;
  }
  const TaxCode& getNonRefundableYQCode() const { return _nonRefundableYQCode; }

  void setNonRefundableYRCode(const TaxCode& nonRefundableYRCode)
  {
    _nonRefundableYRCode = nonRefundableYRCode;
  }
  const TaxCode& getNonRefundableYRCode() const { return _nonRefundableYRCode; }

private:
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTvlDate;
  DateTime _lastTvlDate;
  int _memoNo;
  Indicator _freebaggageexempt;
  Indicator _availabilityApplyrul2st;
  Indicator _availabilityApplyrul3st;
  Indicator _bypassosc;
  Indicator _bypassrsc;
  Indicator _applysamenuctort;
  Indicator _applyrtevaltoterminalpt;
  Indicator _noApplydrvexceptus;
  Indicator _applyleastRestrStOptopu;
  Indicator _applyleastRestrtrnsftopu;
  Indicator _noApplycombtag1and3;
  Indicator _applysingleaddonconstr;
  Indicator _applyspecoveraddon;
  Indicator _noApplynigeriaCuradj;
  Indicator _noSurfaceAtFareBreak;
  CarrierCode _carrierbasenation;
  std::string _description;
  Indicator _flowMktJourneyType;
  Indicator _localMktJourneyType;
  Indicator _activateSoloPricing;
  Indicator _activateSoloShopping;
  Indicator _activateJourneyPricing;
  Indicator _activateJourneyShopping;
  Indicator _applyUS2TaxOnFreeTkt;
  std::vector<CombPref> _combPrefs;
  std::vector<VendorCode> _fbrPrefs;
  Indicator _privateFareInd;
  Indicator _freeBaggageCarrierExempt;
  Indicator _applyPremBusCabinDiffCalc;
  Indicator _applyPremEconCabinDiffCalc;
  Indicator _noApplySlideToNonPremium;
  Indicator _noApplyBagExceptOnCombArea1_3;
  Indicator _ovrideFreeBagFareCompLogic;
  Indicator _applyNormalFareOJInDiffCntrys;
  Indicator _applySingleTOJBetwAreasShorterFC;
  Indicator _applySingleTOJBetwAreasLongerFC;
  Indicator _applySpclDOJEurope;
  Indicator _applyHigherRTOJ;
  Indicator _applyHigherRT;
  Indicator _applyFBCinFC;
  Indicator _applyBrandedFaresPerFc;
  TaxCode _nonRefundableYQCode;
  TaxCode _nonRefundableYRCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _firstTvlDate);
    FLATTENIZE(archive, _lastTvlDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _freebaggageexempt);
    FLATTENIZE(archive, _availabilityApplyrul2st);
    FLATTENIZE(archive, _availabilityApplyrul3st);
    FLATTENIZE(archive, _bypassosc);
    FLATTENIZE(archive, _bypassrsc);
    FLATTENIZE(archive, _applysamenuctort);
    FLATTENIZE(archive, _applyrtevaltoterminalpt);
    FLATTENIZE(archive, _noApplydrvexceptus);
    FLATTENIZE(archive, _applyleastRestrStOptopu);
    FLATTENIZE(archive, _applyleastRestrtrnsftopu);
    FLATTENIZE(archive, _noApplycombtag1and3);
    FLATTENIZE(archive, _applysingleaddonconstr);
    FLATTENIZE(archive, _applyspecoveraddon);
    FLATTENIZE(archive, _noApplynigeriaCuradj);
    FLATTENIZE(archive, _noSurfaceAtFareBreak);
    FLATTENIZE(archive, _carrierbasenation);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _flowMktJourneyType);
    FLATTENIZE(archive, _localMktJourneyType);
    FLATTENIZE(archive, _activateSoloPricing);
    FLATTENIZE(archive, _activateSoloShopping);
    FLATTENIZE(archive, _activateJourneyPricing);
    FLATTENIZE(archive, _activateJourneyShopping);
    FLATTENIZE(archive, _applyUS2TaxOnFreeTkt);
    FLATTENIZE(archive, _combPrefs);
    FLATTENIZE(archive, _fbrPrefs);
    FLATTENIZE(archive, _privateFareInd);
    FLATTENIZE(archive, _freeBaggageCarrierExempt);
    FLATTENIZE(archive, _applyPremBusCabinDiffCalc);
    FLATTENIZE(archive, _applyPremEconCabinDiffCalc);
    FLATTENIZE(archive, _noApplySlideToNonPremium);
    FLATTENIZE(archive, _noApplyBagExceptOnCombArea1_3);
    FLATTENIZE(archive, _ovrideFreeBagFareCompLogic);
    FLATTENIZE(archive, _applyNormalFareOJInDiffCntrys);
    FLATTENIZE(archive, _applySingleTOJBetwAreasShorterFC);
    FLATTENIZE(archive, _applySingleTOJBetwAreasLongerFC);
    FLATTENIZE(archive, _applySpclDOJEurope);
    FLATTENIZE(archive, _applyHigherRTOJ);
    FLATTENIZE(archive, _applyHigherRT);
    FLATTENIZE(archive, _applyFBCinFC);
    FLATTENIZE(archive, _applyBrandedFaresPerFc);
    FLATTENIZE(archive, _nonRefundableYQCode);
    FLATTENIZE(archive, _nonRefundableYRCode);
  }

private:
};
}
