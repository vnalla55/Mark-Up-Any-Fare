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
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TransfersInfoSeg1
{
public:
  TransfersInfoSeg1()
    : _orderNo(0),
      _transferAppl(' '),
      _noTransfersPermitted(""),
      _primeOnline(' '),
      _sameOnline(' '),
      _primeInterline(' '),
      _otherInterline(' '),
      _stopoverConnectInd(' '),
      _carrierAppl(' '),
      _carrierIn(""),
      _carrierOut(""),
      _inCarrierApplTblItemNo(0),
      _outCarrierApplTblItemNo(0),
      _tsi(0),
      _loc1(),
      _loc2(),
      _zoneTblItemNo(0),
      _betweenAppl(' '),
      _gateway(' '),
      _restriction(' '),
      _outInPortion(' '),
      _chargeAppl(' ')
  {
  }

  ~TransfersInfoSeg1() {}

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  Indicator& transferAppl() { return _transferAppl; }
  const Indicator& transferAppl() const { return _transferAppl; }

  std::string& noTransfersPermitted() { return _noTransfersPermitted; }
  const std::string& noTransfersPermitted() const { return _noTransfersPermitted; }

  Indicator& primeOnline() { return _primeOnline; }
  const Indicator& primeOnline() const { return _primeOnline; }

  Indicator& sameOnline() { return _sameOnline; }
  const Indicator& sameOnline() const { return _sameOnline; }

  Indicator& primeInterline() { return _primeInterline; }
  const Indicator& primeInterline() const { return _primeInterline; }

  Indicator& otherInterline() { return _otherInterline; }
  const Indicator& otherInterline() const { return _otherInterline; }

  Indicator& stopoverConnectInd() { return _stopoverConnectInd; }
  const Indicator& stopoverConnectInd() const { return _stopoverConnectInd; }

  Indicator& carrierAppl() { return _carrierAppl; }
  const Indicator& carrierAppl() const { return _carrierAppl; }

  CarrierCode& carrierIn() { return _carrierIn; }
  const CarrierCode& carrierIn() const { return _carrierIn; }

  CarrierCode& carrierOut() { return _carrierOut; }
  const CarrierCode& carrierOut() const { return _carrierOut; }

  uint32_t& inCarrierApplTblItemNo() { return _inCarrierApplTblItemNo; }
  const uint32_t& inCarrierApplTblItemNo() const { return _inCarrierApplTblItemNo; }

  uint32_t& outCarrierApplTblItemNo() { return _outCarrierApplTblItemNo; }
  const uint32_t& outCarrierApplTblItemNo() const { return _outCarrierApplTblItemNo; }

  int16_t& tsi() { return _tsi; }
  const int16_t& tsi() const { return _tsi; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  uint32_t& zoneTblItemNo() { return _zoneTblItemNo; }
  const uint32_t& zoneTblItemNo() const { return _zoneTblItemNo; }

  Indicator& betweenAppl() { return _betweenAppl; }
  const Indicator& betweenAppl() const { return _betweenAppl; }

  Indicator& gateway() { return _gateway; }
  const Indicator& gateway() const { return _gateway; }

  Indicator& restriction() { return _restriction; }
  const Indicator& restriction() const { return _restriction; }

  Indicator& outInPortion() { return _outInPortion; }
  const Indicator& outInPortion() const { return _outInPortion; }

  Indicator& chargeAppl() { return _chargeAppl; }
  const Indicator& chargeAppl() const { return _chargeAppl; }

  bool operator==(const TransfersInfoSeg1& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_transferAppl == rhs._transferAppl) &&
            (_noTransfersPermitted == rhs._noTransfersPermitted) &&
            (_primeOnline == rhs._primeOnline) && (_sameOnline == rhs._sameOnline) &&
            (_primeInterline == rhs._primeInterline) && (_otherInterline == rhs._otherInterline) &&
            (_stopoverConnectInd == rhs._stopoverConnectInd) &&
            (_carrierAppl == rhs._carrierAppl) && (_carrierIn == rhs._carrierIn) &&
            (_carrierOut == rhs._carrierOut) &&
            (_inCarrierApplTblItemNo == rhs._inCarrierApplTblItemNo) &&
            (_outCarrierApplTblItemNo == rhs._outCarrierApplTblItemNo) && (_tsi == rhs._tsi) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
            (_zoneTblItemNo == rhs._zoneTblItemNo) && (_betweenAppl == rhs._betweenAppl) &&
            (_gateway == rhs._gateway) && (_restriction == rhs._restriction) &&
            (_outInPortion == rhs._outInPortion) && (_chargeAppl == rhs._chargeAppl));
  }

  static void dummyData(TransfersInfoSeg1& obj)
  {
    obj._orderNo = 1;
    obj._transferAppl = 'A';
    obj._noTransfersPermitted = "aaaaaaaa";
    obj._primeOnline = 'B';
    obj._sameOnline = 'C';
    obj._primeInterline = 'D';
    obj._otherInterline = 'E';
    obj._stopoverConnectInd = 'F';
    obj._carrierAppl = 'G';
    obj._carrierIn = "HIJ";
    obj._carrierOut = "KLM";
    obj._inCarrierApplTblItemNo = 2;
    obj._outCarrierApplTblItemNo = 3;
    obj._tsi = 4;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._zoneTblItemNo = 5;
    obj._betweenAppl = 'N';
    obj._gateway = 'O';
    obj._restriction = 'P';
    obj._outInPortion = 'Q';
    obj._chargeAppl = 'R';
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
  int _orderNo;
  Indicator _transferAppl;
  std::string _noTransfersPermitted;
  Indicator _primeOnline;
  Indicator _sameOnline;
  Indicator _primeInterline;
  Indicator _otherInterline;
  Indicator _stopoverConnectInd;
  Indicator _carrierAppl;
  CarrierCode _carrierIn;
  CarrierCode _carrierOut;
  uint32_t _inCarrierApplTblItemNo;
  uint32_t _outCarrierApplTblItemNo;
  int16_t _tsi;
  LocKey _loc1;
  LocKey _loc2;
  uint32_t _zoneTblItemNo;
  Indicator _betweenAppl;
  Indicator _gateway;
  Indicator _restriction;
  Indicator _outInPortion;
  Indicator _chargeAppl;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _transferAppl);
    FLATTENIZE(archive, _noTransfersPermitted);
    FLATTENIZE(archive, _primeOnline);
    FLATTENIZE(archive, _sameOnline);
    FLATTENIZE(archive, _primeInterline);
    FLATTENIZE(archive, _otherInterline);
    FLATTENIZE(archive, _stopoverConnectInd);
    FLATTENIZE(archive, _carrierAppl);
    FLATTENIZE(archive, _carrierIn);
    FLATTENIZE(archive, _carrierOut);
    FLATTENIZE(archive, _inCarrierApplTblItemNo);
    FLATTENIZE(archive, _outCarrierApplTblItemNo);
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _zoneTblItemNo);
    FLATTENIZE(archive, _betweenAppl);
    FLATTENIZE(archive, _gateway);
    FLATTENIZE(archive, _restriction);
    FLATTENIZE(archive, _outInPortion);
    FLATTENIZE(archive, _chargeAppl);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_orderNo
           & ptr->_transferAppl
           & ptr->_noTransfersPermitted
           & ptr->_primeOnline
           & ptr->_sameOnline
           & ptr->_primeInterline
           & ptr->_otherInterline
           & ptr->_stopoverConnectInd
           & ptr->_carrierAppl
           & ptr->_carrierIn
           & ptr->_carrierOut
           & ptr->_inCarrierApplTblItemNo
           & ptr->_outCarrierApplTblItemNo
           & ptr->_tsi
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_zoneTblItemNo
           & ptr->_betweenAppl
           & ptr->_gateway
           & ptr->_restriction
           & ptr->_outInPortion
           & ptr->_chargeAppl;
  }
};
}
