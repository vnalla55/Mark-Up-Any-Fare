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
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

#include <vector>

namespace tse
{

class RuleApplication : public RuleItemInfo
{
public:
  RuleApplication()
    : _itemNo(0),
      _validityInd(' '),
      _inhibit(' '),
      _unavailTag(' '),
      _serviceFirst(' '),
      _serviceBus(' '),
      _serviceEcon(' '),
      _serviceCoach(' '),
      _serviceOffPeak(' '),
      _serviceSuper(' '),
      _oneWayInd(' '),
      _roundTripInd(' '),
      _journeyOneWayInd(' '),
      _journeyRoundTripInd(' '),
      _journeyCircleTripInd(' '),
      _journeyOpenJawInd(' '),
      _journeySingleOpenJawInd(' '),
      _journeySOJAtOriginInd(' '),
      _journeySOJTurnaroundInd(' '),
      _journeyDOJawInd(' '),
      _journeyRTWInd(' '),
      _jointTransInd(' '),
      _inclusiveToursInd(' '),
      _groupsInd(' '),
      _textTblItemNoAdditionalFare(0),
      _purchaseInd(' '),
      _intermediatePointsInd(' '),
      _capacityInd(' '),
      _capacityTextTblItemNo(0),
      _rulesNotApplTextTblItemNo(0),
      _otherTextTblItemNo(0),
      _overrideDateTblItemNo(0),
      _textTblItemNo(0),
      _segCnt(0)
  {
  }

  virtual ~RuleApplication() {}

  class RuleApplSeg
  {
  public:
    RuleApplSeg() : _orderNo(0), _applInd(' ') {}

    virtual ~RuleApplSeg() {}

    int& orderNo() { return _orderNo; }
    const int& orderNo() const { return _orderNo; }

    Indicator& applInd() { return _applInd; }
    const Indicator& applInd() const { return _applInd; }

    LocKey& loc() { return _loc; }
    const LocKey& loc() const { return _loc; }

    bool operator==(const RuleApplSeg& rhs) const
    {
      return ((_orderNo == rhs._orderNo) && (_applInd == rhs._applInd) && (_loc == rhs._loc));
    }

    static void dummyData(RuleApplSeg& obj)
    {
      obj._orderNo = 1;
      obj._applInd = 'A';
      LocKey::dummyData(obj._loc);
    }

  private:
    int _orderNo;
    Indicator _applInd;
    LocKey _loc;

