//----------------------------------------------------------------------------
// File:           TaxCodeReg.h
// Description:    TaxCodeReg processing data
// Created:        2/4/2004
// Authors:        Roger Kelly
//
// Updates:
//
// � 2004, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DBAccess/TaxCodeGenText.h"
#include "DBAccess/TaxExemptionCarrier.h"
#include "DBAccess/TaxRestrictionPsg.h"
#include "DBAccess/TaxRestrictionTktDesignator.h"
#include "DBAccess/TaxRestrictionTransit.h"

#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>

namespace tse
{

class TaxCodeReg
{
  friend class TaxCodeRegTest;

  class AmtInt
  {
    friend class TaxCodeRegTest;

  public:
    AmtInt() : _nodec(0), _amt(0) {}

    unsigned int& amt() { return _amt; }
    const unsigned int& amt() const { return _amt; }

    unsigned int& nodec() { return _nodec; }
    const unsigned int& nodec() const { return _nodec; }

    std::string toString() { return toString(_amt, _nodec); }
    std::string percentageToString() { return toString(_amt * 100, _nodec); }

    bool operator==(const AmtInt& rhs) const
    {
      return ((_nodec == rhs._nodec) && (_amt == rhs._amt));
    }

    static void dummyData(AmtInt& obj)
    {
      obj._nodec = 2;
      obj._amt = 123;
    }

    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _nodec);
      FLATTENIZE(archive, _amt);
    }

  protected:
    std::string toString(unsigned int amount, unsigned int noDec)
    {

      std::string amountStr = boost::lexical_cast<std::string>(amount);

      if (!noDec)
      {
        return amountStr;
      }
      else if (amountStr.size() > noDec)
      {
        return zeroStripper(amountStr.substr(0, amountStr.size() - noDec) + "." +
                            amountStr.substr(amountStr.size() - noDec, noDec));
      }
      else
      {
        return zeroStripper("0." + std::string(noDec - amountStr.size(), '0') + amountStr);
      }
    }

    std::string zeroStripper(const std::string& amount)
    {
      size_t i = 0;
      std::string::const_reverse_iterator it = amount.rbegin();
      std::string::const_reverse_iterator itEnd = amount.rend();

      for (; it != itEnd; it++)
      {
        if (*it != '0')
        {
          if (*it == '.')
            i++;

          break;
        }
        i++;
      }

      return amount.substr(0, amount.size() - i);
    }

    unsigned int _nodec;
    unsigned int _amt;
  };

