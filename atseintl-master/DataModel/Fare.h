//-------------------------------------------------------------------
//
//  File:        Fare.h
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: A generic class to represent common part of fare.
//               It includes FareInfo, TariffCrossRef and status
//               fields and accessors.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/SmallBitSet.h"
#include "Common/TseConsts.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/SITAConstructedFareInfo.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"

#include <vector>

namespace tse
{

class FareMarket;

class WarningMap
{
public:
  enum WarningID
  {
    cat2_warning_1 = 0,
    cat2_warning_2,
    cat4_warning,
    cat5_warning_1,
    cat5_warning_2,
    cat6_warning,
    cat7_warning,
    cat11_warning,
    cat12_warning,
    cat13_warning,
    cat14_warning,
    cat15_warning_1,
    cat15_warning_2,
    cat19_22_warning,
    map_size
  };

  WarningMap();

  bool isSet(const WarningID warning) const;
  void set(const WarningID warning, bool value = true) const;

  bool isCat5WqWarning(const int seg) const;
  void setCat5WqWarning(const int seg, bool value = true) const;

  WarningMap& operator=(const WarningMap& copy);

private:
  mutable SmallBitSet<uint16_t, uint16_t> _warningMap;
  mutable SmallBitSet<uint16_t, uint16_t> _cat5Warnings;
};

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

class Fare
{
public:
  // public types
  // ====== =====
  enum FareState
  {
    FS_Domestic = 0x00000001,
    FS_Transborder = 0x00000002,
    FS_International = 0x00000004,
    FS_ForeignDomestic = 0x00000008,
    FS_ScopeIsDefined = 0x0000000F,
    FS_PublishedFare = 0x00000010,
    FS_ConstructedFare = 0x00000020,
    FS_FBRBaseFare = 0x00000040,
    FS_IndustryFare = 0x00000080,
    FS_RoutingProcessed = 0x00000100,
    FS_IsRouting = 0x00000200,
    FS_RoutingValid = 0x00000400,
    FS_ReversedDirection = 0x00000800,
    FS_Cat15SecurityValid = 0x00001000, // 0 == pass
    FS_FareNotValidForDisplay = 0x00002000, // 0 == pass
    FS_ElectronicTktRequired = 0x00004000, // rule status bit
    FS_PaperTktSurchargeMayApply = 0x00008000, // rule status bit
    FS_PaperTktSurchargeIncluded = 0x00010000, // rule status bit
    FS_InvalidGlobalDirection = 0x00020000, // 0 == valid
    FS_InvalidCalcCurrForDomItin = 0x00040000, // 0 == valid
    FS_PsrHipExempt = 0x00080000,
    FS_InvalidBySecondaryActionCode = 0x00100000, // 0 == valid
    FS_WebFare = 0x00200000, // 1 == Web fare
    FS_FareValidForDisplayOnly = 0x00400000, // Tariff Inhibit = 'D'
    FS_Cat15GeneralRuleProcess = 0x00800000, // 1 == yes
    FS_Cat23PublishedFail = 0x01000000, // 1 == yes(fail)
    FS_Cat15HasSecurity = 0x02000000, // 1 == yes
    FS_ElectronicTktable = 0x04000000, // 0 == yes, 1 == no
    FS_RoutingMapValid = 0x08000000,
    FS_DirectionalityFail = 0x10000000,
    FS_FootnotesPrevalidated = 0x20000000
  };

  typedef SmallBitSet<uint32_t, FareState> FareStatus;

  enum BrandedFareState
  {
    FS_InvBrand = 0x01,
    FS_InvBrandCorpID = 0x02,
  };

  typedef SmallBitSet<uint8_t, BrandedFareState> BrandedFareStatus;

  enum FopState : uint8_t
  { FOP_CASH = 0x01,
    FOP_CHECK = 0x02,
    FOP_CREDIT = 0x04,
    FOP_GTR = 0x08 };

  typedef SmallBitSet<uint8_t, FopState> FopStatus;

  Fare() = default;
  Fare(const Fare&) = delete;
  Fare& operator=(const Fare&) = delete;

  virtual ~Fare() = default;

  /**
   * This methods obtains a new Fare pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual Fare* clone(DataHandle& dataHandle, bool resetStatus = true) const;

  /**
   * This methods populates a given Fare to be
   * 'equal' to the current object
   *
   * @param Fare - object to populate
   */
  virtual void clone(DataHandle& dataHandle, Fare& cloneObj, bool resetStatus = true) const;

  // initialization
  // ==============

  bool initialize(const FareState initialStatus,
                  const FareInfo* fareInfo,
                  const FareMarket& fareMarket,
                  const TariffCrossRefInfo* tariffCrossRefInfo = nullptr,
                  const ConstructedFareInfo* constructedFareInfo = nullptr);
  // fare status
  // ==== ======

