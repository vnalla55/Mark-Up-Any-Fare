//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class Groups : public RuleItemInfo
{
public:
  Groups()
    : _fltMaxPercent(0),
      _childPercent(0),
      _addlPercent(0),
      _substPercent(0),
      _indvPercent(0),
      _indvGeoTblItemNoBetw(0),
      _indvGeoTblItemNoAnd(0),
      _assemblyGeoTblItemNo(0),
      _grpGeoTblItemNoBetw(0),
      _grpGeoTblItemNoAnd(0),
      _indvAmt1(0),
      _indvAmt2(0),
      _geoTblItemNoBetween(0),
      _tourGeoTblItemNoBetw(0),
      _tourGeoTblItemNoAnd(0),
      _geoTblItemNoAnd(0),
      _fltMaxPercentNoDec(0),
      _childNo(0),
      _childPercentNoDec(0),
      _nameTOD(0),
      _indvTOD(0),
      _addlPercentNoDec(0),
      _addlTOD(0),
      _substMaxNo(0),
      _substPercentNoDec(0),
      _substTOD(0),
      _minNoSectors(0),
      _addlTktTOD(0),
      _substTktTOD(0),
      _indvNoDec1(0),
      _indvNoDec2(0),
      _indvPercentNoDec(0),
      _tourMinNo(0),
      _tourMaxNo(0),
      _noTourConductors(0),
      _noPayingPsgrs(0),
      _maxNoTourConductors(0),
      _tourConductorMinAge(0),
      _assemblyMinNo(0),
      _validityInd(' '),
      _inhibit(' '),
      _unavailTag(' '),
      _fareClassGroupInd(' '),
      _locValidityInd(' '),
      _localeApplInd(' '),
      _childType(' '),
      _nameReq(' '),
      _nameAppl(' '),
      _itinerary(' '),
      _pnr(' '),
      _fareChargeInd(' '),
      _addlTktInd(' '),
      _substTktInd(' '),
      _tvlCondOut(' '),
      _tvlCondInb(' '),
      _tvlCondEither(' '),
      _grpTime(' '),
      _grpWaiver(' '),
      _indvPermitted(' '),
      _indvSame(' '),
      _indvInOutInd(' '),
      _assemblyReturnInd(' '),
      _tourConductorTvlInd(' '),
      _tourConductorDiffInd(' '),
      _tourConductorAppInd(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Percent& fltMaxPercent() { return _fltMaxPercent; }
  const Percent& fltMaxPercent() const { return _fltMaxPercent; }

  Percent& childPercent() { return _childPercent; }
  const Percent& childPercent() const { return _childPercent; }

  Percent& addlPercent() { return _addlPercent; }
  const Percent& addlPercent() const { return _addlPercent; }

  Percent& substPercent() { return _substPercent; }
  const Percent& substPercent() const { return _substPercent; }

  Percent& indvPercent() { return _indvPercent; }
  const Percent& indvPercent() const { return _indvPercent; }

  DateTime& nameDate() { return _nameDate; }
  const DateTime& nameDate() const { return _nameDate; }

  int& indvGeoTblItemNoBetw() { return _indvGeoTblItemNoBetw; }
  const int& indvGeoTblItemNoBetw() const { return _indvGeoTblItemNoBetw; }

  int& indvGeoTblItemNoAnd() { return _indvGeoTblItemNoAnd; }
  const int& indvGeoTblItemNoAnd() const { return _indvGeoTblItemNoAnd; }

  int& assemblyGeoTblItemNo() { return _assemblyGeoTblItemNo; }
  const int& assemblyGeoTblItemNo() const { return _assemblyGeoTblItemNo; }

  int& grpGeoTblItemNoBetw() { return _grpGeoTblItemNoBetw; }
  const int& grpGeoTblItemNoBetw() const { return _grpGeoTblItemNoBetw; }

  int& grpGeoTblItemNoAnd() { return _grpGeoTblItemNoAnd; }
  const int& grpGeoTblItemNoAnd() const { return _grpGeoTblItemNoAnd; }

  MoneyAmount& indvAmt1() { return _indvAmt1; }
  const MoneyAmount& indvAmt1() const { return _indvAmt1; }

  MoneyAmount& indvAmt2() { return _indvAmt2; }
  const MoneyAmount& indvAmt2() const { return _indvAmt2; }

  int& geoTblItemNoBetween() { return _geoTblItemNoBetween; }
  const int& geoTblItemNoBetween() const { return _geoTblItemNoBetween; }

  int& tourGeoTblItemNoBetw() { return _tourGeoTblItemNoBetw; }
  const int& tourGeoTblItemNoBetw() const { return _tourGeoTblItemNoBetw; }

  int& tourGeoTblItemNoAnd() { return _tourGeoTblItemNoAnd; }
  const int& tourGeoTblItemNoAnd() const { return _tourGeoTblItemNoAnd; }

  int& geoTblItemNoAnd() { return _geoTblItemNoAnd; }
  const int& geoTblItemNoAnd() const { return _geoTblItemNoAnd; }

  int& fltMaxPercentNoDec() { return _fltMaxPercentNoDec; }
  const int& fltMaxPercentNoDec() const { return _fltMaxPercentNoDec; }

  int& childNo() { return _childNo; }
  const int& childNo() const { return _childNo; }

  int& childPercentNoDec() { return _childPercentNoDec; }
  const int& childPercentNoDec() const { return _childPercentNoDec; }

  int& nameTOD() { return _nameTOD; }
  const int& nameTOD() const { return _nameTOD; }

  int& indvTOD() { return _indvTOD; }
  const int& indvTOD() const { return _indvTOD; }

  int& addlPercentNoDec() { return _addlPercentNoDec; }
  const int& addlPercentNoDec() const { return _addlPercentNoDec; }

  int& addlTOD() { return _addlTOD; }
  const int& addlTOD() const { return _addlTOD; }

  int& substMaxNo() { return _substMaxNo; }
  const int& substMaxNo() const { return _substMaxNo; }

  int& substPercentNoDec() { return _substPercentNoDec; }
  const int& substPercentNoDec() const { return _substPercentNoDec; }

  int& substTOD() { return _substTOD; }
  const int& substTOD() const { return _substTOD; }

  int& minNoSectors() { return _minNoSectors; }
  const int& minNoSectors() const { return _minNoSectors; }

  int& addlTktTOD() { return _addlTktTOD; }
  const int& addlTktTOD() const { return _addlTktTOD; }

  int& substTktTOD() { return _substTktTOD; }
  const int& substTktTOD() const { return _substTktTOD; }

  int& indvNoDec1() { return _indvNoDec1; }
  const int& indvNoDec1() const { return _indvNoDec1; }

  int& indvNoDec2() { return _indvNoDec2; }
  const int& indvNoDec2() const { return _indvNoDec2; }

  int& indvPercentNoDec() { return _indvPercentNoDec; }
  const int& indvPercentNoDec() const { return _indvPercentNoDec; }

  int& tourMinNo() { return _tourMinNo; }
  const int& tourMinNo() const { return _tourMinNo; }

  int& tourMaxNo() { return _tourMaxNo; }
  const int& tourMaxNo() const { return _tourMaxNo; }

  int& noTourConductors() { return _noTourConductors; }
  const int& noTourConductors() const { return _noTourConductors; }

  int& noPayingPsgrs() { return _noPayingPsgrs; }
  const int& noPayingPsgrs() const { return _noPayingPsgrs; }

  int& maxNoTourConductors() { return _maxNoTourConductors; }
  const int& maxNoTourConductors() const { return _maxNoTourConductors; }

  int& tourConductorMinAge() { return _tourConductorMinAge; }
  const int& tourConductorMinAge() const { return _tourConductorMinAge; }

  int& assemblyMinNo() { return _assemblyMinNo; }
  const int& assemblyMinNo() const { return _assemblyMinNo; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  std::string& groupType() { return _groupType; }
  const std::string& groupType() const { return _groupType; }

  std::string& minGroupSize() { return _minGroupSize; }
  const std::string& minGroupSize() const { return _minGroupSize; }

  std::string& maxGroupSize() { return _maxGroupSize; }
  const std::string& maxGroupSize() const { return _maxGroupSize; }

  Indicator& fareClassGroupInd() { return _fareClassGroupInd; }
  const Indicator& fareClassGroupInd() const { return _fareClassGroupInd; }

  std::string& fltMaxNoPsgr() { return _fltMaxNoPsgr; }
  const std::string& fltMaxNoPsgr() const { return _fltMaxNoPsgr; }

  std::string& fltMaxNo() { return _fltMaxNo; }
  const std::string& fltMaxNo() const { return _fltMaxNo; }

  Indicator& locValidityInd() { return _locValidityInd; }
  const Indicator& locValidityInd() const { return _locValidityInd; }

  Indicator& localeApplInd() { return _localeApplInd; }
  const Indicator& localeApplInd() const { return _localeApplInd; }

  std::string& childFareClass() { return _childFareClass; }
  const std::string& childFareClass() const { return _childFareClass; }

  Indicator& childType() { return _childType; }
  const Indicator& childType() const { return _childType; }

  Indicator& nameReq() { return _nameReq; }
  const Indicator& nameReq() const { return _nameReq; }

  std::string& namePeriod() { return _namePeriod; }
  const std::string& namePeriod() const { return _namePeriod; }

  std::string& nameUnit() { return _nameUnit; }
  const std::string& nameUnit() const { return _nameUnit; }

  Indicator& nameAppl() { return _nameAppl; }
  const Indicator& nameAppl() const { return _nameAppl; }

  Indicator& itinerary() { return _itinerary; }
  const Indicator& itinerary() const { return _itinerary; }

  Indicator& pnr() { return _pnr; }
  const Indicator& pnr() const { return _pnr; }

  Indicator& fareChargeInd() { return _fareChargeInd; }
  const Indicator& fareChargeInd() const { return _fareChargeInd; }

  std::string& addlMaxNo() { return _addlMaxNo; }
  const std::string& addlMaxNo() const { return _addlMaxNo; }

  std::string& addlPeriod() { return _addlPeriod; }
  const std::string& addlPeriod() const { return _addlPeriod; }

  std::string& addlUnit() { return _addlUnit; }
  const std::string& addlUnit() const { return _addlUnit; }

  std::string& addlTktPeriod() { return _addlTktPeriod; }
  const std::string& addlTktPeriod() const { return _addlTktPeriod; }

  std::string& addlTktunit() { return _addlTktunit; }
  const std::string& addlTktunit() const { return _addlTktunit; }

  Indicator& addlTktInd() { return _addlTktInd; }
  const Indicator& addlTktInd() const { return _addlTktInd; }

  std::string& substPeriod() { return _substPeriod; }
  const std::string& substPeriod() const { return _substPeriod; }

  std::string& substUnit() { return _substUnit; }
  const std::string& substUnit() const { return _substUnit; }

  std::string& substTktPeriod() { return _substTktPeriod; }
  const std::string& substTktPeriod() const { return _substTktPeriod; }

  std::string& substTktUnit() { return _substTktUnit; }
  const std::string& substTktUnit() const { return _substTktUnit; }

  Indicator& substTktInd() { return _substTktInd; }
  const Indicator& substTktInd() const { return _substTktInd; }

  Indicator& tvlCondOut() { return _tvlCondOut; }
  const Indicator& tvlCondOut() const { return _tvlCondOut; }

  Indicator& tvlCondInb() { return _tvlCondInb; }
  const Indicator& tvlCondInb() const { return _tvlCondInb; }

  Indicator& tvlCondEither() { return _tvlCondEither; }
  const Indicator& tvlCondEither() const { return _tvlCondEither; }

  Indicator& grpTime() { return _grpTime; }
  const Indicator& grpTime() const { return _grpTime; }

  Indicator& grpWaiver() { return _grpWaiver; }
  const Indicator& grpWaiver() const { return _grpWaiver; }

  Indicator& indvPermitted() { return _indvPermitted; }
  const Indicator& indvPermitted() const { return _indvPermitted; }

  Indicator& indvSame() { return _indvSame; }
  const Indicator& indvSame() const { return _indvSame; }

  std::string& indvPeriod() { return _indvPeriod; }
  const std::string& indvPeriod() const { return _indvPeriod; }

  std::string& indvUnit() { return _indvUnit; }
  const std::string& indvUnit() const { return _indvUnit; }

  Indicator& indvInOutInd() { return _indvInOutInd; }
  const Indicator& indvInOutInd() const { return _indvInOutInd; }

  CurrencyCode& indvCurr1() { return _indvCurr1; }
  const CurrencyCode& indvCurr1() const { return _indvCurr1; }

  CurrencyCode& indvCurr2() { return _indvCurr2; }
  const CurrencyCode& indvCurr2() const { return _indvCurr2; }

  Indicator& assemblyReturnInd() { return _assemblyReturnInd; }
  const Indicator& assemblyReturnInd() const { return _assemblyReturnInd; }

  Indicator& tourConductorTvlInd() { return _tourConductorTvlInd; }
  const Indicator& tourConductorTvlInd() const { return _tourConductorTvlInd; }

  Indicator& tourConductorDiffInd() { return _tourConductorDiffInd; }
  const Indicator& tourConductorDiffInd() const { return _tourConductorDiffInd; }

  Indicator& tourConductorAppInd() { return _tourConductorAppInd; }
  const Indicator& tourConductorAppInd() const { return _tourConductorAppInd; }

  bool operator==(const Groups& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_fltMaxPercent == rhs._fltMaxPercent) &&
        (_childPercent == rhs._childPercent) && (_addlPercent == rhs._addlPercent) &&
        (_substPercent == rhs._substPercent) && (_indvPercent == rhs._indvPercent) &&
        (_nameDate == rhs._nameDate) && (_indvGeoTblItemNoBetw == rhs._indvGeoTblItemNoBetw) &&
        (_indvGeoTblItemNoAnd == rhs._indvGeoTblItemNoAnd) &&
        (_assemblyGeoTblItemNo == rhs._assemblyGeoTblItemNo) &&
        (_grpGeoTblItemNoBetw == rhs._grpGeoTblItemNoBetw) &&
        (_grpGeoTblItemNoAnd == rhs._grpGeoTblItemNoAnd) && (_indvAmt1 == rhs._indvAmt1) &&
        (_indvAmt2 == rhs._indvAmt2) && (_geoTblItemNoBetween == rhs._geoTblItemNoBetween) &&
        (_tourGeoTblItemNoBetw == rhs._tourGeoTblItemNoBetw) &&
        (_tourGeoTblItemNoAnd == rhs._tourGeoTblItemNoAnd) &&
        (_geoTblItemNoAnd == rhs._geoTblItemNoAnd) &&
        (_fltMaxPercentNoDec == rhs._fltMaxPercentNoDec) && (_childNo == rhs._childNo) &&
        (_childPercentNoDec == rhs._childPercentNoDec) && (_nameTOD == rhs._nameTOD) &&
        (_indvTOD == rhs._indvTOD) && (_addlPercentNoDec == rhs._addlPercentNoDec) &&
        (_addlTOD == rhs._addlTOD) && (_substMaxNo == rhs._substMaxNo) &&
        (_substPercentNoDec == rhs._substPercentNoDec) && (_substTOD == rhs._substTOD) &&
        (_minNoSectors == rhs._minNoSectors) && (_addlTktTOD == rhs._addlTktTOD) &&
        (_substTktTOD == rhs._substTktTOD) && (_indvNoDec1 == rhs._indvNoDec1) &&
        (_indvNoDec2 == rhs._indvNoDec2) && (_indvPercentNoDec == rhs._indvPercentNoDec) &&
        (_tourMinNo == rhs._tourMinNo) && (_tourMaxNo == rhs._tourMaxNo) &&
        (_noTourConductors == rhs._noTourConductors) && (_noPayingPsgrs == rhs._noPayingPsgrs) &&
        (_maxNoTourConductors == rhs._maxNoTourConductors) &&
        (_tourConductorMinAge == rhs._tourConductorMinAge) &&
        (_assemblyMinNo == rhs._assemblyMinNo) && (_validityInd == rhs._validityInd) &&
        (_inhibit == rhs._inhibit) && (_unavailTag == rhs._unavailTag) &&
        (_groupType == rhs._groupType) && (_minGroupSize == rhs._minGroupSize) &&
        (_maxGroupSize == rhs._maxGroupSize) && (_fareClassGroupInd == rhs._fareClassGroupInd) &&
        (_fltMaxNoPsgr == rhs._fltMaxNoPsgr) && (_fltMaxNo == rhs._fltMaxNo) &&
        (_locValidityInd == rhs._locValidityInd) && (_localeApplInd == rhs._localeApplInd) &&
        (_childFareClass == rhs._childFareClass) && (_childType == rhs._childType) &&
        (_nameReq == rhs._nameReq) && (_namePeriod == rhs._namePeriod) &&
        (_nameUnit == rhs._nameUnit) && (_nameAppl == rhs._nameAppl) &&
        (_itinerary == rhs._itinerary) && (_pnr == rhs._pnr) &&
        (_fareChargeInd == rhs._fareChargeInd) && (_addlMaxNo == rhs._addlMaxNo) &&
        (_addlPeriod == rhs._addlPeriod) && (_addlUnit == rhs._addlUnit) &&
        (_addlTktPeriod == rhs._addlTktPeriod) && (_addlTktunit == rhs._addlTktunit) &&
        (_addlTktInd == rhs._addlTktInd) && (_substPeriod == rhs._substPeriod) &&
        (_substUnit == rhs._substUnit) && (_substTktPeriod == rhs._substTktPeriod) &&
        (_substTktUnit == rhs._substTktUnit) && (_substTktInd == rhs._substTktInd) &&
        (_tvlCondOut == rhs._tvlCondOut) && (_tvlCondInb == rhs._tvlCondInb) &&
        (_tvlCondEither == rhs._tvlCondEither) && (_grpTime == rhs._grpTime) &&
        (_grpWaiver == rhs._grpWaiver) && (_indvPermitted == rhs._indvPermitted) &&
        (_indvSame == rhs._indvSame) && (_indvPeriod == rhs._indvPeriod) &&
        (_indvUnit == rhs._indvUnit) && (_indvInOutInd == rhs._indvInOutInd) &&
        (_indvCurr1 == rhs._indvCurr1) && (_indvCurr2 == rhs._indvCurr2) &&
        (_assemblyReturnInd == rhs._assemblyReturnInd) &&
        (_tourConductorTvlInd == rhs._tourConductorTvlInd) &&
        (_tourConductorDiffInd == rhs._tourConductorDiffInd) &&
        (_tourConductorAppInd == rhs._tourConductorAppInd));
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  Percent _fltMaxPercent;
  Percent _childPercent;
  Percent _addlPercent;
  Percent _substPercent;
  Percent _indvPercent;
  DateTime _nameDate;
  int _indvGeoTblItemNoBetw;
  int _indvGeoTblItemNoAnd;
  int _assemblyGeoTblItemNo;
  int _grpGeoTblItemNoBetw;
  int _grpGeoTblItemNoAnd;
  MoneyAmount _indvAmt1;
  MoneyAmount _indvAmt2;
  int _geoTblItemNoBetween;
  int _tourGeoTblItemNoBetw;
  int _tourGeoTblItemNoAnd;
  int _geoTblItemNoAnd;
  int _fltMaxPercentNoDec;
  int _childNo;
  int _childPercentNoDec;
  int _nameTOD;
  int _indvTOD;
  int _addlPercentNoDec;
  int _addlTOD;
  int _substMaxNo;
  int _substPercentNoDec;
  int _substTOD;
  int _minNoSectors;
  int _addlTktTOD;
  int _substTktTOD;
  int _indvNoDec1;
  int _indvNoDec2;
  int _indvPercentNoDec;
  int _tourMinNo;
  int _tourMaxNo;
  int _noTourConductors;
  int _noPayingPsgrs;
  int _maxNoTourConductors;
  int _tourConductorMinAge;
  int _assemblyMinNo;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailTag;
  std::string _groupType;
  std::string _minGroupSize;
  std::string _maxGroupSize;
  Indicator _fareClassGroupInd;
  std::string _fltMaxNoPsgr;
  std::string _fltMaxNo;
  Indicator _locValidityInd;
  Indicator _localeApplInd;
  std::string _childFareClass;
  Indicator _childType;
  Indicator _nameReq;
  std::string _namePeriod;
  std::string _nameUnit;
  Indicator _nameAppl;
  Indicator _itinerary;
  Indicator _pnr;
  Indicator _fareChargeInd;
  std::string _addlMaxNo;
  std::string _addlPeriod;
  std::string _addlUnit;
  std::string _addlTktPeriod;
  std::string _addlTktunit;
  Indicator _addlTktInd;
  std::string _substPeriod;
  std::string _substUnit;
  std::string _substTktPeriod;
  std::string _substTktUnit;
  Indicator _substTktInd;
  Indicator _tvlCondOut;
  Indicator _tvlCondInb;
  Indicator _tvlCondEither;
  Indicator _grpTime;
  Indicator _grpWaiver;
  Indicator _indvPermitted;
  Indicator _indvSame;
  std::string _indvPeriod;
  std::string _indvUnit;
  Indicator _indvInOutInd;
  CurrencyCode _indvCurr1;
  CurrencyCode _indvCurr2;
  Indicator _assemblyReturnInd;
  Indicator _tourConductorTvlInd;
  Indicator _tourConductorDiffInd;
  Indicator _tourConductorAppInd;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fltMaxPercent);
    FLATTENIZE(archive, _childPercent);
    FLATTENIZE(archive, _addlPercent);
    FLATTENIZE(archive, _substPercent);
    FLATTENIZE(archive, _indvPercent);
    FLATTENIZE(archive, _nameDate);
    FLATTENIZE(archive, _indvGeoTblItemNoBetw);
    FLATTENIZE(archive, _indvGeoTblItemNoAnd);
    FLATTENIZE(archive, _assemblyGeoTblItemNo);
    FLATTENIZE(archive, _grpGeoTblItemNoBetw);
    FLATTENIZE(archive, _grpGeoTblItemNoAnd);
    FLATTENIZE(archive, _indvAmt1);
    FLATTENIZE(archive, _indvAmt2);
    FLATTENIZE(archive, _geoTblItemNoBetween);
    FLATTENIZE(archive, _tourGeoTblItemNoBetw);
    FLATTENIZE(archive, _tourGeoTblItemNoAnd);
    FLATTENIZE(archive, _geoTblItemNoAnd);
    FLATTENIZE(archive, _fltMaxPercentNoDec);
    FLATTENIZE(archive, _childNo);
    FLATTENIZE(archive, _childPercentNoDec);
    FLATTENIZE(archive, _nameTOD);
    FLATTENIZE(archive, _indvTOD);
    FLATTENIZE(archive, _addlPercentNoDec);
    FLATTENIZE(archive, _addlTOD);
    FLATTENIZE(archive, _substMaxNo);
    FLATTENIZE(archive, _substPercentNoDec);
    FLATTENIZE(archive, _substTOD);
    FLATTENIZE(archive, _minNoSectors);
    FLATTENIZE(archive, _addlTktTOD);
    FLATTENIZE(archive, _substTktTOD);
    FLATTENIZE(archive, _indvNoDec1);
    FLATTENIZE(archive, _indvNoDec2);
    FLATTENIZE(archive, _indvPercentNoDec);
    FLATTENIZE(archive, _tourMinNo);
    FLATTENIZE(archive, _tourMaxNo);
    FLATTENIZE(archive, _noTourConductors);
    FLATTENIZE(archive, _noPayingPsgrs);
    FLATTENIZE(archive, _maxNoTourConductors);
    FLATTENIZE(archive, _tourConductorMinAge);
    FLATTENIZE(archive, _assemblyMinNo);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _groupType);
    FLATTENIZE(archive, _minGroupSize);
    FLATTENIZE(archive, _maxGroupSize);
    FLATTENIZE(archive, _fareClassGroupInd);
    FLATTENIZE(archive, _fltMaxNoPsgr);
    FLATTENIZE(archive, _fltMaxNo);
    FLATTENIZE(archive, _locValidityInd);
    FLATTENIZE(archive, _localeApplInd);
    FLATTENIZE(archive, _childFareClass);
    FLATTENIZE(archive, _childType);
    FLATTENIZE(archive, _nameReq);
    FLATTENIZE(archive, _namePeriod);
    FLATTENIZE(archive, _nameUnit);
    FLATTENIZE(archive, _nameAppl);
    FLATTENIZE(archive, _itinerary);
    FLATTENIZE(archive, _pnr);
    FLATTENIZE(archive, _fareChargeInd);
    FLATTENIZE(archive, _addlMaxNo);
    FLATTENIZE(archive, _addlPeriod);
    FLATTENIZE(archive, _addlUnit);
    FLATTENIZE(archive, _addlTktPeriod);
    FLATTENIZE(archive, _addlTktunit);
    FLATTENIZE(archive, _addlTktInd);
    FLATTENIZE(archive, _substPeriod);
    FLATTENIZE(archive, _substUnit);
    FLATTENIZE(archive, _substTktPeriod);
    FLATTENIZE(archive, _substTktUnit);
    FLATTENIZE(archive, _substTktInd);
    FLATTENIZE(archive, _tvlCondOut);
    FLATTENIZE(archive, _tvlCondInb);
    FLATTENIZE(archive, _tvlCondEither);
    FLATTENIZE(archive, _grpTime);
    FLATTENIZE(archive, _grpWaiver);
    FLATTENIZE(archive, _indvPermitted);
    FLATTENIZE(archive, _indvSame);
    FLATTENIZE(archive, _indvPeriod);
    FLATTENIZE(archive, _indvUnit);
    FLATTENIZE(archive, _indvInOutInd);
    FLATTENIZE(archive, _indvCurr1);
    FLATTENIZE(archive, _indvCurr2);
    FLATTENIZE(archive, _assemblyReturnInd);
    FLATTENIZE(archive, _tourConductorTvlInd);
    FLATTENIZE(archive, _tourConductorDiffInd);
    FLATTENIZE(archive, _tourConductorAppInd);
  }

};
}

