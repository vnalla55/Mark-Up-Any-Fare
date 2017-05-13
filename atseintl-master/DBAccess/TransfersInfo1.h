// ----------------------------------------------------------------------------
//  � 2006, Sabre Inc.  All rights reserved.  This software/documentation
//    is the confidential and proprietary product of Sabre Inc. Any
//    unauthorized use, reproduction, or transfer of this
//    software/documentation, in any medium, or incorporation of this
//    software/documentation into any system or publication, is strictly
//    prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/TransfersInfoSeg1.h"

namespace tse
{

class TransfersInfo1 : public RuleItemInfo
{
public:
  TransfersInfo1()
    : _createDate(0),
      _expireDate(0),
      _unavailTag(' '),
      _primeCxrPrimeCxr(' '),
      _sameCxrSameCxr(' '),
      _primeCxrOtherCxr(' '),
      _otherCxrOtherCxr(' '),
      _noOfTransfersAppl(' '),
      _fareBreakSurfaceInd(' '),
      _embeddedSurfaceInd(' '),
      _fareBreakSurfaceTblItemNo(0),
      _embeddedSurfaceTblItemNo(0),
      _transfersChargeAppl(' '),
      _charge1Cur1Amt(0),
      _charge2Cur1Amt(0),
      _noDec1(0),
      _charge1Cur2Amt(0),
      _charge2Cur2Amt(0),
      _noDec2(0),
      _segCnt(0),
      _inhibit(' ')
  {
  }

  virtual ~TransfersInfo1()
  { // Nuke the Kids!
    std::vector<TransfersInfoSeg1*>::iterator SegIt;
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

  std::string& noTransfersMin() { return _noTransfersMin; }
  const std::string& noTransfersMin() const { return _noTransfersMin; }

  std::string& noTransfersMax() { return _noTransfersMax; }
  const std::string& noTransfersMax() const { return _noTransfersMax; }

  Indicator& primeCxrPrimeCxr() { return _primeCxrPrimeCxr; }
  const Indicator& primeCxrPrimeCxr() const { return _primeCxrPrimeCxr; }

  std::string& primePrimeMaxTransfers() { return _primePrimeMaxTransfers; }
  const std::string& primePrimeMaxTransfers() const { return _primePrimeMaxTransfers; }

  Indicator& sameCxrSameCxr() { return _sameCxrSameCxr; }
  const Indicator& sameCxrSameCxr() const { return _sameCxrSameCxr; }

  std::string& sameSameMaxTransfers() { return _sameSameMaxTransfers; }
  const std::string& sameSameMaxTransfers() const { return _sameSameMaxTransfers; }

  Indicator& primeCxrOtherCxr() { return _primeCxrOtherCxr; }
  const Indicator& primeCxrOtherCxr() const { return _primeCxrOtherCxr; }

  std::string& primeOtherMaxTransfers() { return _primeOtherMaxTransfers; }
  const std::string& primeOtherMaxTransfers() const { return _primeOtherMaxTransfers; }

  Indicator& otherCxrOtherCxr() { return _otherCxrOtherCxr; }
  const Indicator& otherCxrOtherCxr() const { return _otherCxrOtherCxr; }

  std::string& otherOtherMaxTransfers() { return _otherOtherMaxTransfers; }
  const std::string& otherOtherMaxTransfers() const { return _otherOtherMaxTransfers; }

  std::string& noOfTransfersOut() { return _noOfTransfersOut; }
  const std::string& noOfTransfersOut() const { return _noOfTransfersOut; }

  std::string& noOfTransfersIn() { return _noOfTransfersIn; }
  const std::string& noOfTransfersIn() const { return _noOfTransfersIn; }

  Indicator& noOfTransfersAppl() { return _noOfTransfersAppl; }
  const Indicator& noOfTransfersAppl() const { return _noOfTransfersAppl; }

  Indicator& fareBreakSurfaceInd() { return _fareBreakSurfaceInd; }
  const Indicator& fareBreakSurfaceInd() const { return _fareBreakSurfaceInd; }

  uint32_t& fareBreakSurfaceTblItemNo() { return _fareBreakSurfaceTblItemNo; }
  const uint32_t& fareBreakSurfaceTblItemNo() const { return _fareBreakSurfaceTblItemNo; }

  Indicator& embeddedSurfaceInd() { return _embeddedSurfaceInd; }
  const Indicator& embeddedSurfaceInd() const { return _embeddedSurfaceInd; }

  uint32_t& embeddedSurfaceTblItemNo() { return _embeddedSurfaceTblItemNo; }
  const uint32_t& embeddedSurfaceTblItemNo() const { return _embeddedSurfaceTblItemNo; }

  Indicator& transfersChargeAppl() { return _transfersChargeAppl; }
  const Indicator& transfersChargeAppl() const { return _transfersChargeAppl; }

  std::string& maxNoTransfersCharge1() { return _maxNoTransfersCharge1; }
  const std::string& maxNoTransfersCharge1() const { return _maxNoTransfersCharge1; }

  std::string& maxNoTransfersCharge2() { return _maxNoTransfersCharge2; }
  const std::string& maxNoTransfersCharge2() const { return _maxNoTransfersCharge2; }

  MoneyAmount& charge1Cur1Amt() { return _charge1Cur1Amt; }
  const MoneyAmount& charge1Cur1Amt() const { return _charge1Cur1Amt; }

  MoneyAmount& charge2Cur1Amt() { return _charge2Cur1Amt; }
  const MoneyAmount& charge2Cur1Amt() const { return _charge2Cur1Amt; }

  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }

