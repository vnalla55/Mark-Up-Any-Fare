//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/StopoversInfoSeg.h"

namespace tse
{

class StopoversInfo : public RuleItemInfo
{
public:
  StopoversInfo()
    : _unavailTag(' '),
      _geoTblItemNoBtw(0),
      _geoTblItemNoAnd(0),
      _geoTblItemNoGateway(0),
      _samePointsTblItemNo(0),
      _charge1NoDec(0),
      _charge1FirstAmt(0),
      _charge1AddAmt(0),
      _charge2NoDec(0),
      _charge2FirstAmt(0),
      _charge2AddAmt(0),
      _segCnt(0),
      _minStayTime(0),
      _maxStayTime(0),
      _samePntStops(0),
      _samePntTransit(0),
      _samePntConnections(0),
      _outOrReturnInd(' '),
      _sameCarrierInd(' '),
      _ojStopoverInd(' '),
      _ct2StopoverInd(' '),
      _ct2PlusStopoverInd(' '),
      _gtwyInd(' '),
      _minStayTimeUnit(' '),
      _maxStayTimeUnit(' '),
      _charge1Appl(' '),
      _charge1Total(' '),
      _charge2Appl(' '),
      _charge2Total(' '),
      _inhibit(' ')
  {
  }

  virtual ~StopoversInfo()
  { // Nuke the Kids!
    std::vector<StopoversInfoSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    {
      delete *SegIt;
    }
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& geoTblItemNoBtw() { return _geoTblItemNoBtw; }
  const int& geoTblItemNoBtw() const { return _geoTblItemNoBtw; }

  int& geoTblItemNoAnd() { return _geoTblItemNoAnd; }
  const int& geoTblItemNoAnd() const { return _geoTblItemNoAnd; }

  int& geoTblItemNoGateway() { return _geoTblItemNoGateway; }
  const int& geoTblItemNoGateway() const { return _geoTblItemNoGateway; }

  int& samePointsTblItemNo() { return _samePointsTblItemNo; }
  const int& samePointsTblItemNo() const { return _samePointsTblItemNo; }

  int& charge1NoDec() { return _charge1NoDec; }
  const int& charge1NoDec() const { return _charge1NoDec; }

  MoneyAmount& charge1FirstAmt() { return _charge1FirstAmt; }
  const MoneyAmount& charge1FirstAmt() const { return _charge1FirstAmt; }

  MoneyAmount& charge1AddAmt() { return _charge1AddAmt; }
  const MoneyAmount& charge1AddAmt() const { return _charge1AddAmt; }

  int& charge2NoDec() { return _charge2NoDec; }
  const int& charge2NoDec() const { return _charge2NoDec; }

  MoneyAmount& charge2FirstAmt() { return _charge2FirstAmt; }
  const MoneyAmount& charge2FirstAmt() const { return _charge2FirstAmt; }

  MoneyAmount& charge2AddAmt() { return _charge2AddAmt; }
  const MoneyAmount& charge2AddAmt() const { return _charge2AddAmt; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  int& minStayTime() { return _minStayTime; }
  const int& minStayTime() const { return _minStayTime; }

  int& maxStayTime() { return _maxStayTime; }
  const int& maxStayTime() const { return _maxStayTime; }

  int& samePntStops() { return _samePntStops; }
  const int& samePntStops() const { return _samePntStops; }

  int& samePntTransit() { return _samePntTransit; }
  const int& samePntTransit() const { return _samePntTransit; }

  int& samePntConnections() { return _samePntConnections; }
  const int& samePntConnections() const { return _samePntConnections; }

  std::string& noStopsMin() { return _noStopsMin; }
  const std::string& noStopsMin() const { return _noStopsMin; }

  std::string& noStopsMax() { return _noStopsMax; }
  const std::string& noStopsMax() const { return _noStopsMax; }

  std::string& noStopsOutbound() { return _noStopsOutbound; }
  const std::string& noStopsOutbound() const { return _noStopsOutbound; }

  std::string& noStopsInbound() { return _noStopsInbound; }
  const std::string& noStopsInbound() const { return _noStopsInbound; }

  Indicator& outOrReturnInd() { return _outOrReturnInd; }
  const Indicator& outOrReturnInd() const { return _outOrReturnInd; }

  Indicator& sameCarrierInd() { return _sameCarrierInd; }
  const Indicator& sameCarrierInd() const { return _sameCarrierInd; }

  Indicator& ojStopoverInd() { return _ojStopoverInd; }
  const Indicator& ojStopoverInd() const { return _ojStopoverInd; }

  Indicator& ct2StopoverInd() { return _ct2StopoverInd; }
  const Indicator& ct2StopoverInd() const { return _ct2StopoverInd; }

  Indicator& ct2PlusStopoverInd() { return _ct2PlusStopoverInd; }
  const Indicator& ct2PlusStopoverInd() const { return _ct2PlusStopoverInd; }

  Indicator& gtwyInd() { return _gtwyInd; }
  const Indicator& gtwyInd() const { return _gtwyInd; }

  Indicator& minStayTimeUnit() { return _minStayTimeUnit; }
  const Indicator& minStayTimeUnit() const { return _minStayTimeUnit; }

  Indicator& maxStayTimeUnit() { return _maxStayTimeUnit; }
  const Indicator& maxStayTimeUnit() const { return _maxStayTimeUnit; }

  Indicator& charge1Appl() { return _charge1Appl; }
  const Indicator& charge1Appl() const { return _charge1Appl; }

  Indicator& charge1Total() { return _charge1Total; }
  const Indicator& charge1Total() const { return _charge1Total; }

  std::string& charge1First() { return _charge1First; }
  const std::string& charge1First() const { return _charge1First; }

  std::string& charge1AddNo() { return _charge1AddNo; }
  const std::string& charge1AddNo() const { return _charge1AddNo; }

  CurrencyCode& charge1Cur() { return _charge1Cur; }
  const CurrencyCode& charge1Cur() const { return _charge1Cur; }

  Indicator& charge2Appl() { return _charge2Appl; }
  const Indicator& charge2Appl() const { return _charge2Appl; }

  Indicator& charge2Total() { return _charge2Total; }
  const Indicator& charge2Total() const { return _charge2Total; }

  std::string& charge2First() { return _charge2First; }
  const std::string& charge2First() const { return _charge2First; }

  std::string& charge2AddNo() { return _charge2AddNo; }
  const std::string& charge2AddNo() const { return _charge2AddNo; }

  CurrencyCode& charge2Cur() { return _charge2Cur; }
  const CurrencyCode& charge2Cur() const { return _charge2Cur; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<StopoversInfoSeg*>& segs() { return _segs; }
  const std::vector<StopoversInfoSeg*>& segs() const { return _segs; }

  virtual bool operator==(const StopoversInfo& rhs) const
  {
    bool eq(
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
        (_geoTblItemNoBtw == rhs._geoTblItemNoBtw) && (_geoTblItemNoAnd == rhs._geoTblItemNoAnd) &&
        (_geoTblItemNoGateway == rhs._geoTblItemNoGateway) &&
        (_samePointsTblItemNo == rhs._samePointsTblItemNo) &&
        (_charge1NoDec == rhs._charge1NoDec) && (_charge1FirstAmt == rhs._charge1FirstAmt) &&
        (_charge1AddAmt == rhs._charge1AddAmt) && (_charge2NoDec == rhs._charge2NoDec) &&
        (_charge2FirstAmt == rhs._charge2FirstAmt) && (_charge2AddAmt == rhs._charge2AddAmt) &&
        (_segCnt == rhs._segCnt) && (_minStayTime == rhs._minStayTime) &&
        (_maxStayTime == rhs._maxStayTime) && (_samePntStops == rhs._samePntStops) &&
        (_samePntTransit == rhs._samePntTransit) &&
        (_samePntConnections == rhs._samePntConnections) && (_noStopsMin == rhs._noStopsMin) &&
        (_noStopsMax == rhs._noStopsMax) && (_noStopsOutbound == rhs._noStopsOutbound) &&
        (_noStopsInbound == rhs._noStopsInbound) && (_outOrReturnInd == rhs._outOrReturnInd) &&
        (_sameCarrierInd == rhs._sameCarrierInd) && (_ojStopoverInd == rhs._ojStopoverInd) &&
        (_ct2StopoverInd == rhs._ct2StopoverInd) &&
        (_ct2PlusStopoverInd == rhs._ct2PlusStopoverInd) && (_gtwyInd == rhs._gtwyInd) &&
        (_minStayTimeUnit == rhs._minStayTimeUnit) && (_maxStayTimeUnit == rhs._maxStayTimeUnit) &&
        (_charge1Appl == rhs._charge1Appl) && (_charge1Total == rhs._charge1Total) &&
        (_charge1First == rhs._charge1First) && (_charge1AddNo == rhs._charge1AddNo) &&
        (_charge1Cur == rhs._charge1Cur) && (_charge2Appl == rhs._charge2Appl) &&
        (_charge2Total == rhs._charge2Total) && (_charge2First == rhs._charge2First) &&
        (_charge2AddNo == rhs._charge2AddNo) && (_charge2Cur == rhs._charge2Cur) &&
        (_inhibit == rhs._inhibit) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(StopoversInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNoBtw = 1;
    obj._geoTblItemNoAnd = 2;
    obj._geoTblItemNoGateway = 3;
    obj._samePointsTblItemNo = 4;
    obj._charge1NoDec = 5;
    obj._charge1FirstAmt = 6.66;
    obj._charge1AddAmt = 7.77;
    obj._charge2NoDec = 8;
    obj._charge2FirstAmt = 9.99;
    obj._charge2AddAmt = 10.10;
    obj._segCnt = 11;
    obj._minStayTime = 12;
    obj._maxStayTime = 13;
    obj._samePntStops = 14;
    obj._samePntTransit = 15;
    obj._samePntConnections = 16;
    obj._noStopsMin = "aaaaaaaa";
    obj._noStopsMax = "bbbbbbbb";
    obj._noStopsOutbound = "cccccccc";
    obj._noStopsInbound = "dddddddd";
    obj._outOrReturnInd = 'B';
    obj._sameCarrierInd = 'C';
    obj._ojStopoverInd = 'D';
    obj._ct2StopoverInd = 'E';
    obj._ct2PlusStopoverInd = 'F';
    obj._gtwyInd = 'G';
    obj._minStayTimeUnit = 'H';
    obj._maxStayTimeUnit = 'I';
    obj._charge1Appl = 'J';
    obj._charge1Total = 'K';
    obj._charge1First = "eeeeeeee";
    obj._charge1AddNo = "ffffffff";
    obj._charge1Cur = "LMN";
    obj._charge2Appl = 'O';
    obj._charge2Total = 'P';
    obj._charge2First = "gggggggg";
    obj._charge2AddNo = "hhhhhhhh";
    obj._charge2Cur = "RST";
    obj._inhibit = 'U';

    StopoversInfoSeg* sis1 = new StopoversInfoSeg;
    StopoversInfoSeg* sis2 = new StopoversInfoSeg;

    StopoversInfoSeg::dummyData(*sis1);
    StopoversInfoSeg::dummyData(*sis2);

    obj._segs.push_back(sis1);
    obj._segs.push_back(sis2);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailTag;
  int _geoTblItemNoBtw;
  int _geoTblItemNoAnd;
  int _geoTblItemNoGateway;
  int _samePointsTblItemNo;
  int _charge1NoDec;
  MoneyAmount _charge1FirstAmt;
  MoneyAmount _charge1AddAmt;
  int _charge2NoDec;
  MoneyAmount _charge2FirstAmt;
  MoneyAmount _charge2AddAmt;
  int _segCnt;
  int _minStayTime;
  int _maxStayTime;
  int _samePntStops;
  int _samePntTransit;
  int _samePntConnections;
  std::string _noStopsMin;
  std::string _noStopsMax;
  std::string _noStopsOutbound;
  std::string _noStopsInbound;
  Indicator _outOrReturnInd;
  Indicator _sameCarrierInd;
  Indicator _ojStopoverInd;
  Indicator _ct2StopoverInd;
  Indicator _ct2PlusStopoverInd;
  Indicator _gtwyInd;
  Indicator _minStayTimeUnit;
  Indicator _maxStayTimeUnit;
  Indicator _charge1Appl;
  Indicator _charge1Total;
  std::string _charge1First;
  std::string _charge1AddNo;
  CurrencyCode _charge1Cur;
  Indicator _charge2Appl;
  Indicator _charge2Total;
  std::string _charge2First;
  std::string _charge2AddNo;
  CurrencyCode _charge2Cur;
  Indicator _inhibit;
  std::vector<StopoversInfoSeg*> _segs;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNoBtw);
    FLATTENIZE(archive, _geoTblItemNoAnd);
    FLATTENIZE(archive, _geoTblItemNoGateway);
    FLATTENIZE(archive, _samePointsTblItemNo);
    FLATTENIZE(archive, _charge1NoDec);
    FLATTENIZE(archive, _charge1FirstAmt);
    FLATTENIZE(archive, _charge1AddAmt);
    FLATTENIZE(archive, _charge2NoDec);
    FLATTENIZE(archive, _charge2FirstAmt);
    FLATTENIZE(archive, _charge2AddAmt);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _minStayTime);
    FLATTENIZE(archive, _maxStayTime);
    FLATTENIZE(archive, _samePntStops);
    FLATTENIZE(archive, _samePntTransit);
    FLATTENIZE(archive, _samePntConnections);
    FLATTENIZE(archive, _noStopsMin);
    FLATTENIZE(archive, _noStopsMax);
    FLATTENIZE(archive, _noStopsOutbound);
    FLATTENIZE(archive, _noStopsInbound);
    FLATTENIZE(archive, _outOrReturnInd);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _ojStopoverInd);
    FLATTENIZE(archive, _ct2StopoverInd);
    FLATTENIZE(archive, _ct2PlusStopoverInd);
    FLATTENIZE(archive, _gtwyInd);
    FLATTENIZE(archive, _minStayTimeUnit);
    FLATTENIZE(archive, _maxStayTimeUnit);
    FLATTENIZE(archive, _charge1Appl);
    FLATTENIZE(archive, _charge1Total);
    FLATTENIZE(archive, _charge1First);
    FLATTENIZE(archive, _charge1AddNo);
    FLATTENIZE(archive, _charge1Cur);
    FLATTENIZE(archive, _charge2Appl);
    FLATTENIZE(archive, _charge2Total);
    FLATTENIZE(archive, _charge2First);
    FLATTENIZE(archive, _charge2AddNo);
    FLATTENIZE(archive, _charge2Cur);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _segs);
  }

protected:
private:
  StopoversInfo(const StopoversInfo&);
  StopoversInfo& operator=(const StopoversInfo&);

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailTag
           & ptr->_geoTblItemNoBtw
           & ptr->_geoTblItemNoAnd
           & ptr->_geoTblItemNoGateway
           & ptr->_samePointsTblItemNo
           & ptr->_charge1NoDec
           & ptr->_charge1FirstAmt
           & ptr->_charge1AddAmt
           & ptr->_charge2NoDec
           & ptr->_charge2FirstAmt
           & ptr->_charge2AddAmt
           & ptr->_segCnt
           & ptr->_minStayTime
           & ptr->_maxStayTime
           & ptr->_samePntStops
           & ptr->_samePntTransit
           & ptr->_samePntConnections
           & ptr->_noStopsMin
           & ptr->_noStopsMax
           & ptr->_noStopsOutbound
           & ptr->_noStopsInbound
           & ptr->_outOrReturnInd
           & ptr->_sameCarrierInd
           & ptr->_ojStopoverInd
           & ptr->_ct2StopoverInd
           & ptr->_ct2PlusStopoverInd
           & ptr->_gtwyInd
           & ptr->_minStayTimeUnit
           & ptr->_maxStayTimeUnit
           & ptr->_charge1Appl
           & ptr->_charge1Total
           & ptr->_charge1First
           & ptr->_charge1AddNo
           & ptr->_charge1Cur
           & ptr->_charge2Appl
           & ptr->_charge2Total
           & ptr->_charge2First
           & ptr->_charge2AddNo
           & ptr->_charge2Cur
           & ptr->_inhibit
           & ptr->_segs;
  }
};
}