public:
  TaxCodeReg()
  {
    _taxCode = "";
    _versionDate = 0;
    _seqNo = 0;
    // DateTime _createDate;   Dates init themselves
    // DateTime _expireDate;   Dates init themselves
    // DateTime _lockDate;     Dates init themselves
    _specialProcessNo = 0;
    _taxNodec = 0;
    _taxcdRoundUnitNodec = 0;
    _fareRangeNodec = 0;
    _discPercentNodec = 0;
    _discPercent = 0.00;
    _taxAmt = 0.00;
    _minTax = 0.00;
    _maxTax = 0.00;
    _plusupAmt = 0.00;
    _lowRange = 0.00;
    _highRange = 0.00;
    _rangeincrement = 0.00;
    _taxcdRoundUnit = 0.00;
    // DateTime _effDate;      Dates init themselves
    // DateTime _discDate;     Dates init themselves
    // DateTime _firstTvlDate; Dates init themselves
    // DateTime _lastTvlDate;  Dates init themselves
    // DateTime _modDate;  Dates init themselves
    _loc1ExclInd = '\0';
    _loc1Type = '\0';
    _loc1 = "";
    _loc2ExclInd = '\0';
    _loc2Type = '\0';
    _loc2 = "";
    _taxType = '\0';
    _nation = "";
    _taxcdRoundRule = EMPTY;
    _taxfullFareInd = '\0';
    _taxequivAmtInd = '\0';
    _taxexcessbagInd = '\0';
    _tvlDateasoriginInd = '\0';
    _displayonlyInd = '\0';
    _feeInd = '\0';
    _interlinableTaxInd = '\0';
    _showseparateInd = '\0';
    _posExclInd = '\0';
    _posLocType = UNKNOWN_LOC;
    _posLoc = "";
    _poiExclInd = '\0';
    _poiLocType = UNKNOWN_LOC;
    _poiLoc = "";
    _sellCurExclInd = '\0';
    _sellCur = "";
    _occurrence = '\0';
    _freeTktexempt = '\0';
    _idTvlexempt = '\0';
    _rangeType = '\0';
    _rangeInd = '\0';
    _nextstopoverrestr = '\0';
    _spclTaxRounding = '\0';
    _taxCur = "";
    _taxCurNodec = 0;
    _fareclassExclInd = '\0';
    _tktdsgExclInd = '\0';
    _valcxrExclInd = '\0';
    _exempequipExclInd = '\0';
    _psgrExclInd = '\0';
    _exempcxrExclInd = '\0';
    _fareTypeExclInd = '\0';
    _multioccconvrndInd = '\0';
    _originLocType = UNKNOWN_LOC;
    _originLoc = "";
    _originLocExclInd = '\0';
    _versioninheritedInd = '\0';
    _versiondisplayInd = '\0';
    _loc1Appl = '\0';
    _loc2Appl = '\0';
    _tripType = '\0';
    _travelType = '\0';
    _itineraryType = '\0';
    _formOfPayment = '\0';
    _taxOnTaxExcl = '\0';
    _feeApplInd = '\0';
    _bookingCode1 = NULL_CODE;
    _bookingCode2 = NULL_CODE;
    _bookingCode3 = NULL_CODE;
    _taxRestrictionLocNo = NULL_CODE;
    _specConfigName = "";
  }

  ~TaxCodeReg()
  { // Nuke the Kids!
    std::vector<TaxCodeGenText*>::iterator textIt;
    for (textIt = _taxCodeGenTexts.begin(); textIt != _taxCodeGenTexts.end(); textIt++)
    {
      delete *textIt;
    }
    std::vector<TaxCodeCabin*>::iterator cabIt;
    for (cabIt = _cabins.begin(); cabIt != _cabins.end(); cabIt++)
    {
      delete *cabIt;
    }
    std::vector<TaxRestrictionTktDesignator*>::iterator dsgIt;
    for (dsgIt = _taxRestrTktDsgs.begin(); dsgIt != _taxRestrTktDsgs.end(); dsgIt++)
    {
      delete *dsgIt;
    }
  }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  uint64_t& versionDate() { return _versionDate; }
  const uint64_t& versionDate() const { return _versionDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  long& specialProcessNo() { return _specialProcessNo; }
  const long& specialProcessNo() const { return _specialProcessNo; }

  CurrencyNoDec& taxNodec() { return _taxNodec; }
  const CurrencyNoDec& taxNodec() const { return _taxNodec; }

  CurrencyNoDec& taxcdRoundUnitNodec() { return _taxcdRoundUnitNodec; }
  const CurrencyNoDec& taxcdRoundUnitNodec() const { return _taxcdRoundUnitNodec; }

  CurrencyNoDec& fareRangeNodec() { return _fareRangeNodec; }
  const CurrencyNoDec& fareRangeNodec() const { return _fareRangeNodec; }

  CurrencyNoDec& discPercentNodec() { return _discPercentNodec; }
  const CurrencyNoDec& discPercentNodec() const { return _discPercentNodec; }

  Percent& discPercent() { return _discPercent; }
  const Percent& discPercent() const { return _discPercent; }

  MoneyAmount& taxAmt() { return _taxAmt; }
  const MoneyAmount& taxAmt() const { return _taxAmt; }

  MoneyAmount& minTax() { return _minTax; }
  const MoneyAmount& minTax() const { return _minTax; }

  MoneyAmount& maxTax() { return _maxTax; }
  const MoneyAmount& maxTax() const { return _maxTax; }

  MoneyAmount& plusupAmt() { return _plusupAmt; }
  const MoneyAmount& plusupAmt() const { return _plusupAmt; }

  MoneyAmount& lowRange() { return _lowRange; }
  const MoneyAmount& lowRange() const { return _lowRange; }

  MoneyAmount& highRange() { return _highRange; }
  const MoneyAmount& highRange() const { return _highRange; }

  MoneyAmount& rangeincrement() { return _rangeincrement; }
  const MoneyAmount& rangeincrement() const { return _rangeincrement; }

  RoundingFactor& taxcdRoundUnit() { return _taxcdRoundUnit; }
  const RoundingFactor& taxcdRoundUnit() const { return _taxcdRoundUnit; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& firstTvlDate() { return _firstTvlDate; }
  const DateTime& firstTvlDate() const { return _firstTvlDate; }

  DateTime& lastTvlDate() { return _lastTvlDate; }
  const DateTime& lastTvlDate() const { return _lastTvlDate; }

  DateTime& modDate() { return _modDate; }
  const DateTime& modDate() const { return _modDate; }

  Indicator& loc1ExclInd() { return _loc1ExclInd; }
  const Indicator& loc1ExclInd() const { return _loc1ExclInd; }

  LocTypeCode& loc1Type() { return _loc1Type; }
  const LocTypeCode& loc1Type() const { return _loc1Type; }

  LocCode& loc1() { return _loc1; }
  const LocCode& loc1() const { return _loc1; }

  Indicator& loc2ExclInd() { return _loc2ExclInd; }
  const Indicator& loc2ExclInd() const { return _loc2ExclInd; }

  LocTypeCode& loc2Type() { return _loc2Type; }
  const LocTypeCode& loc2Type() const { return _loc2Type; }

  LocCode& loc2() { return _loc2; }
  const LocCode& loc2() const { return _loc2; }

  TaxTypeCode& taxType() { return _taxType; }
  const TaxTypeCode& taxType() const { return _taxType; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  RoundingRule& taxcdRoundRule() { return _taxcdRoundRule; }
  const RoundingRule& taxcdRoundRule() const { return _taxcdRoundRule; }

  Indicator& taxfullFareInd() { return _taxfullFareInd; }
  const Indicator& taxfullFareInd() const { return _taxfullFareInd; }

  Indicator& taxequivAmtInd() { return _taxequivAmtInd; }
  const Indicator& taxequivAmtInd() const { return _taxequivAmtInd; }

  Indicator& taxexcessbagInd() { return _taxexcessbagInd; }
  const Indicator& taxexcessbagInd() const { return _taxexcessbagInd; }

  Indicator& tvlDateasoriginInd() { return _tvlDateasoriginInd; }
  const Indicator& tvlDateasoriginInd() const { return _tvlDateasoriginInd; }

  Indicator& displayonlyInd() { return _displayonlyInd; }
  const Indicator& displayonlyInd() const { return _displayonlyInd; }

  Indicator& feeInd() { return _feeInd; }
  const Indicator& feeInd() const { return _feeInd; }

  Indicator& interlinableTaxInd() { return _interlinableTaxInd; }
  const Indicator& interlinableTaxInd() const { return _interlinableTaxInd; }

  Indicator& showseparateInd() { return _showseparateInd; }
  const Indicator& showseparateInd() const { return _showseparateInd; }

  Indicator& posExclInd() { return _posExclInd; }
  const Indicator& posExclInd() const { return _posExclInd; }

  LocType& posLocType() { return _posLocType; }
  const LocType& posLocType() const { return _posLocType; }

  LocCode& posLoc() { return _posLoc; }
  const LocCode& posLoc() const { return _posLoc; }

  Indicator& poiExclInd() { return _poiExclInd; }
  const Indicator& poiExclInd() const { return _poiExclInd; }

  LocType& poiLocType() { return _poiLocType; }
  const LocType& poiLocType() const { return _poiLocType; }

  LocCode& poiLoc() { return _poiLoc; }
  const LocCode& poiLoc() const { return _poiLoc; }

  Indicator& sellCurExclInd() { return _sellCurExclInd; }
  const Indicator& sellCurExclInd() const { return _sellCurExclInd; }

  CurrencyCode& sellCur() { return _sellCur; }
  const CurrencyCode& sellCur() const { return _sellCur; }

  Indicator& occurrence() { return _occurrence; }
  const Indicator& occurrence() const { return _occurrence; }

  Indicator& freeTktexempt() { return _freeTktexempt; }
  const Indicator& freeTktexempt() const { return _freeTktexempt; }

  Indicator& idTvlexempt() { return _idTvlexempt; }
  const Indicator& idTvlexempt() const { return _idTvlexempt; }

  TaxRangeType& rangeType() { return _rangeType; }
  const TaxRangeType& rangeType() const { return _rangeType; }

  Indicator& rangeInd() { return _rangeInd; }
  const Indicator& rangeInd() const { return _rangeInd; }

  Indicator& nextstopoverrestr() { return _nextstopoverrestr; }
  const Indicator& nextstopoverrestr() const { return _nextstopoverrestr; }

  Indicator& spclTaxRounding() { return _spclTaxRounding; }
  const Indicator& spclTaxRounding() const { return _spclTaxRounding; }

  CurrencyCode& taxCur() { return _taxCur; }
  const CurrencyCode& taxCur() const { return _taxCur; }

  CurrencyNoDec& taxCurNodec() { return _taxCurNodec; }
  const CurrencyNoDec& taxCurNodec() const { return _taxCurNodec; }

  Indicator& fareclassExclInd() { return _fareclassExclInd; }
  const Indicator& fareclassExclInd() const { return _fareclassExclInd; }

  Indicator& tktdsgExclInd() { return _tktdsgExclInd; }
  const Indicator& tktdsgExclInd() const { return _tktdsgExclInd; }

  Indicator& valcxrExclInd() { return _valcxrExclInd; }
  const Indicator& valcxrExclInd() const { return _valcxrExclInd; }

  Indicator& exempequipExclInd() { return _exempequipExclInd; }
  const Indicator& exempequipExclInd() const { return _exempequipExclInd; }

  Indicator& psgrExclInd() { return _psgrExclInd; }
  const Indicator& psgrExclInd() const { return _psgrExclInd; }

  Indicator& exempcxrExclInd() { return _exempcxrExclInd; }
  const Indicator& exempcxrExclInd() const { return _exempcxrExclInd; }

  Indicator& fareTypeExclInd() { return _fareTypeExclInd; }
  const Indicator& fareTypeExclInd() const { return _fareTypeExclInd; }

  Indicator& multioccconvrndInd() { return _multioccconvrndInd; }
  const Indicator& multioccconvrndInd() const { return _multioccconvrndInd; }

  LocType& originLocType() { return _originLocType; }
  const LocType& originLocType() const { return _originLocType; }

  LocCode& originLoc() { return _originLoc; }
  const LocCode& originLoc() const { return _originLoc; }

  Indicator& originLocExclInd() { return _originLocExclInd; }
  const Indicator& originLocExclInd() const { return _originLocExclInd; }

  Indicator& versioninheritedInd() { return _versioninheritedInd; }
  const Indicator& versioninheritedInd() const { return _versioninheritedInd; }

  Indicator& versiondisplayInd() { return _versiondisplayInd; }
  const Indicator& versiondisplayInd() const { return _versiondisplayInd; }

  Indicator& loc1Appl() { return _loc1Appl; }
  const Indicator& loc1Appl() const { return _loc1Appl; }

  Indicator& loc2Appl() { return _loc2Appl; }
  const Indicator& loc2Appl() const { return _loc2Appl; }

  Indicator& tripType() { return _tripType; }
  const Indicator& tripType() const { return _tripType; }

  Indicator& travelType() { return _travelType; }
  const Indicator& travelType() const { return _travelType; }

  Indicator& itineraryType() { return _itineraryType; }
  const Indicator& itineraryType() const { return _itineraryType; }

  Indicator& formOfPayment() { return _formOfPayment; }
  const Indicator& formOfPayment() const { return _formOfPayment; }

  Indicator& taxOnTaxExcl() { return _taxOnTaxExcl; }
  const Indicator& taxOnTaxExcl() const { return _taxOnTaxExcl; }

  Indicator& feeApplInd() { return _feeApplInd; }
  const Indicator& feeApplInd() const { return _feeApplInd; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  BookingCode& bookingCode3() { return _bookingCode3; }
  const BookingCode& bookingCode3() const { return _bookingCode3; }

  std::vector<CarrierCode>& restrictionValidationCxr() { return _restrictionValidationCxr; }
  const std::vector<CarrierCode>& restrictionValidationCxr() const
  {
    return _restrictionValidationCxr;
  }

  std::vector<TaxRestrictionPsg>& restrictionPsg() { return _restrictionPsg; }
  const std::vector<TaxRestrictionPsg>& restrictionPsg() const { return _restrictionPsg; }

  std::vector<FareType>& restrictionFareType() { return _restrictionFareType; }
  const std::vector<FareType>& restrictionFareType() const { return _restrictionFareType; }

  std::vector<FareClassCode>& restrictionFareClass() { return _restrictionFareClass; }
  const std::vector<FareClassCode>& restrictionFareClass() const { return _restrictionFareClass; }

  std::vector<std::string>& equipmentCode() { return _equipmentCode; }
  const std::vector<std::string>& equipmentCode() const { return _equipmentCode; }

  std::vector<TaxExemptionCarrier>& exemptionCxr() { return _exemptionCxr; }
  const std::vector<TaxExemptionCarrier>& exemptionCxr() const { return _exemptionCxr; }

  std::vector<std::string>& taxOnTaxCode() { return _taxOnTaxCode; }
  const std::vector<std::string>& taxOnTaxCode() const { return _taxOnTaxCode; }

  std::vector<TaxRestrictionTransit>& restrictionTransit() { return _restrictionTransit; }
  const std::vector<TaxRestrictionTransit>& restrictionTransit() const
  {
    return _restrictionTransit;
  }

  std::vector<TaxCodeGenText*>& taxCodeGenTexts() { return _taxCodeGenTexts; }
  const std::vector<TaxCodeGenText*>& taxCodeGenTexts() const { return _taxCodeGenTexts; }

  std::vector<TaxCodeCabin*>& cabins() { return _cabins; }
  const std::vector<TaxCodeCabin*>& cabins() const { return _cabins; }

  std::vector<TaxRestrictionTktDesignator*>& taxRestrTktDsgs() { return _taxRestrTktDsgs; }
  const std::vector<TaxRestrictionTktDesignator*>& taxRestrTktDsgs() const
  {
    return _taxRestrTktDsgs;
  }

  AmtInt& taxAmtInt() { return _taxAmtInt; }
  const AmtInt& taxAmtInt() const { return _taxAmtInt; }

  AmtInt& minTaxInt() { return _minTaxInt; }
  const AmtInt& minTaxInt() const { return _minTaxInt; }

  AmtInt& maxTaxInt() { return _maxTaxInt; }
  const AmtInt& maxTaxInt() const { return _maxTaxInt; }

  AmtInt& lowRangeInt() { return _lowRangeInt; }
  const AmtInt& lowRangeInt() const { return _lowRangeInt; }

  AmtInt& highRangeInt() { return _highRangeInt; }
  const AmtInt& highRangeInt() const { return _highRangeInt; }

  AmtInt& taxcdRoundUnitInt() { return _taxcdRoundUnitInt; }
  const AmtInt& taxcdRoundUnitInt() const { return _taxcdRoundUnitInt; }

  AmtInt& discPercentInt() { return _discPercentInt; }
  const AmtInt& discPercentInt() const { return _discPercentInt; }

  TaxRestrictionLocation& taxRestrictionLocNo() { return _taxRestrictionLocNo; }
  const TaxRestrictionLocation& taxRestrictionLocNo() const { return _taxRestrictionLocNo; }

  TaxSpecConfigName& specConfigName() { return _specConfigName; }
  const TaxSpecConfigName& specConfigName() const { return _specConfigName; }

  bool operator==(const TaxCodeReg& rhs) const
  {
    bool
    eq((_taxCode == rhs._taxCode) && (_versionDate == rhs._versionDate) && (_seqNo == rhs._seqNo) &&
       (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
       (_lockDate == rhs._lockDate) && (_specialProcessNo == rhs._specialProcessNo) &&
       (_taxNodec == rhs._taxNodec) && (_taxcdRoundUnitNodec == rhs._taxcdRoundUnitNodec) &&
       (_fareRangeNodec == rhs._fareRangeNodec) && (_discPercentNodec == rhs._discPercentNodec) &&
       (_discPercent == rhs._discPercent) && (_taxAmt == rhs._taxAmt) && (_minTax == rhs._minTax) &&
       (_maxTax == rhs._maxTax) && (_plusupAmt == rhs._plusupAmt) && (_lowRange == rhs._lowRange) &&
       (_highRange == rhs._highRange) && (_rangeincrement == rhs._rangeincrement) &&
       (_taxcdRoundUnit == rhs._taxcdRoundUnit) && (_effDate == rhs._effDate) &&
       (_discDate == rhs._discDate) && (_firstTvlDate == rhs._firstTvlDate) &&
       (_lastTvlDate == rhs._lastTvlDate) && (_modDate == rhs._modDate) &&
       (_loc1ExclInd == rhs._loc1ExclInd) && (_loc1Type == rhs._loc1Type) && (_loc1 == rhs._loc1) &&
       (_loc2ExclInd == rhs._loc2ExclInd) && (_loc2Type == rhs._loc2Type) && (_loc2 == rhs._loc2) &&
       (_taxType == rhs._taxType) && (_nation == rhs._nation) &&
       (_taxcdRoundRule == rhs._taxcdRoundRule) && (_taxfullFareInd == rhs._taxfullFareInd) &&
       (_taxequivAmtInd == rhs._taxequivAmtInd) && (_taxexcessbagInd == rhs._taxexcessbagInd) &&
       (_tvlDateasoriginInd == rhs._tvlDateasoriginInd) &&
       (_displayonlyInd == rhs._displayonlyInd) && (_feeInd == rhs._feeInd) &&
       (_interlinableTaxInd == rhs._interlinableTaxInd) &&
       (_showseparateInd == rhs._showseparateInd) && (_posExclInd == rhs._posExclInd) &&
       (_posLocType == rhs._posLocType) && (_posLoc == rhs._posLoc) &&
       (_poiExclInd == rhs._poiExclInd) && (_poiLocType == rhs._poiLocType) &&
       (_poiLoc == rhs._poiLoc) && (_sellCurExclInd == rhs._sellCurExclInd) &&
       (_sellCur == rhs._sellCur) && (_occurrence == rhs._occurrence) &&
       (_freeTktexempt == rhs._freeTktexempt) && (_idTvlexempt == rhs._idTvlexempt) &&
       (_rangeType == rhs._rangeType) && (_rangeInd == rhs._rangeInd) &&
       (_nextstopoverrestr == rhs._nextstopoverrestr) &&
       (_spclTaxRounding == rhs._spclTaxRounding) && (_taxCur == rhs._taxCur) &&
       (_taxCurNodec == rhs._taxCurNodec) && (_fareclassExclInd == rhs._fareclassExclInd) &&
       (_tktdsgExclInd == rhs._tktdsgExclInd) && (_valcxrExclInd == rhs._valcxrExclInd) &&
       (_exempequipExclInd == rhs._exempequipExclInd) && (_psgrExclInd == rhs._psgrExclInd) &&
       (_exempcxrExclInd == rhs._exempcxrExclInd) && (_fareTypeExclInd == rhs._fareTypeExclInd) &&
       (_multioccconvrndInd == rhs._multioccconvrndInd) && (_originLocType == rhs._originLocType) &&
       (_originLoc == rhs._originLoc) && (_originLocExclInd == rhs._originLocExclInd) &&
       (_versioninheritedInd == rhs._versioninheritedInd) &&
       (_versiondisplayInd == rhs._versiondisplayInd) && (_loc1Appl == rhs._loc1Appl) &&
       (_loc2Appl == rhs._loc2Appl) && (_tripType == rhs._tripType) &&
       (_travelType == rhs._travelType) && (_itineraryType == rhs._itineraryType) &&
       (_formOfPayment == rhs._formOfPayment) && (_taxOnTaxExcl == rhs._taxOnTaxExcl) &&
       (_feeApplInd == rhs._feeApplInd) && (_bookingCode1 == rhs._bookingCode1) &&
       (_bookingCode2 == rhs._bookingCode2) && (_bookingCode3 == rhs._bookingCode3) &&
       (_restrictionValidationCxr == rhs._restrictionValidationCxr) &&
       (_restrictionPsg == rhs._restrictionPsg) &&
       (_restrictionFareType == rhs._restrictionFareType) &&
       (_restrictionFareClass == rhs._restrictionFareClass) &&
       (_equipmentCode == rhs._equipmentCode) && (_exemptionCxr == rhs._exemptionCxr) &&
       (_taxOnTaxCode == rhs._taxOnTaxCode) && (_restrictionTransit == rhs._restrictionTransit) &&
       (_taxCodeGenTexts.size() == rhs._taxCodeGenTexts.size()) &&
       (_cabins.size() == rhs._cabins.size()) &&
       (_taxRestrTktDsgs.size() == rhs._taxRestrTktDsgs.size()) && (_taxAmtInt == rhs._taxAmtInt) &&
       (_minTaxInt == rhs._minTaxInt) && (_maxTaxInt == rhs._maxTaxInt) &&
       (_taxcdRoundUnitInt == rhs._taxcdRoundUnitInt) && (_lowRangeInt == rhs._lowRangeInt) &&
       (_highRangeInt == rhs._highRangeInt) && (_discPercentInt == rhs._discPercentInt) &&
       (_taxRestrictionLocNo == rhs._taxRestrictionLocNo) &&
       (_specConfigName == rhs._specConfigName));

    for (size_t i = 0; (eq && (i < _taxCodeGenTexts.size())); ++i)
    {
      eq = (*(_taxCodeGenTexts[i]) == *(rhs._taxCodeGenTexts[i]));
    }

    for (size_t j = 0; (eq && (j < _cabins.size())); ++j)
    {
      eq = (*(_cabins[j]) == *(rhs._cabins[j]));
    }

    for (size_t k = 0; (eq && (k < _taxRestrTktDsgs.size())); ++k)
    {
      eq = (*(_taxRestrTktDsgs[k]) == *(rhs._taxRestrTktDsgs[k]));
    }

    return eq;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxCodeReg& obj)
  {
    obj._taxCode = "ABC";
    obj._versionDate = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._specialProcessNo = 3;
    obj._taxNodec = 4;
    obj._taxcdRoundUnitNodec = 5;
    obj._fareRangeNodec = 6;
    obj._discPercentNodec = 7;
    obj._discPercent = 8.888;
    obj._taxAmt = 9.99;
    obj._minTax = 10.10;
    obj._maxTax = 11.11;
    obj._plusupAmt = 12.12;
    obj._lowRange = 13.13;
    obj._highRange = 14.14;
    obj._rangeincrement = 15.15;
    obj._taxcdRoundUnit = 16.1616;
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._firstTvlDate = time(nullptr);
    obj._lastTvlDate = time(nullptr);
    obj._modDate = time(nullptr);
    obj._loc1ExclInd = 'D';
    obj._loc1Type = NATION;
    obj._loc1 = "MNOPQRST";
    obj._loc2ExclInd = 'U';
    obj._loc2Type = MARKET;
    obj._loc2 = "cdefghij";
    obj._taxType = 'k';
    obj._nation = "lmno";
    obj._taxcdRoundRule = DOWN;
    obj._taxfullFareInd = 'p';
    obj._taxequivAmtInd = 'q';
    obj._taxexcessbagInd = 'r';
    obj._tvlDateasoriginInd = 's';
    obj._displayonlyInd = 't';
    obj._feeInd = 'u';
    obj._interlinableTaxInd = 'v';
    obj._showseparateInd = 'w';
    obj._posExclInd = 'x';
    obj._posLocType = NATION;
    obj._posLoc = "yz123456";
    obj._poiExclInd = '7';
    obj._poiLocType = NATION;
    obj._poiLoc = "7890ABCD";
    obj._sellCurExclInd = 'E';
    obj._sellCur = "FGH";
    obj._occurrence = 'I';
    obj._freeTktexempt = 'J';
    obj._idTvlexempt = 'K';
    obj._rangeType = 'L';
    obj._rangeInd = 'M';
    obj._nextstopoverrestr = 'N';
    obj._spclTaxRounding = 'O';
    obj._taxCur = "PQR";
    obj._taxCurNodec = 17;
    obj._fareclassExclInd = 'S';
    obj._tktdsgExclInd = 'T';
    obj._valcxrExclInd = 'U';
    obj._exempequipExclInd = 'V';
    obj._psgrExclInd = 'W';
    obj._exempcxrExclInd = 'X';
    obj._fareTypeExclInd = 'Y';
    obj._multioccconvrndInd = 'Z';
    obj._originLocType = MARKET;
    obj._originLoc = "abcdefgh";
    obj._originLocExclInd = 'i';
    obj._versioninheritedInd = 'j';
    obj._versiondisplayInd = 'k';
    obj._loc1Appl = 'l';
    obj._loc2Appl = 'm';
    obj._tripType = 'n';
    obj._travelType = 'o';
    obj._itineraryType = 'p';
    obj._formOfPayment = 'q';
    obj._taxOnTaxExcl = 'r';
    obj._feeApplInd = '1';
    obj._bookingCode1 = 'a';
    obj._bookingCode2 = 'b';
    obj._bookingCode3 = 'c';
    obj._taxRestrictionLocNo = 'c';
    obj._specConfigName = 'c';

    obj._restrictionValidationCxr.push_back("stu");
    obj._restrictionValidationCxr.push_back("vwx");

    TaxRestrictionPsg trp1;
    TaxRestrictionPsg trp2;

    TaxRestrictionPsg::dummyData(trp1);
    TaxRestrictionPsg::dummyData(trp2);

    obj._restrictionPsg.push_back(trp1);
    obj._restrictionPsg.push_back(trp2);

    obj._restrictionFareType.push_back("yz123456");
    obj._restrictionFareType.push_back("7890ABCD");

    obj._restrictionFareClass.push_back("EFGHIJKL");
    obj._restrictionFareClass.push_back("MNOPQRST");

    obj._equipmentCode.push_back("UVWXYZab");
    obj._equipmentCode.push_back("cdefghij");

    TaxExemptionCarrier tec1;
    TaxExemptionCarrier tec2;

    TaxExemptionCarrier::dummyData(tec1);
    TaxExemptionCarrier::dummyData(tec2);

    obj._exemptionCxr.push_back(tec1);
    obj._exemptionCxr.push_back(tec2);

    obj._taxOnTaxCode.push_back("qrstuvwx");
    obj._taxOnTaxCode.push_back("yz123456");

    TaxRestrictionTransit tt1;
    TaxRestrictionTransit tt2;

    TaxRestrictionTransit::dummyData(tt1);
    TaxRestrictionTransit::dummyData(tt2);

    obj._restrictionTransit.push_back(tt1);
    obj._restrictionTransit.push_back(tt2);

    TaxCodeGenText* tct1 = new TaxCodeGenText;
    TaxCodeGenText* tct2 = new TaxCodeGenText;

    TaxCodeGenText::dummyData(*tct1);
    TaxCodeGenText::dummyData(*tct2);

    obj._taxCodeGenTexts.push_back(tct1);
    obj._taxCodeGenTexts.push_back(tct2);

    TaxCodeCabin* tcc1 = new TaxCodeCabin;
    TaxCodeCabin* tcc2 = new TaxCodeCabin;

    TaxCodeCabin::dummyData(*tcc1);
    TaxCodeCabin::dummyData(*tcc2);

    obj._cabins.push_back(tcc1);
    obj._cabins.push_back(tcc2);

    TaxRestrictionTktDesignator* trtd1 = new TaxRestrictionTktDesignator;
    TaxRestrictionTktDesignator* trtd2 = new TaxRestrictionTktDesignator;

    TaxRestrictionTktDesignator::dummyData(*trtd1);
    TaxRestrictionTktDesignator::dummyData(*trtd2);

    obj._taxRestrTktDsgs.push_back(trtd1);
    obj._taxRestrTktDsgs.push_back(trtd2);

    AmtInt::dummyData(obj._taxAmtInt);
    AmtInt::dummyData(obj._minTaxInt);
    AmtInt::dummyData(obj._maxTaxInt);
    AmtInt::dummyData(obj._taxcdRoundUnitInt);
    AmtInt::dummyData(obj._lowRangeInt);
    AmtInt::dummyData(obj._highRangeInt);
    AmtInt::dummyData(obj._discPercentInt);
  }

protected:
  TaxCode _taxCode;
  uint64_t _versionDate;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _lockDate;
  long _specialProcessNo;
  CurrencyNoDec _taxNodec;
  CurrencyRoundUnit _taxcdRoundUnitNodec;
  CurrencyNoDec _fareRangeNodec;
  CurrencyNoDec _discPercentNodec;
  Percent _discPercent;
  MoneyAmount _taxAmt;
  MoneyAmount _minTax;
  MoneyAmount _maxTax;
  MoneyAmount _plusupAmt;
  MoneyAmount _lowRange;
  MoneyAmount _highRange;
  MoneyAmount _rangeincrement;
  RoundingFactor _taxcdRoundUnit;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTvlDate;
  DateTime _lastTvlDate;
  DateTime _modDate;
  Indicator _loc1ExclInd;
  LocTypeCode _loc1Type;
  LocCode _loc1;
  Indicator _loc2ExclInd;
  LocTypeCode _loc2Type;
  LocCode _loc2;
  TaxTypeCode _taxType;
  NationCode _nation;
  RoundingRule _taxcdRoundRule;
  Indicator _taxfullFareInd;
  Indicator _taxequivAmtInd;
  Indicator _taxexcessbagInd;
  Indicator _tvlDateasoriginInd;
  Indicator _displayonlyInd;
  Indicator _feeInd;
  Indicator _interlinableTaxInd;
  Indicator _showseparateInd;
  Indicator _posExclInd;
  LocType _posLocType;
  LocCode _posLoc;
  Indicator _poiExclInd;
  LocType _poiLocType;
  LocCode _poiLoc;
  Indicator _sellCurExclInd;
  CurrencyCode _sellCur;
  Indicator _occurrence;
  Indicator _freeTktexempt;
  Indicator _idTvlexempt;
  TaxRangeType _rangeType;
  Indicator _rangeInd;
  Indicator _nextstopoverrestr;
  Indicator _spclTaxRounding;
  CurrencyCode _taxCur;
  CurrencyNoDec _taxCurNodec;
  Indicator _fareclassExclInd;
  Indicator _tktdsgExclInd;
  Indicator _valcxrExclInd;
  Indicator _exempequipExclInd;
  Indicator _psgrExclInd;
  Indicator _exempcxrExclInd;
  Indicator _fareTypeExclInd;
  Indicator _multioccconvrndInd;
  LocType _originLocType;
  LocCode _originLoc;
  Indicator _originLocExclInd;
  Indicator _versioninheritedInd;
  Indicator _versiondisplayInd;
  Indicator _loc1Appl;
  Indicator _loc2Appl;
  Indicator _tripType;
  Indicator _travelType;
  Indicator _itineraryType;
  Indicator _formOfPayment;
  Indicator _taxOnTaxExcl;
  Indicator _feeApplInd;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  BookingCode _bookingCode3;
  TaxRestrictionLocation _taxRestrictionLocNo;
  TaxSpecConfigName _specConfigName;
  std::vector<CarrierCode> _restrictionValidationCxr;
  std::vector<TaxRestrictionPsg> _restrictionPsg;
  std::vector<FareType> _restrictionFareType;
  std::vector<FareClassCode> _restrictionFareClass;
  std::vector<std::string> _equipmentCode;
  std::vector<TaxExemptionCarrier> _exemptionCxr;
  std::vector<std::string> _taxOnTaxCode;
  std::vector<TaxRestrictionTransit> _restrictionTransit;
  std::vector<TaxCodeGenText*> _taxCodeGenTexts;
  std::vector<TaxCodeCabin*> _cabins;
  std::vector<TaxRestrictionTktDesignator*> _taxRestrTktDsgs;

  AmtInt _taxAmtInt;
  AmtInt _minTaxInt;
  AmtInt _maxTaxInt;
  AmtInt _taxcdRoundUnitInt;
  AmtInt _lowRangeInt;
  AmtInt _highRangeInt;
  AmtInt _discPercentInt;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _specialProcessNo);
    FLATTENIZE(archive, _taxNodec);
    FLATTENIZE(archive, _taxcdRoundUnitNodec);
    FLATTENIZE(archive, _fareRangeNodec);
    FLATTENIZE(archive, _discPercentNodec);
    FLATTENIZE(archive, _discPercent);
    FLATTENIZE(archive, _taxAmt);
    FLATTENIZE(archive, _minTax);
    FLATTENIZE(archive, _maxTax);
    FLATTENIZE(archive, _plusupAmt);
    FLATTENIZE(archive, _lowRange);
    FLATTENIZE(archive, _highRange);
    FLATTENIZE(archive, _rangeincrement);
    FLATTENIZE(archive, _taxcdRoundUnit);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _firstTvlDate);
    FLATTENIZE(archive, _lastTvlDate);
    FLATTENIZE(archive, _modDate);
    FLATTENIZE(archive, _loc1ExclInd);
    FLATTENIZE(archive, _loc1Type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2ExclInd);
    FLATTENIZE(archive, _loc2Type);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _taxType);
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _taxcdRoundRule);
    FLATTENIZE(archive, _taxfullFareInd);
    FLATTENIZE(archive, _taxequivAmtInd);
    FLATTENIZE(archive, _taxexcessbagInd);
    FLATTENIZE(archive, _tvlDateasoriginInd);
    FLATTENIZE(archive, _displayonlyInd);
    FLATTENIZE(archive, _feeInd);
    FLATTENIZE(archive, _interlinableTaxInd);
    FLATTENIZE(archive, _showseparateInd);
    FLATTENIZE(archive, _posExclInd);
    FLATTENIZE(archive, _posLocType);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _poiExclInd);
    FLATTENIZE(archive, _poiLocType);
    FLATTENIZE(archive, _poiLoc);
    FLATTENIZE(archive, _sellCurExclInd);
    FLATTENIZE(archive, _sellCur);
    FLATTENIZE(archive, _occurrence);
    FLATTENIZE(archive, _freeTktexempt);
    FLATTENIZE(archive, _idTvlexempt);
    FLATTENIZE(archive, _rangeType);
    FLATTENIZE(archive, _rangeInd);
    FLATTENIZE(archive, _nextstopoverrestr);
    FLATTENIZE(archive, _spclTaxRounding);
    FLATTENIZE(archive, _taxCur);
    FLATTENIZE(archive, _taxCurNodec);
    FLATTENIZE(archive, _fareclassExclInd);
    FLATTENIZE(archive, _tktdsgExclInd);
    FLATTENIZE(archive, _valcxrExclInd);
    FLATTENIZE(archive, _exempequipExclInd);
    FLATTENIZE(archive, _psgrExclInd);
    FLATTENIZE(archive, _exempcxrExclInd);
    FLATTENIZE(archive, _fareTypeExclInd);
    FLATTENIZE(archive, _multioccconvrndInd);
    FLATTENIZE(archive, _originLocType);
    FLATTENIZE(archive, _originLoc);
    FLATTENIZE(archive, _originLocExclInd);
    FLATTENIZE(archive, _versioninheritedInd);
    FLATTENIZE(archive, _versiondisplayInd);
    FLATTENIZE(archive, _loc1Appl);
    FLATTENIZE(archive, _loc2Appl);
    FLATTENIZE(archive, _tripType);
    FLATTENIZE(archive, _travelType);
    FLATTENIZE(archive, _itineraryType);
    FLATTENIZE(archive, _formOfPayment);
    FLATTENIZE(archive, _taxOnTaxExcl);
    FLATTENIZE(archive, _feeApplInd);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _bookingCode3);
    FLATTENIZE(archive, _restrictionValidationCxr);
    FLATTENIZE(archive, _restrictionPsg);
    FLATTENIZE(archive, _restrictionFareType);
    FLATTENIZE(archive, _restrictionFareClass);
    FLATTENIZE(archive, _equipmentCode);
    FLATTENIZE(archive, _exemptionCxr);
    FLATTENIZE(archive, _taxOnTaxCode);
    FLATTENIZE(archive, _restrictionTransit);
    FLATTENIZE(archive, _taxCodeGenTexts);
    FLATTENIZE(archive, _cabins);
    FLATTENIZE(archive, _taxRestrTktDsgs);
    FLATTENIZE(archive, _taxAmtInt);
    FLATTENIZE(archive, _minTaxInt);
    FLATTENIZE(archive, _maxTaxInt);
    FLATTENIZE(archive, _taxcdRoundUnitInt);
    FLATTENIZE(archive, _lowRangeInt);
    FLATTENIZE(archive, _highRangeInt);
    FLATTENIZE(archive, _discPercentInt);
    FLATTENIZE(archive, _taxRestrictionLocNo);
    FLATTENIZE(archive, _specConfigName);
  }

