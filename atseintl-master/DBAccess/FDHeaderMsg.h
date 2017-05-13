//----------------------------------------------------------------------------
//  File: FDHeaderMsg.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class FDHeaderMsg
{
public:
  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  std::string& userAppl() { return _userAppl; }
  const std::string& userAppl() const { return _userAppl; }

  PseudoCityType& pseudoCityType() { return _pseudoCityType; }
  const PseudoCityType& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCityCode() { return _pseudoCityCode; }
  const PseudoCityCode& pseudoCityCode() const { return _pseudoCityCode; }

  TJRGroup& ssgGroupNo() { return _ssgGroupNo; }
  const TJRGroup& ssgGroupNo() const { return _ssgGroupNo; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  uint64_t& seqNo() { return _seqNo; }
  const uint64_t& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& fareDisplayType() { return _fareDisplayType; }
  const Indicator& fareDisplayType() const { return _fareDisplayType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  std::string& routing1() { return _routing1; }
  const std::string& routing1() const { return _routing1; }

  std::string& routing2() { return _routing2; }
  const std::string& routing2() const { return _routing2; }

  std::string& inclusionCode() { return _inclusionCode; }
  const std::string& inclusionCode() const { return _inclusionCode; }

  std::string& messageType() { return _messageType; }
  const std::string& messageType() const { return _messageType; }

  uint64_t& msgItemNo() { return _msgItemNo; }
  const uint64_t& msgItemNo() const { return _msgItemNo; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  uint64_t& memoNo() { return _memoNo; }
  const uint64_t& memoNo() const { return _memoNo; }

  std::string& creatorBusinessUnit() { return _creatorBusinessUnit; }
  const std::string& creatorBusinessUnit() const { return _creatorBusinessUnit; }

  std::string& creatorId() { return _creatorId; }
  const std::string& creatorId() const { return _creatorId; }

  std::string& startPoint() { return _startPoint; }
  const std::string& startPoint() const { return _startPoint; }

  Indicator& exceptPosLoc() { return _exceptPosLoc; }
  const Indicator& exceptPosLoc() const { return _exceptPosLoc; }

  LocTypeCode& posLocType() { return _posLocType; }
  const LocTypeCode& posLocType() const { return _posLocType; }

  LocCode& posLoc() { return _posLoc; }
  const LocCode& posLoc() const { return _posLoc; }

  uint64_t& newSeqNo() { return _newSeqNo; }
  const uint64_t& newSeqNo() const { return _newSeqNo; }

  Indicator& versionInheritedInd() { return _versionInheritedInd; }
  const Indicator& versionInheritedInd() const { return _versionInheritedInd; }

  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  const Indicator& versionDisplayInd() const { return _versionDisplayInd; }

  int32_t& routingTariff() { return _routingTariff; }
  const int32_t& routingTariff() const { return _routingTariff; }

  Indicator& exceptCur() { return _exceptCur; }
  const Indicator& exceptCur() const { return _exceptCur; }

  Indicator& insideBufferZone() { return _insideBufferZone; }
  const Indicator& insideBufferZone() const { return _insideBufferZone; }

  Indicator& outsideBufferZone() { return _outsideBufferZone; }
  const Indicator& outsideBufferZone() const { return _outsideBufferZone; }

  Indicator& exceptLoc() { return _exceptLoc; }
  const Indicator& exceptLoc() const { return _exceptLoc; }

  std::string& text() { return _text; }
  const std::string& text() const { return _text; }

  int32_t& textSeqNo() { return _textSeqNo; }
  const int32_t& textSeqNo() const { return _textSeqNo; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  bool operator==(const FDHeaderMsg& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCityCode == rhs._pseudoCityCode) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_versionDate == rhs._versionDate) &&
            (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_fareDisplayType == rhs._fareDisplayType) &&
            (_carrier == rhs._carrier) && (_directionality == rhs._directionality) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_globalDir == rhs._globalDir) &&
            (_routing1 == rhs._routing1) && (_routing2 == rhs._routing2) &&
            (_inclusionCode == rhs._inclusionCode) && (_messageType == rhs._messageType) &&
            (_msgItemNo == rhs._msgItemNo) && (_lockDate == rhs._lockDate) &&
            (_memoNo == rhs._memoNo) && (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
            (_creatorId == rhs._creatorId) && (_startPoint == rhs._startPoint) &&
            (_exceptPosLoc == rhs._exceptPosLoc) && (_posLocType == rhs._posLocType) &&
            (_posLoc == rhs._posLoc) && (_newSeqNo == rhs._newSeqNo) &&
            (_versionInheritedInd == rhs._versionInheritedInd) &&
            (_versionDisplayInd == rhs._versionDisplayInd) &&
            (_routingTariff == rhs._routingTariff) && (_exceptCur == rhs._exceptCur) &&
            (_insideBufferZone == rhs._insideBufferZone) &&
            (_outsideBufferZone == rhs._outsideBufferZone) && (_exceptLoc == rhs._exceptLoc) &&
            (_text == rhs._text) && (_textSeqNo == rhs._textSeqNo) && (_cur == rhs._cur));
  }

  static void dummyData(FDHeaderMsg& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "aaaaaaaa";
    obj._pseudoCityType = 'B';
    obj._pseudoCityCode = 'C';
    obj._ssgGroupNo = 1;
    obj._versionDate = time(nullptr);
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._fareDisplayType = 'D';
    obj._carrier = "EFG";
    obj._directionality = TO;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._globalDir = GlobalDirection::US;
    obj._routing1 = "bbbbbbbb";
    obj._routing2 = "cccccccc";
    obj._inclusionCode = "dddddddd";
    obj._messageType = "eeeeeeee";
    obj._msgItemNo = 3;
    obj._lockDate = time(nullptr);
    obj._memoNo = 4;
    obj._creatorBusinessUnit = "ffffffff";
    obj._creatorId = "gggggggg";
    obj._startPoint = "hhhhhhhh";
    obj._exceptPosLoc = 'H';
    obj._posLocType = 'I';
    obj._posLoc = "JKLMNOPQ";
    obj._newSeqNo = 5;
    obj._versionInheritedInd = 'R';
    obj._versionDisplayInd = 'S';
    obj._routingTariff = 6;
    obj._exceptCur = 'T';
    obj._insideBufferZone = 'U';
    obj._outsideBufferZone = 'V';
    obj._exceptLoc = 'W';
    obj._text = "iiiiiiii";
    obj._textSeqNo = 7;
    obj._cur = "XYZ";
  }

private:
  Indicator _userApplType = ' '; // C1
  std::string _userAppl; // C4
  PseudoCityType _pseudoCityType = ' '; // C1
  PseudoCityCode _pseudoCityCode; // C4
  TJRGroup _ssgGroupNo = 0;
  DateTime _versionDate;
  uint64_t _seqNo = 0; // N11
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _fareDisplayType = ' '; // C1
  CarrierCode _carrier; // C3
  Directionality _directionality = Directionality::TERMINATE; // C1
  LocKey _loc1;
  LocKey _loc2;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR; // C2
  std::string _routing1; // C4
  std::string _routing2; // C4
  std::string _inclusionCode; // C4
  std::string _messageType; // C10
  uint64_t _msgItemNo = 0; // N11
  DateTime _lockDate;
  uint64_t _memoNo = 0; // N11
  std::string _creatorBusinessUnit; // C8
  std::string _creatorId; // C8
  std::string _startPoint; // C2
  Indicator _exceptPosLoc = ' '; // C1
  LocTypeCode _posLocType = ' '; // C1
  LocCode _posLoc; // C5
  uint64_t _newSeqNo = 0; // N11
  Indicator _versionInheritedInd = ' '; // C1
  Indicator _versionDisplayInd = ' '; // C1
  int32_t _routingTariff = 0; // SMALLINT
  Indicator _exceptCur = ' '; // C1
  Indicator _insideBufferZone = ' ';
  Indicator _outsideBufferZone = ' ';
  Indicator _exceptLoc = ' ';
  std::string _text; // Actual Header Message Text
  int32_t _textSeqNo = 0;
  CurrencyCode _cur; // C40

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCityCode);
    FLATTENIZE(archive, _ssgGroupNo);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _fareDisplayType);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _routing1);
    FLATTENIZE(archive, _routing2);
    FLATTENIZE(archive, _inclusionCode);
    FLATTENIZE(archive, _messageType);
    FLATTENIZE(archive, _msgItemNo);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _startPoint);
    FLATTENIZE(archive, _exceptPosLoc);
    FLATTENIZE(archive, _posLocType);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _newSeqNo);
    FLATTENIZE(archive, _versionInheritedInd);
    FLATTENIZE(archive, _versionDisplayInd);
    FLATTENIZE(archive, _routingTariff);
    FLATTENIZE(archive, _exceptCur);
    FLATTENIZE(archive, _insideBufferZone);
    FLATTENIZE(archive, _outsideBufferZone);
    FLATTENIZE(archive, _exceptLoc);
    FLATTENIZE(archive, _text);
    FLATTENIZE(archive, _textSeqNo);
    FLATTENIZE(archive, _cur);
  }
};

} // End of namespace tse