  int16_t& noDec1() { return _noDec1; }
  const int16_t& noDec1() const { return _noDec1; }

  MoneyAmount& charge1Cur2Amt() { return _charge1Cur2Amt; }
  const MoneyAmount& charge1Cur2Amt() const { return _charge1Cur2Amt; }

  MoneyAmount& charge2Cur2Amt() { return _charge2Cur2Amt; }
  const MoneyAmount& charge2Cur2Amt() const { return _charge2Cur2Amt; }

  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }

  int16_t& noDec2() { return _noDec2; }
  const int16_t& noDec2() const { return _noDec2; }

  int16_t& segCnt() { return _segCnt; }
  const int16_t& segCnt() const { return _segCnt; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<TransfersInfoSeg1*>& segs() { return _segs; }
  const std::vector<TransfersInfoSeg1*>& segs() const { return _segs; }

  virtual bool operator==(const TransfersInfo1& rhs) const
  {
    bool eq((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
            (_noTransfersMin == rhs._noTransfersMin) && (_noTransfersMax == rhs._noTransfersMax) &&
            (_primeCxrPrimeCxr == rhs._primeCxrPrimeCxr) &&
            (_primePrimeMaxTransfers == rhs._primePrimeMaxTransfers) &&
            (_sameCxrSameCxr == rhs._sameCxrSameCxr) &&
            (_sameSameMaxTransfers == rhs._sameSameMaxTransfers) &&
            (_primeCxrOtherCxr == rhs._primeCxrOtherCxr) &&
            (_primeOtherMaxTransfers == rhs._primeOtherMaxTransfers) &&
            (_otherCxrOtherCxr == rhs._otherCxrOtherCxr) &&
            (_otherOtherMaxTransfers == rhs._otherOtherMaxTransfers) &&
            (_noOfTransfersOut == rhs._noOfTransfersOut) &&
            (_noOfTransfersIn == rhs._noOfTransfersIn) &&
            (_noOfTransfersAppl == rhs._noOfTransfersAppl) &&
            (_fareBreakSurfaceInd == rhs._fareBreakSurfaceInd) &&
            (_fareBreakSurfaceTblItemNo == rhs._fareBreakSurfaceTblItemNo) &&
            (_embeddedSurfaceInd == rhs._embeddedSurfaceInd) &&
            (_embeddedSurfaceTblItemNo == rhs._embeddedSurfaceTblItemNo) &&
            (_transfersChargeAppl == rhs._transfersChargeAppl) &&
            (_maxNoTransfersCharge1 == rhs._maxNoTransfersCharge1) &&
            (_maxNoTransfersCharge2 == rhs._maxNoTransfersCharge2) &&
            (_charge1Cur1Amt == rhs._charge1Cur1Amt) && (_charge2Cur1Amt == rhs._charge2Cur1Amt) &&
            (_cur1 == rhs._cur1) && (_noDec1 == rhs._noDec1) &&
            (_charge1Cur2Amt == rhs._charge1Cur2Amt) && (_charge2Cur2Amt == rhs._charge2Cur2Amt) &&
            (_cur2 == rhs._cur2) && (_noDec2 == rhs._noDec2) && (_segCnt == rhs._segCnt) &&
            (_inhibit == rhs._inhibit) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(TransfersInfo1& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._noTransfersMin = "aaaaaaaa";
    obj._noTransfersMax = "bbbbbbbb";
    obj._primeCxrPrimeCxr = 'B';
    obj._primePrimeMaxTransfers = "cccccccc";
    obj._sameCxrSameCxr = 'C';
    obj._sameSameMaxTransfers = "dddddddd";
    obj._primeCxrOtherCxr = 'D';
    obj._primeOtherMaxTransfers = "eeeeeeee";
    obj._otherCxrOtherCxr = 'E';
    obj._otherOtherMaxTransfers = "ffffffff";
    obj._noOfTransfersOut = "gggggggg";
    obj._noOfTransfersIn = "hhhhhhhh";
    obj._noOfTransfersAppl = 'F';
    obj._fareBreakSurfaceInd = 'G';
    obj._fareBreakSurfaceTblItemNo = 1;
    obj._embeddedSurfaceInd = 'H';
    obj._embeddedSurfaceTblItemNo = 2;
    obj._transfersChargeAppl = 'I';
    obj._maxNoTransfersCharge1 = "iiiiiiii";
    obj._maxNoTransfersCharge2 = "jjjjjjjj";
    obj._charge1Cur1Amt = 3.33;
    obj._charge2Cur1Amt = 4.44;
    obj._cur1 = "JKL";
    obj._noDec1 = 5;
    obj._charge1Cur2Amt = 6.66;
    obj._charge2Cur2Amt = 7.77;
    obj._cur2 = "MNO";
    obj._noDec2 = 8;
    obj._segCnt = 9;
    obj._inhibit = 'P';

    TransfersInfoSeg1* tis1 = new TransfersInfoSeg1;
    TransfersInfoSeg1* tis2 = new TransfersInfoSeg1;

    TransfersInfoSeg1::dummyData(*tis1);
    TransfersInfoSeg1::dummyData(*tis2);

    obj._segs.push_back(tis1);
    obj._segs.push_back(tis2);
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
  Indicator _primeCxrPrimeCxr;
  Indicator _sameCxrSameCxr;
  Indicator _primeCxrOtherCxr;
  Indicator _otherCxrOtherCxr;
  Indicator _noOfTransfersAppl;
  Indicator _fareBreakSurfaceInd;
  Indicator _embeddedSurfaceInd;
  std::string _noTransfersMin;
  std::string _noTransfersMax;
  std::string _primePrimeMaxTransfers;
  std::string _sameSameMaxTransfers;
  std::string _primeOtherMaxTransfers;
  std::string _otherOtherMaxTransfers;
  std::string _noOfTransfersOut;
  std::string _noOfTransfersIn;
  uint32_t _fareBreakSurfaceTblItemNo;
  uint32_t _embeddedSurfaceTblItemNo;
  Indicator _transfersChargeAppl;
  std::string _maxNoTransfersCharge1;
  std::string _maxNoTransfersCharge2;
  MoneyAmount _charge1Cur1Amt;
  MoneyAmount _charge2Cur1Amt;
  CurrencyCode _cur1;
  int16_t _noDec1;
  MoneyAmount _charge1Cur2Amt;
  MoneyAmount _charge2Cur2Amt;
  CurrencyCode _cur2;
  int16_t _noDec2;
  int16_t _segCnt;
  Indicator _inhibit;
  std::vector<TransfersInfoSeg1*> _segs;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _noTransfersMin);
    FLATTENIZE(archive, _noTransfersMax);
    FLATTENIZE(archive, _primeCxrPrimeCxr);
    FLATTENIZE(archive, _primePrimeMaxTransfers);
    FLATTENIZE(archive, _sameCxrSameCxr);
    FLATTENIZE(archive, _sameSameMaxTransfers);
    FLATTENIZE(archive, _primeCxrOtherCxr);
    FLATTENIZE(archive, _primeOtherMaxTransfers);
    FLATTENIZE(archive, _otherCxrOtherCxr);
    FLATTENIZE(archive, _otherOtherMaxTransfers);
    FLATTENIZE(archive, _noOfTransfersOut);
    FLATTENIZE(archive, _noOfTransfersIn);
    FLATTENIZE(archive, _noOfTransfersAppl);
    FLATTENIZE(archive, _fareBreakSurfaceInd);
    FLATTENIZE(archive, _fareBreakSurfaceTblItemNo);
    FLATTENIZE(archive, _embeddedSurfaceInd);
    FLATTENIZE(archive, _embeddedSurfaceTblItemNo);
    FLATTENIZE(archive, _transfersChargeAppl);
    FLATTENIZE(archive, _maxNoTransfersCharge1);
    FLATTENIZE(archive, _maxNoTransfersCharge2);
    FLATTENIZE(archive, _charge1Cur1Amt);
    FLATTENIZE(archive, _charge2Cur1Amt);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _charge1Cur2Amt);
    FLATTENIZE(archive, _charge2Cur2Amt);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _noDec2);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _segs);
  }

protected:

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_unavailTag & ptr->_noTransfersMin &
           ptr->_noTransfersMax & ptr->_primeCxrPrimeCxr & ptr->_primePrimeMaxTransfers &
           ptr->_sameCxrSameCxr & ptr->_sameSameMaxTransfers & ptr->_primeCxrOtherCxr &
           ptr->_primeOtherMaxTransfers & ptr->_otherCxrOtherCxr & ptr->_otherOtherMaxTransfers &
           ptr->_noOfTransfersOut & ptr->_noOfTransfersIn & ptr->_noOfTransfersAppl &
           ptr->_fareBreakSurfaceInd & ptr->_fareBreakSurfaceTblItemNo & ptr->_embeddedSurfaceInd &
           ptr->_embeddedSurfaceTblItemNo & ptr->_transfersChargeAppl &
           ptr->_maxNoTransfersCharge1 & ptr->_maxNoTransfersCharge2 & ptr->_charge1Cur1Amt &
           ptr->_charge2Cur1Amt & ptr->_cur1 & ptr->_noDec1 & ptr->_charge1Cur2Amt &
           ptr->_charge2Cur2Amt & ptr->_cur2 & ptr->_noDec2 & ptr->_segCnt & ptr->_inhibit &
           ptr->_segs;
  }

  TransfersInfo1(const TransfersInfo1&);
  TransfersInfo1& operator=(const TransfersInfo1&);
};
}