protected:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_taxCode & ptr->_versionDate & ptr->_seqNo & ptr->_createDate &
           ptr->_expireDate & ptr->_lockDate & ptr->_specialProcessNo & ptr->_taxNodec &
           ptr->_taxcdRoundUnitNodec & ptr->_fareRangeNodec & ptr->_discPercentNodec &
           ptr->_discPercent & ptr->_taxAmt & ptr->_minTax & ptr->_maxTax & ptr->_plusupAmt &
           ptr->_lowRange & ptr->_highRange & ptr->_rangeincrement & ptr->_taxcdRoundUnit &
           ptr->_effDate & ptr->_discDate & ptr->_firstTvlDate & ptr->_lastTvlDate & ptr->_modDate &
           ptr->_loc1ExclInd & ptr->_loc1Type & ptr->_loc1 & ptr->_loc2ExclInd & ptr->_loc2Type &
           ptr->_loc2 & ptr->_taxType & ptr->_nation & ptr->_taxcdRoundRule & ptr->_taxfullFareInd &
           ptr->_taxequivAmtInd & ptr->_taxexcessbagInd & ptr->_tvlDateasoriginInd &
           ptr->_displayonlyInd & ptr->_feeInd & ptr->_interlinableTaxInd & ptr->_showseparateInd &
           ptr->_posExclInd & ptr->_posLocType & ptr->_posLoc & ptr->_poiExclInd &
           ptr->_poiLocType & ptr->_poiLoc & ptr->_sellCurExclInd & ptr->_sellCur &
           ptr->_occurrence & ptr->_freeTktexempt & ptr->_idTvlexempt & ptr->_rangeType &
           ptr->_rangeInd & ptr->_nextstopoverrestr & ptr->_spclTaxRounding & ptr->_taxCur &
           ptr->_taxCurNodec & ptr->_fareclassExclInd & ptr->_tktdsgExclInd & ptr->_valcxrExclInd &
           ptr->_exempequipExclInd & ptr->_psgrExclInd & ptr->_exempcxrExclInd &
           ptr->_fareTypeExclInd & ptr->_multioccconvrndInd & ptr->_originLocType &
           ptr->_originLoc & ptr->_originLocExclInd & ptr->_versioninheritedInd &
           ptr->_versiondisplayInd & ptr->_loc1Appl & ptr->_loc2Appl & ptr->_tripType &
           ptr->_travelType & ptr->_itineraryType & ptr->_formOfPayment & ptr->_taxOnTaxExcl &
           ptr->_feeApplInd & ptr->_bookingCode1 & ptr->_bookingCode2 & ptr->_bookingCode3 &
           ptr->_restrictionValidationCxr & ptr->_restrictionPsg & ptr->_restrictionFareType &
           ptr->_restrictionFareClass & ptr->_equipmentCode & ptr->_exemptionCxr &
           ptr->_taxOnTaxCode & ptr->_restrictionTransit & ptr->_taxCodeGenTexts & ptr->_cabins &
           ptr->_taxRestrTktDsgs & ptr->_taxAmtInt & ptr->_minTaxInt & ptr->_maxTaxInt &
           ptr->_taxcdRoundUnitInt & ptr->_lowRangeInt & ptr->_highRangeInt & ptr->_discPercentInt &
           ptr->_taxRestrictionLocNo & ptr->_specConfigName;
  }

private:
  TaxCodeReg(const TaxCodeReg&);
  TaxCodeReg& operator=(const TaxCodeReg&);
};
}