  bool isValid() const;
  bool isValidForFareDisplay() const;
  bool isValidForRouting() const;

  bool isDomestic() const { return _status.isSet(FS_Domestic); }
  bool isTransborder() const { return _status.isSet(FS_Transborder); }
  bool isInternational() const { return _status.isSet(FS_International); }
  bool isForeignDomestic() const { return _status.isSet(FS_ForeignDomestic); }

  bool isPublished() const { return _status.isSet(FS_PublishedFare); }
  bool isConstructed() const { return _status.isSet(FS_ConstructedFare); }
  bool isIndustry() const { return _status.isSet(FS_IndustryFare); }

  bool isReversed() const { return _status.isSet(FS_ReversedDirection); }

  bool isGlobalDirectionValid() const { return !_status.isSet(FS_InvalidGlobalDirection); }

  bool setGlobalDirectionValid(bool isValid = true)
  {
    _status.set(FS_InvalidGlobalDirection, !isValid);
    return isValid;
  }

  bool isFareNotValidForDisplay() const { return _status.isSet(FS_FareNotValidForDisplay); }

  bool setFareNotValidForDisplay(bool isNotValid = true)
  {
    _status.set(FS_FareNotValidForDisplay, isNotValid);
    return isNotValid;
  }

  bool isFareValidForDisplayOnly() const { return _status.isSet(FS_FareValidForDisplayOnly); }

  bool setFareValidForDisplayOnly(bool forDisplayOnly = true)
  {
    _status.set(FS_FareValidForDisplayOnly, forDisplayOnly);
    return forDisplayOnly;
  }

  bool isCalcCurrForDomItinValid() const { return !_status.isSet(FS_InvalidCalcCurrForDomItin); }

  bool setCalcCurrForDomItinValid(bool isValid = true)
  {
    _status.set(FS_InvalidCalcCurrForDomItin, !isValid);
    return isValid;
  }

  bool isPsrHipExempt() const { return _status.isSet(FS_PsrHipExempt); }

  bool setPsrHipExempt(bool psrHipExempt = true)
  {
    return _status.set(FS_PsrHipExempt, psrHipExempt);
  }

  bool isValidBySecondaryActionCode() const
  {
    return !_status.isSet(FS_InvalidBySecondaryActionCode);
  }

  bool setFailBySecondaryActionCode() { return _status.set(FS_InvalidBySecondaryActionCode); }

  bool isWebFare() const { return _status.isSet(FS_WebFare); }

  bool setWebFare(bool webFare = true) { return _status.set(FS_WebFare, webFare); }

  bool isCat15GeneralRuleProcess() const { return _status.isSet(FS_Cat15GeneralRuleProcess); }

  bool setCat15GeneralRuleProcess(bool cat15GeneralRuleProcess = true)
  {
    return _status.set(FS_Cat15GeneralRuleProcess, cat15GeneralRuleProcess);
  }

  bool isCat23PublishedFail() const { return _status.isSet(FS_Cat23PublishedFail); }

  bool setCat23PublishedFail(bool cat23PublishedFail = true)
  {
    return _status.set(FS_Cat23PublishedFail, cat23PublishedFail);
  }

  bool cat15HasSecurity() const { return _status.isSet(FS_Cat15HasSecurity); }

  bool setCat15HasSecurity(bool cat15HasSecurity = true)
  {
    return _status.set(FS_Cat15HasSecurity, cat15HasSecurity);
  }

  bool isDirectionalityFail() const { return _status.isSet(FS_DirectionalityFail); }

  bool setDirectionalityFail(bool status = true)
  {
    return _status.set(FS_DirectionalityFail, status);
  }

  bool areFootnotesPrevalidated() const { return _status.isSet(FS_FootnotesPrevalidated); }

  bool setFootnotesPrevalidated(bool status = true)
  {
    return _status.set(FS_FootnotesPrevalidated, status);
  }

  FareStatus& status() { return _status; }
  const FareStatus& status() const { return _status; }

  bool isOutboundFareForCarnivalInbound() const { return _outboundFareForCarnivalInbound; }

  void setOutboundFareForCarnivalInbound(bool offci) { _outboundFareForCarnivalInbound = offci; }

  // branded fare status
  // ==== ======

  bool isInvBrand(const uint16_t index) const
  {
    return getBrandedFareStatus(index).isSet(FS_InvBrand);
  }

  bool setInvBrand(const uint16_t index, bool value = true)
  {
    return _brandedFareStatus[index].set(FS_InvBrand, value);
  }

  bool isInvBrandCorpID(const uint16_t index) const
  {
    return getBrandedFareStatus(index).isSet(FS_InvBrandCorpID);
  }