  public:
    virtual void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _orderNo);
      FLATTENIZE(archive, _applInd);
      FLATTENIZE(archive, _loc);
    }
  };

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  std::string& applTitle() { return _applTitle; }
  const std::string& applTitle() const { return _applTitle; }

  Indicator& serviceFirst() { return _serviceFirst; }
  const Indicator& serviceFirst() const { return _serviceFirst; }

  Indicator& serviceBus() { return _serviceBus; }
  const Indicator& serviceBus() const { return _serviceBus; }

  Indicator& serviceEcon() { return _serviceEcon; }
  const Indicator& serviceEcon() const { return _serviceEcon; }

  Indicator& serviceCoach() { return _serviceCoach; }
  const Indicator& serviceCoach() const { return _serviceCoach; }

  Indicator& serviceOffPeak() { return _serviceOffPeak; }
  const Indicator& serviceOffPeak() const { return _serviceOffPeak; }

  Indicator& serviceSuper() { return _serviceSuper; }
  const Indicator& serviceSuper() const { return _serviceSuper; }

  Indicator& oneWayInd() { return _oneWayInd; }
  const Indicator& oneWayInd() const { return _oneWayInd; }

  Indicator& roundTripInd() { return _roundTripInd; }
  const Indicator& roundTripInd() const { return _roundTripInd; }

  Indicator& journeyOneWayInd() { return _journeyOneWayInd; }
  const Indicator& journeyOneWayInd() const { return _journeyOneWayInd; }

  Indicator& journeyRoundTripInd() { return _journeyRoundTripInd; }
  const Indicator& journeyRoundTripInd() const { return _journeyRoundTripInd; }

  Indicator& journeyCircleTripInd() { return _journeyCircleTripInd; }
  const Indicator& journeyCircleTripInd() const { return _journeyCircleTripInd; }

  Indicator& journeyOpenJawInd() { return _journeyOpenJawInd; }
  const Indicator& journeyOpenJawInd() const { return _journeyOpenJawInd; }

  Indicator& journeySingleOpenJawInd() { return _journeySingleOpenJawInd; }
  const Indicator& journeySingleOpenJawInd() const { return _journeySingleOpenJawInd; }

  Indicator& journeySOJAtOriginInd() { return _journeySOJAtOriginInd; }
  const Indicator& journeySOJAtOriginInd() const { return _journeySOJAtOriginInd; }

  Indicator& journeySOJTurnaroundInd() { return _journeySOJTurnaroundInd; }
  const Indicator& journeySOJTurnaroundInd() const { return _journeySOJTurnaroundInd; }

  Indicator& journeyDOJawInd() { return _journeyDOJawInd; }
  const Indicator& journeyDOJawInd() const { return _journeyDOJawInd; }

  Indicator& journeyRTWInd() { return _journeyRTWInd; }
  const Indicator& journeyRTWInd() const { return _journeyRTWInd; }

  Indicator& jointTransInd() { return _jointTransInd; }
  const Indicator& jointTransInd() const { return _jointTransInd; }

  Indicator& inclusiveToursInd() { return _inclusiveToursInd; }
  const Indicator& inclusiveToursInd() const { return _inclusiveToursInd; }

  Indicator& groupsInd() { return _groupsInd; }
  const Indicator& groupsInd() const { return _groupsInd; }

  int& textTblItemNoAdditionalFare() { return _textTblItemNoAdditionalFare; }
  const int& textTblItemNoAdditionalFare() const { return _textTblItemNoAdditionalFare; }

  Indicator& purchaseInd() { return _purchaseInd; }
  const Indicator& purchaseInd() const { return _purchaseInd; }

  Indicator& intermediatePointsInd() { return _intermediatePointsInd; }
  const Indicator& intermediatePointsInd() const { return _intermediatePointsInd; }

  Indicator& capacityInd() { return _capacityInd; }
  const Indicator& capacityInd() const { return _capacityInd; }

  int& capacityTextTblItemNo() { return _capacityTextTblItemNo; }
  const int& capacityTextTblItemNo() const { return _capacityTextTblItemNo; }

  int& rulesNotApplTextTblItemNo() { return _rulesNotApplTextTblItemNo; }
  const int& rulesNotApplTextTblItemNo() const { return _rulesNotApplTextTblItemNo; }

  int& otherTextTblItemNo() { return _otherTextTblItemNo; }
  const int& otherTextTblItemNo() const { return _otherTextTblItemNo; }

  int& overrideDateTblItemNo() { return _overrideDateTblItemNo; }
  const int& overrideDateTblItemNo() const { return _overrideDateTblItemNo; }

  int& textTblItemNo() { return _textTblItemNo; }
  const int& textTblItemNo() const { return _textTblItemNo; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  std::vector<RuleApplSeg*>& segs() { return _segs; }
  const std::vector<RuleApplSeg*>& segs() const { return _segs; }

  virtual bool operator==(const RuleApplication& rhs) const
  {
    bool eq((RuleItemInfo::operator==(rhs)) && (_vendor == rhs._vendor) &&
            (_itemNo == rhs._itemNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_validityInd == rhs._validityInd) &&
            (_inhibit == rhs._inhibit) && (_unavailTag == rhs._unavailTag) &&
            (_applTitle == rhs._applTitle) && (_serviceFirst == rhs._serviceFirst) &&
            (_serviceBus == rhs._serviceBus) && (_serviceEcon == rhs._serviceEcon) &&
            (_serviceCoach == rhs._serviceCoach) && (_serviceOffPeak == rhs._serviceOffPeak) &&
            (_serviceSuper == rhs._serviceSuper) && (_oneWayInd == rhs._oneWayInd) &&
            (_roundTripInd == rhs._roundTripInd) && (_journeyOneWayInd == rhs._journeyOneWayInd) &&
            (_journeyRoundTripInd == rhs._journeyRoundTripInd) &&
            (_journeyCircleTripInd == rhs._journeyCircleTripInd) &&
            (_journeyOpenJawInd == rhs._journeyOpenJawInd) &&
            (_journeySingleOpenJawInd == rhs._journeySingleOpenJawInd) &&
            (_journeySOJAtOriginInd == rhs._journeySOJAtOriginInd) &&
            (_journeySOJTurnaroundInd == rhs._journeySOJTurnaroundInd) &&
            (_journeyDOJawInd == rhs._journeyDOJawInd) && (_journeyRTWInd == rhs._journeyRTWInd) &&
            (_jointTransInd == rhs._jointTransInd) &&
            (_inclusiveToursInd == rhs._inclusiveToursInd) && (_groupsInd == rhs._groupsInd) &&
            (_textTblItemNoAdditionalFare == rhs._textTblItemNoAdditionalFare) &&
            (_purchaseInd == rhs._purchaseInd) &&
            (_intermediatePointsInd == rhs._intermediatePointsInd) &&
            (_capacityInd == rhs._capacityInd) &&
            (_capacityTextTblItemNo == rhs._capacityTextTblItemNo) &&
            (_rulesNotApplTextTblItemNo == rhs._rulesNotApplTextTblItemNo) &&
            (_otherTextTblItemNo == rhs._otherTextTblItemNo) &&
            (_overrideDateTblItemNo == rhs._overrideDateTblItemNo) &&
            (_textTblItemNo == rhs._textTblItemNo) && (_segCnt == rhs._segCnt) &&
            (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(RuleApplication& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'E';
    obj._inhibit = 'F';
    obj._unavailTag = 'G';
    obj._applTitle = "aaaaaaaa";
    obj._serviceFirst = 'H';
    obj._serviceBus = 'I';
    obj._serviceEcon = 'J';
    obj._serviceCoach = 'K';
    obj._serviceOffPeak = 'L';
    obj._serviceSuper = 'M';
    obj._oneWayInd = 'N';
    obj._roundTripInd = 'O';
    obj._journeyOneWayInd = 'P';
    obj._journeyRoundTripInd = 'Q';
    obj._journeyCircleTripInd = 'R';
    obj._journeyOpenJawInd = 'S';
    obj._journeySingleOpenJawInd = 'T';
    obj._journeySOJAtOriginInd = 'U';
    obj._journeySOJTurnaroundInd = 'V';
    obj._journeyDOJawInd = 'W';
    obj._journeyRTWInd = 'X';
    obj._jointTransInd = 'Y';
    obj._inclusiveToursInd = 'Z';
    obj._groupsInd = 'a';
    obj._textTblItemNoAdditionalFare = 2;
    obj._purchaseInd = 'b';
    obj._intermediatePointsInd = 'c';
    obj._capacityInd = 'd';
    obj._capacityTextTblItemNo = 3;
    obj._rulesNotApplTextTblItemNo = 4;
    obj._otherTextTblItemNo = 5;
    obj._overrideDateTblItemNo = 6;
    obj._textTblItemNo = 7;
    obj._segCnt = 8;

    RuleApplSeg* ras1 = new RuleApplSeg;
    RuleApplSeg* ras2 = new RuleApplSeg;

    RuleApplSeg::dummyData(*ras1);
    RuleApplSeg::dummyData(*ras2);

    obj._segs.push_back(ras1);
    obj._segs.push_back(ras2);
  }

private:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailTag;
  std::string _applTitle;
  Indicator _serviceFirst;
  Indicator _serviceBus;
  Indicator _serviceEcon;
  Indicator _serviceCoach;
  Indicator _serviceOffPeak;
  Indicator _serviceSuper;
  Indicator _oneWayInd;
  Indicator _roundTripInd;
  Indicator _journeyOneWayInd;
  Indicator _journeyRoundTripInd;
  Indicator _journeyCircleTripInd;
  Indicator _journeyOpenJawInd;
  Indicator _journeySingleOpenJawInd;
  Indicator _journeySOJAtOriginInd;
  Indicator _journeySOJTurnaroundInd;
  Indicator _journeyDOJawInd;
  Indicator _journeyRTWInd;
  Indicator _jointTransInd;
  Indicator _inclusiveToursInd;
  Indicator _groupsInd;
  int _textTblItemNoAdditionalFare;
  Indicator _purchaseInd;
  Indicator _intermediatePointsInd;
  Indicator _capacityInd;
  int _capacityTextTblItemNo;
  int _rulesNotApplTextTblItemNo;
  int _otherTextTblItemNo;
  int _overrideDateTblItemNo;
  int _textTblItemNo;
  int _segCnt;
  std::vector<RuleApplSeg*> _segs;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _applTitle);
    FLATTENIZE(archive, _serviceFirst);
    FLATTENIZE(archive, _serviceBus);
    FLATTENIZE(archive, _serviceEcon);
    FLATTENIZE(archive, _serviceCoach);
    FLATTENIZE(archive, _serviceOffPeak);
    FLATTENIZE(archive, _serviceSuper);
    FLATTENIZE(archive, _oneWayInd);
    FLATTENIZE(archive, _roundTripInd);
    FLATTENIZE(archive, _journeyOneWayInd);
    FLATTENIZE(archive, _journeyRoundTripInd);
    FLATTENIZE(archive, _journeyCircleTripInd);
    FLATTENIZE(archive, _journeyOpenJawInd);
    FLATTENIZE(archive, _journeySingleOpenJawInd);
    FLATTENIZE(archive, _journeySOJAtOriginInd);
    FLATTENIZE(archive, _journeySOJTurnaroundInd);
    FLATTENIZE(archive, _journeyDOJawInd);
    FLATTENIZE(archive, _journeyRTWInd);
    FLATTENIZE(archive, _jointTransInd);
    FLATTENIZE(archive, _inclusiveToursInd);
    FLATTENIZE(archive, _groupsInd);
    FLATTENIZE(archive, _textTblItemNoAdditionalFare);
    FLATTENIZE(archive, _purchaseInd);
    FLATTENIZE(archive, _intermediatePointsInd);
    FLATTENIZE(archive, _capacityInd);
    FLATTENIZE(archive, _capacityTextTblItemNo);
    FLATTENIZE(archive, _rulesNotApplTextTblItemNo);
    FLATTENIZE(archive, _otherTextTblItemNo);
    FLATTENIZE(archive, _overrideDateTblItemNo);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _segs);
  }
};

} // namespace tse