  bool setInvBrandCorpID(const uint16_t index, bool value = true)
  {
    return _brandedFareStatus[index].set(FS_InvBrandCorpID, value);
  }

  std::map<uint16_t, BrandedFareStatus>& brandedFareStatus() { return _brandedFareStatus; }
  const std::map<uint16_t, BrandedFareStatus>& brandedFareStatus() const
  {
    return _brandedFareStatus;
  }

  const BrandedFareStatus& getBrandedFareStatus(const uint16_t index) const;
  bool isAnyBrandedFareValid() const;
  void getBrandIndexes(std::set<uint16_t> &result) const;

  // rule status
  // ==== ======

  bool isCategoryValid(const unsigned int category) const
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? true : !_ruleStatus.isSet(ruleBit);
  }

  bool setCategoryValid(const unsigned int category, const bool catIsValid = true)
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? false : !_ruleStatus.set(ruleBit, !catIsValid);
  }

  bool areAllCategoryValid() const { return !_ruleStatus.isSet(RS_AllCat); }

  bool isCategoryProcessed(const unsigned int category) const
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? true : _ruleProcessStatus.isSet(ruleBit);
  }

  bool setCategoryProcessed(const unsigned int category, const bool catIsProcessed = true)
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? false : _ruleProcessStatus.set(ruleBit, catIsProcessed);
  }

  bool isAllDiffCatProcessed() const { return _ruleProcessStatus.isAllSet(RS_AllDiffCat); }

  bool isCategorySoftPassed(const unsigned int category) const
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? true : _ruleSoftPassStatus.isSet(ruleBit);
  }

  bool setCategorySoftPassed(const unsigned int category, const bool catSoftPassed = true)
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? false : _ruleSoftPassStatus.set(ruleBit, catSoftPassed);
  }

  bool checkFareRuleAltGenRule(const unsigned int category) const
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? true : !_dontCheckFareRuleAltGenRule.isSet(ruleBit);
  }

  void setCheckFareRuleAltGenRule(const unsigned int category, const bool check = true)
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    if (LIKELY(ruleBit != RS_CatNotSupported))
      _dontCheckFareRuleAltGenRule.set(ruleBit, !check);
  }

  bool footnoteRec2Status(const unsigned int category) const
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    return ruleBit == RS_CatNotSupported ? false : _footnoteRec2Status.isSet(ruleBit);
  }

  void setFootnoteRec2Status(const unsigned int category, const bool status = true)
  {
    const RuleState ruleBit = mapRuleStateBit(category);
    if (LIKELY(ruleBit != RS_CatNotSupported))
      _footnoteRec2Status.set(ruleBit, status);
  }

  bool isSoftPassed() const { return _ruleSoftPassStatus.isSet(RS_AllCat); }

  void resetRuleStatus();

  bool isCat15SecurityValid() const { return !_status.isSet(FS_Cat15SecurityValid); }

  bool setCat15SecurityValid(const bool isValid = true)
  {
    return !_status.set(FS_Cat15SecurityValid, !isValid);
  }

  bool isElectronicTktable() const { return !_status.isSet(FS_ElectronicTktable); }

  bool setElectronicTktable(const bool isEticketable = false)
  {
    return _status.set(FS_ElectronicTktable, isEticketable);
  }

  bool isElectronicTktRequired() const { return _status.isSet(FS_ElectronicTktRequired); }

  bool setElectronicTktRequired(const bool isRequired = false)
  {
    return _status.set(FS_ElectronicTktRequired, isRequired);
  }

  bool isPaperTktSurchargeMayApply() const { return _status.isSet(FS_PaperTktSurchargeMayApply); }

  bool setPaperTktSurchargeMayApply(const bool mayApply = false)
  {
    return _status.set(FS_PaperTktSurchargeMayApply, mayApply);
  }

  bool isPaperTktSurchargeIncluded() const { return _status.isSet(FS_PaperTktSurchargeIncluded); }

  bool setPaperTktSurchargeIncluded(const bool isIncluded = false)
  {
    return _status.set(FS_PaperTktSurchargeIncluded, isIncluded);
  }

  void resetFnMissingStat(const uint16_t& numFootnotes);

  void setFoundFootnote(uint16_t fnIndex);

  bool missedFootnote(uint16_t& missingFnIndex) const;

  bool setMissingFootnote(const bool isMissing = true)
  {
    return _footNoteStatus.set(MissingFootNote, isMissing);
  }

  bool isMissingFootnote() const { return _footNoteStatus.isSet(MissingFootNote); }

  void setNotMissingAnyFootnote() { _footNoteStatus.initialize(InitializedFnStat); }

  uint8_t getMissFootnoteStat() { return (_footNoteStatus.value() & MissingAnyFootNote); }

  // routing status
  // ======= ======

  bool isRoutingMapValid() const { return _status.isSet(FS_RoutingMapValid); }
  bool setRoutingMapValid(const bool rtMapValid = false)
  {
    return _status.set(FS_RoutingMapValid, rtMapValid);
  }

  bool isRoutingProcessed() const { return _status.isSet(FS_RoutingProcessed); }
  bool setRoutingProcessed(const bool rtProcessed = false)
  {
    return _status.set(FS_RoutingProcessed, rtProcessed);
  }

  bool isRouting() const { return _status.isSet(FS_IsRouting); }
  bool setIsRouting(const bool isRouting = true) { return _status.set(FS_IsRouting, isRouting); }

  bool isRoutingValid() const { return _status.isSet(FS_RoutingValid); }
  bool setRoutingValid(const bool rtValid = true) { return _status.set(FS_RoutingValid, rtValid); }

  const uint16_t mileageSurchargePctg() const { return _mileageSurchargePctg; }
  uint16_t& mileageSurchargePctg() { return _mileageSurchargePctg; }
  const MoneyAmount mileageSurchargeAmt() const { return _mileageSurchargeAmt; }
  MoneyAmount& mileageSurchargeAmt() { return _mileageSurchargeAmt; }

  const MoneyAmount rexSecondMileageSurchargeAmt() const { return _rexSecondMileageSurchargeAmt; }
  MoneyAmount& rexSecondMileageSurchargeAmt() { return _rexSecondMileageSurchargeAmt; }

  // fare parts check
  // ==== ===== =====

  bool isFareInfoMissing() const { return (_fareInfo == nullptr); }

  bool isTariffCrossRefMissing() const { return (_tariffCrossRefInfo == nullptr); }

  bool isConstructedFareInfoMissing() const { return (_constructedFareInfo == nullptr); }

  // access to fare parts
  // ====== == ==== =====

  static const Fare* emptyFare()
  {
    return &_emptyFare;
  };

  void setFareInfo(const FareInfo* fareInfo) { _fareInfo = fareInfo; }
  const FareInfo* fareInfo() const { return _fareInfo; }

  const TariffCrossRefInfo* tariffCrossRefInfo() const { return _tariffCrossRefInfo; }
  void setTariffCrossRefInfo(const TariffCrossRefInfo* tariffCrossRefInfo)
  {
    _tariffCrossRefInfo = tariffCrossRefInfo;
  }

  const ConstructedFareInfo*& constructedFareInfo() { return _constructedFareInfo; }
  const ConstructedFareInfo* constructedFareInfo() const { return _constructedFareInfo; }

  // Fare amount
  // ==== ======

  const CurrencyNoDec numDecimal() const { return _fareInfo->noDec(); }
  const CurrencyCode& currency() const { return _fareInfo->currency(); }

  const MoneyAmount originalFareAmount() const { return _fareInfo->originalFareAmount(); }

  // One way fare amount
  // === === ==== ======

  const MoneyAmount fareAmount() const { return _fareInfo->fareAmount(); }

  // one way fare amount converted to NUCs
  // === === ==== ====== ========= == ====

  MoneyAmount& nucFareAmount() { return _nucFareAmount; }
  const MoneyAmount nucFareAmount() const { return _nucFareAmount; }

  MoneyAmount& nucMarkupAmount() { return _nucMarkupAmount; }
  const MoneyAmount nucMarkupAmount() const { return _nucMarkupAmount; }

  MoneyAmount& rexSecondNucFareAmount() { return _rexSecondNucFareAmount; }
  const MoneyAmount rexSecondNucFareAmount() const { return _rexSecondNucFareAmount; }

  // round trip fare amount converted to NUCs
  // ===== ==== ==== ====== ========= == ====

  MoneyAmount& nucOriginalFareAmount() { return _nucOriginalFareAmount; }
  const MoneyAmount nucOriginalFareAmount() const { return _nucOriginalFareAmount; }

  MoneyAmount& rexSecondNucOriginalFareAmount() { return _rexSecondNucOriginalFareAmount; }
  const MoneyAmount rexSecondNucOriginalFareAmount() const
  {
    return _rexSecondNucOriginalFareAmount;
  }

  // accessors to FareInfo members
  // ========= == ======== =======

  const VendorCode& vendor() const { return _fareInfo->vendor(); }
  virtual const CarrierCode& carrier() const { return _fareInfo->carrier(); }

  const LocCode& origin() const
  {
    if (_fareInfo->directionality() == BOTH)
      return market1();
    else
      return _fareInfo->directionality() == TO ? market2() : market1();
  }

  const LocCode& destination() const
  {
    if (_fareInfo->directionality() == BOTH)
      return market2();
    else
      return _fareInfo->directionality() == TO ? market1() : market2();
  }

  const LocCode& market1() const { return _fareInfo->market1(); }
  const LocCode& market2() const { return _fareInfo->market2(); }

  virtual const FareClassCode& fareClass() const { return _fareInfo->fareClass(); }

  const TariffNumber fareTariff() const { return _fareInfo->fareTariff(); }

  const DateTime& effectiveDate() const { return _fareInfo->effDate(); }
  const DateTime& expirationDate() const { return _fareInfo->expireDate(); }
  const DateTime& discDate() const { return _fareInfo->discDate(); }
  const DateTime& lastModDate() const { return _fareInfo->lastModDate(); }

  const Footnote& footNote1() const { return _fareInfo->footNote1(); }
  const Footnote& footNote2() const { return _fareInfo->footNote2(); }

  const Indicator owrt() const { return _fareInfo->owrt(); }
  bool isRoundTrip() const { return _fareInfo->isRoundTrip(); }
  const RuleNumber& ruleNumber() const { return _fareInfo->ruleNumber(); }
  const RoutingNumber& routingNumber() const { return _fareInfo->routingNumber(); }

  const bool vendorFWS() const { return _fareInfo->vendorFWS(); }

  // this accessor returns fare directionality regarding to FareMarket

  const Directionality directionality() const
  {
    if (_fareInfo->directionality() == BOTH)
      return _fareInfo->directionality();

    if (isReversed())
      return (_fareInfo->directionality() == TO) ? FROM : TO;
    else
      return _fareInfo->directionality();
  }

  const GlobalDirection globalDirection() const { return _fareInfo->globalDirection(); }

  const Indicator inhibit() const { return _fareInfo->inhibit(); }

  const Indicator increasedFareAmtTag() const { return _fareInfo->increasedFareAmtTag(); }

  const Indicator reducedFareAmtTag() const { return _fareInfo->reducedFareAmtTag(); }

  const Indicator footnoteTag() const { return _fareInfo->footnoteTag(); }

  const Indicator routingTag() const { return _fareInfo->routingTag(); }

  const Indicator mpmTag() const { return _fareInfo->mpmTag(); }

  const Indicator effectiveDateTag() const { return _fareInfo->effectiveDateTag(); }

  const Indicator currencyCodeTag() const { return _fareInfo->currencyCodeTag(); }

  const Indicator ruleTag() const { return _fareInfo->ruleTag(); }

  const SequenceNumberLong sequenceNumber() const { return _fareInfo->sequenceNumber(); }

  const LinkNumber linkNumber() const { return _fareInfo->linkNumber(); }

  const DateTime& createDate() const { return _fareInfo->createDate(); }

  // accessors to TariffCrossRefInfo members
  // ========= == ================== =======

  const TariffCode& tcrFareTariffCode() const { return _tariffCrossRefInfo->fareTariffCode(); }

  const TariffCategory tcrTariffCat() const { return _tariffCrossRefInfo->tariffCat(); }

  const TariffNumber tcrRuleTariff() const { return _tariffCrossRefInfo->ruleTariff(); }
  const TariffCode& tcrRuleTariffCode() const { return _tariffCrossRefInfo->ruleTariffCode(); }

  const TariffNumber tcrRoutingTariff1() const { return _tariffCrossRefInfo->routingTariff1(); }
  const TariffCode& tcrRoutingTariff1Code() const
  {
    return _tariffCrossRefInfo->routingTariff1Code();
  }

  const TariffNumber tcrRoutingTariff2() const { return _tariffCrossRefInfo->routingTariff2(); }
  const TariffCode& tcrRoutingTariff2Code() const
  {
    return _tariffCrossRefInfo->routingTariff2Code();
  }

  const TariffCode& tcrAddonTariff1Code() const { return _tariffCrossRefInfo->addonTariff1Code(); }

  const TariffCode& tcrAddonTariff2Code() const { return _tariffCrossRefInfo->addonTariff2Code(); }

  // accessors to SITA extra fields
  // ========= == ==== ===== ======

  const RouteCode& routeCode() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->routeCode();
  }
  const DBEClass& dbeClass() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->dbeClass();
  }

  const Indicator fareQualCode() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->fareQualCode();
  }
  const Indicator tariffFamily() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->tariffFamily();
  }

  const Indicator cabotageInd() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->cabotageInd();
  }
  const Indicator govtAppvlInd() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->govtAppvlInd();
  }
  const Indicator constructionInd() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->constructionInd();
  }
  const Indicator multiLateralInd() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->multiLateralInd();
  }

  const LocCode& airport1() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->airport1();
  }
  const LocCode& airport2() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->airport2();
  }

  const Indicator viaCityInd() const
  {
    return dynamic_cast<const SITAFareInfo*>(_fareInfo)->viaCityInd();
  }
  const LocCode& viaCity() const { return dynamic_cast<const SITAFareInfo*>(_fareInfo)->viaCity(); }

  // constructed fare common data accessors
  // =========== ==== ====== ==== =========

  const ConstructedFareInfo::ConstructionType constructionType() const
  {
    return _constructedFareInfo->constructionType();
  }

  const LocCode& gateway1() const { return _constructedFareInfo->gateway1(); }
  const LocCode& gateway2() const { return _constructedFareInfo->gateway2(); }

  const MoneyAmount specifiedFareAmount() const
  {
    return _constructedFareInfo->specifiedFareAmount();
  }

  const MoneyAmount constructedNucAmount() const
  {
    return _constructedFareInfo->constructedNucAmount();
  }

  // origin add-on data accessors
  // ====== ====== ==== =========

  const AddonZone origAddonZone() const { return _constructedFareInfo->origAddonZone(); }

  const Footnote& origAddonFootNote1() const { return _constructedFareInfo->origAddonFootNote1(); }
  const Footnote& origAddonFootNote2() const { return _constructedFareInfo->origAddonFootNote2(); }

  const FareClassCode& origAddonFareClass() const
  {
    return _constructedFareInfo->origAddonFareClass();
  }

  const TariffNumber origAddonTariff() const { return _constructedFareInfo->origAddonTariff(); }

  const RoutingNumber& origAddonRouting() const { return _constructedFareInfo->origAddonRouting(); }

  const MoneyAmount origAddonAmount() const { return _constructedFareInfo->origAddonAmount(); }

  const CurrencyCode& origAddonCurrency() const
  {
    return _constructedFareInfo->origAddonCurrency();
  }

  const Indicator origAddonOWRT() const { return _constructedFareInfo->origAddonOWRT(); }

  // destination add-on data accessors
  // =========== ====== ==== =========

  const AddonZone destAddonZone() const { return _constructedFareInfo->destAddonZone(); }

  const Footnote& destAddonFootNote1() const { return _constructedFareInfo->destAddonFootNote1(); }
  const Footnote& destAddonFootNote2() const { return _constructedFareInfo->destAddonFootNote2(); }

  const FareClassCode& destAddonFareClass() const
  {
    return _constructedFareInfo->destAddonFareClass();
  }

  const TariffNumber destAddonTariff() const { return _constructedFareInfo->destAddonTariff(); }

  const RoutingNumber& destAddonRouting() const { return _constructedFareInfo->destAddonRouting(); }

  const MoneyAmount destAddonAmount() const { return _constructedFareInfo->destAddonAmount(); }

  const CurrencyCode& destAddonCurrency() const
  {
    return _constructedFareInfo->destAddonCurrency();
  }

  const Indicator destAddonOWRT() const { return _constructedFareInfo->destAddonOWRT(); }

  // accessors to constructed SITA fare fields
  // ========= == =========== ==== ==== ======

  const RoutingNumber& throughFareRouting() const
  {
    return dynamic_cast<const SITAConstructedFareInfo*>(_constructedFareInfo)->throughFareRouting();
  }
  const Indicator throughMPMInd() const
  {
    return dynamic_cast<const SITAConstructedFareInfo*>(_constructedFareInfo)->throughMPMInd();
  }
  const RuleNumber& throughRule() const
  {
    return dynamic_cast<const SITAConstructedFareInfo*>(_constructedFareInfo)->throughRule();
  }

  FopStatus& mutableForbiddenFop() { return _forbiddenFop; }
  const FopStatus& forbiddenFop() const { return _forbiddenFop; }

  DateTime& latestTktDT() { return _latestTktDT; }
  const DateTime& latestTktDT() const { return _latestTktDT; }

  DateTime& latestTktDTFootN() { return _latestTktDTFootN; }
  const DateTime& latestTktDTFootN() const { return _latestTktDTFootN; }

  DateTime& latestTktDTFareR() { return _latestTktDTFareR; }
  const DateTime& latestTktDTFareR() const { return _latestTktDTFareR; }

  DateTime& latestTktDTGenR() { return _latestTktDTGenR; }
  const DateTime& latestTktDTGenR() const { return _latestTktDTGenR; }

  void skipAllCategoryValidation() { _ruleProcessStatus.set(RS_AllCat); }

  void setAllCategoryValidForShopping() { _ruleStatus.set(RS_AllCat, false); }

  Indicator& domesticFootNote() { return _domesticFootNote; }
  const Indicator& domesticFootNote() const { return _domesticFootNote; }

  std::vector<CarrierCode>& invalidValidatingCarriers() { return _invalidValidatingCarriers; }
  const std::vector<CarrierCode>& invalidValidatingCarriers() const { return _invalidValidatingCarriers; }

  // protected types
  // ========= =====

  enum RuleState
  {
    RS_CatNotSupported = 0,
    RS_Cat02 = 0x00000001,
    RS_Cat03 = 0x00000002,
    RS_Cat04 = 0x00000004,
    RS_Cat05 = 0x00000008,
    RS_Cat06 = 0x00000010,
    RS_Cat07 = 0x00000020,
    RS_Cat08 = 0x00000040,
    RS_Cat09 = 0x00000080,
    RS_Cat10 = 0x00000100,
    RS_Cat11 = 0x00000200,
    RS_Cat12 = 0x00000400,
    RS_Cat14 = 0x00000800,
    RS_Cat15 = 0x00001000,
    RS_Cat16 = 0x00002000,
    RS_Cat23 = 0x00004000,
    RS_Cat27 = 0x00008000,
    RS_AllCat = 0x0000FFFF,

    // Categories that differential dynamic validation will process
    RS_AllDiffCat = RS_Cat02 & RS_Cat03 & RS_Cat04 & RS_Cat05 & RS_Cat06 & RS_Cat07 & RS_Cat08 &
                    RS_Cat09 & RS_Cat11 & RS_Cat14 & RS_Cat15,
    RS_MaxCat = 35
  };

  typedef SmallBitSet<uint16_t, RuleState> RuleStatus;

  enum FootNoteState
  {
    FootNote_Unknown = 0x00000000,
    MissFootNote1 = 0x00000001,
    MissFootNote2 = 0x00000002,
    MissFootNote3 = 0x00000004,
    MissFootNote4 = 0x00000008,
    MissFootNote5 = 0x00000010,
    MissFootNote6 = 0x00000020,
    MissingAnyFootNote = 0x0000003F,
    MissingFootNote = 0x00000040,
    InitializedFnStat = 0x00000080,
    MAX_NUMOF_FOOTNOTE = 6
  };

  typedef SmallBitSet<uint8_t, FootNoteState> FootNoteStatus;

  // fare status
  // ==== ======

  FareStatus _status;

  // branded fare status
  // ==== ======

  std::map<uint16_t, BrandedFareStatus> _brandedFareStatus;

  // rule status
  // ==== ======

  // to represent rule status we use negative logic.
  // if for any category status bit == 1, this means
  // that the category was FAILED.

  // rule process uses normal (positive) logic.

  RuleStatus _ruleStatus;
  RuleStatus _ruleProcessStatus;
  RuleStatus _ruleSoftPassStatus;

  RuleStatus _dontCheckFareRuleAltGenRule;
  RuleStatus _footnoteRec2Status;

  FootNoteStatus _footNoteStatus;

  enum NationFranceC15State
  {
    NFRS_NationFR_Unknown = 0x00,
    NFRS_NationFRInCat15 = 0x01,
    NFRS_NationFRInCat15Fn = 0x02,
    NFRS_NationFRInCat15Fr = 0x04,
    NFRS_NationFRInCat15Gr = 0x08
  };

  typedef SmallBitSet<uint8_t, NationFranceC15State> NationFranceC15Status;

  bool isNationFRInCat15() const { return _nFrC15Status.isSet(NFRS_NationFRInCat15); }
  bool setNationFRInCat15(const bool isValid = true)
  {
    return _nFrC15Status.set(NFRS_NationFRInCat15, isValid);
  }

  bool isNationFRInCat15Fn() const { return _nFrC15Status.isSet(NFRS_NationFRInCat15Fn); }
  bool setNationFRInCat15Fn(const bool isValid = true)
  {
    return _nFrC15Status.set(NFRS_NationFRInCat15Fn, isValid);
  }

  bool isNationFRInCat15Fr() const { return _nFrC15Status.isSet(NFRS_NationFRInCat15Fr); }
  bool setNationFRInCat15Fr(const bool isValid = true)
  {
    return _nFrC15Status.set(NFRS_NationFRInCat15Fr, isValid);
  }

  bool isNationFRInCat15Gr() const { return _nFrC15Status.isSet(NFRS_NationFRInCat15Gr); }
  bool setNationFRInCat15Gr(const bool isValid = true)
  {
    return _nFrC15Status.set(NFRS_NationFRInCat15Gr, isValid);
  }

  enum WarningEtktC15State
  {
    WETS_WarningEtkt_Unknown = 0x00,
    WETS_WarningEtktInCat15 = 0x01,
    WETS_WarningEtktInCat15Fn = 0x02,
    WETS_WarningEtktInCat15Fr = 0x04,
    WETS_WarningEtktInCat15Gr = 0x08
  };

  typedef SmallBitSet<uint8_t, WarningEtktC15State> WarningEtktC15Status;

  bool isWarningEtktInCat15() const { return _wEtktC15Status.isSet(WETS_WarningEtktInCat15); }
  bool setWarningEtktInCat15(const bool isValid = true)
  {
    return _wEtktC15Status.set(WETS_WarningEtktInCat15, isValid);
  }

  bool isWarningEtktInCat15Fn() const { return _wEtktC15Status.isSet(WETS_WarningEtktInCat15Fn); }
  bool setWarningEtktInCat15Fn(const bool isValid = true)
  {
    return _wEtktC15Status.set(WETS_WarningEtktInCat15Fn, isValid);
  }

  bool isWarningEtktInCat15Fr() const { return _wEtktC15Status.isSet(WETS_WarningEtktInCat15Fr); }
  bool setWarningEtktInCat15Fr(const bool isValid = true)
  {
    return _wEtktC15Status.set(WETS_WarningEtktInCat15Fr, isValid);
  }

  bool isWarningEtktInCat15Gr() const { return _wEtktC15Status.isSet(WETS_WarningEtktInCat15Gr); }
  bool setWarningEtktInCat15Gr(const bool isValid = true)
  {
    return _wEtktC15Status.set(WETS_WarningEtktInCat15Gr, isValid);
  }

  enum SecurityC15State
  {
    SC15S_SecurityInCat15_Unknown = 0x00,
    SC15S_SecurityInCat15 = 0x01,
    SC15S_SecurityInCat15Fn = 0x02,
    SC15S_SecurityInCat15Fr = 0x04,
    SC15S_SecurityInCat15Gr = 0x08
  };

  typedef SmallBitSet<uint8_t, SecurityC15State> SecurityC15Status;

  bool isSecurityInCat15Fn() const { return _secC15Status.isSet(SC15S_SecurityInCat15Fn); }
  bool setSecurityInCat15Fn(const bool isValid = true)
  {
    return _secC15Status.set(SC15S_SecurityInCat15Fn, isValid);
  }

  bool isSecurityInCat15Fr() const { return _secC15Status.isSet(SC15S_SecurityInCat15Fr); }
  bool setSecurityInCat15Fr(const bool isValid = true)
  {
    return _secC15Status.set(SC15S_SecurityInCat15Fr, isValid);
  }

  bool isSecurityInCat15Gr() const { return _secC15Status.isSet(SC15S_SecurityInCat15Gr); }
  bool setSecurityInCat15Gr(const bool isValid = true)
  {
    return _secC15Status.set(SC15S_SecurityInCat15Gr, isValid);
  }

  const RuleState mapRuleStateBit(const unsigned int category) const
  {
    return category > RS_MaxCat ? RS_CatNotSupported : _ruleStateMap[category];
  }

  // fare parts
  // ==== =====

  const FareInfo* _fareInfo = nullptr;
  const TariffCrossRefInfo* _tariffCrossRefInfo = nullptr;
  const ConstructedFareInfo* _constructedFareInfo = nullptr;

  // fare extra fields
  // ==== ===== ======

  mutable MoneyAmount _nucFareAmount = 0;
  mutable MoneyAmount _nucMarkupAmount = 0;
  mutable MoneyAmount _rexSecondNucFareAmount = 0;
  mutable MoneyAmount _rexSecondNucOriginalFareAmount = 0;
  mutable MoneyAmount _nucOriginalFareAmount = 0;

  bool _outboundFareForCarnivalInbound = false;

  // Mileage surcharge Fields
  // ======= ========= ======

  uint16_t _mileageSurchargePctg = 0;
  MoneyAmount _mileageSurchargeAmt = 0;
  MoneyAmount _rexSecondMileageSurchargeAmt = 0;

  // static fields
  // ====== ======

  static const Fare _emptyFare;
  static const RuleState _ruleStateMap[RS_MaxCat + 1];
  // FOP trailer MSG from SalesRestriction rule
  // ====== ======
  FopStatus _forbiddenFop;

  // Latest Tkt date from Cat15
  // ====== === ==== ==== =====
  DateTime _latestTktDT = DateTime::openDate();
  DateTime _latestTktDTFootN = DateTime::openDate();
  DateTime _latestTktDTFareR = DateTime::openDate();
  DateTime _latestTktDTGenR = DateTime::openDate();

  NationFranceC15Status _nFrC15Status;
  WarningEtktC15Status _wEtktC15Status;
  SecurityC15Status _secC15Status;

  Indicator _domesticFootNote = ' ';

  bool _reProcessCat05NoMatch = false;

  std::vector<CarrierCode> _invalidValidatingCarriers;

  // initialization
  // ==============

  void setGeoTravelType(const GeoTravelType geoTravelType);

  void checkGlobalDirection(const FareMarket& fareMarket);

  void addInvalidValidatingCxr(const CarrierCode& cxr);
  void addInvalidValidatingCxr(const std::vector<CarrierCode>& cxrs);

  WarningMap& warningMap();
  const WarningMap& warningMap() const;

private:
  mutable WarningMap _warningMap;

};
} // tse namespace

