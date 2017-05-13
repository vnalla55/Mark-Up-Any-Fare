//-------------------------------------------------------------------
//
//  File:        PaxType.h
//  Created:     March 8, 2004
//  Authors:     Vadim Nikushin
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <functional>
#include <map>
#include <vector>

namespace tse
{

class PaxTypeInfo;
class MaxPenaltyInfo;

class PaxType
{
public:
  class FreqFlyerTierWithCarrier
  {
    FreqFlyerTierLevel _ffTierLevel = 0;
    CarrierCode _cxr;
    bool _isFromPnr = true;

  public:
    void setFreqFlyerTierLevel(const FreqFlyerTierLevel ffTierLevel) { _ffTierLevel = ffTierLevel; }
    const FreqFlyerTierLevel& freqFlyerTierLevel() const { return _ffTierLevel; }

    void setCxr(const CarrierCode cxr) { _cxr = cxr; }
    const CarrierCode& cxr() const { return _cxr; }

    void setIsFromPnr(const bool isFromPnr) { _isFromPnr = isFromPnr; }
    const bool& isFromPnr() const { return _isFromPnr; }
  };

  class TktInfo
  {
    PsgNameNumber _psgNameNumber;
    TktNumber _tktNumber;
    TktNumber _tktRefNumber;

  public:
    PsgNameNumber& psgNameNumber() { return _psgNameNumber; }
    const PsgNameNumber& psgNameNumber() const { return _psgNameNumber; }

    TktNumber& tktNumber() { return _tktNumber; }
    const TktNumber& tktNumber() const { return _tktNumber; }

    TktNumber& tktRefNumber() { return _tktRefNumber; }
    const TktNumber& tktRefNumber() const { return _tktRefNumber; }
  };

private:
  PaxTypeCode _paxType; // PaxType code
  PaxTypeCode _requestedPaxType; // Requested Pax Type code
  uint16_t _number = 0; // number of seats
  uint16_t _age = 0; // age
  DateTime _birthDate;
  StateCode _stateCode; // state code
  uint16_t _totalPaxNumber = 0; // total number of passengers in the PNR
  uint16_t _inputOrder = 0; // order in which PaxType has appeared in request

  VendorCode _vendorCode; // each vendor has own set of PaxTypes

  const PaxTypeInfo* _paxTypeInfo = nullptr;
  std::map<CarrierCode, std::vector<PaxType*>*> _actualPaxType;

  std::vector<FreqFlyerTierWithCarrier*> _freqFlyerTiersWithCarriers;

  std::vector<TktInfo*> _psgTktInfo;

  MaxPenaltyInfo* _maxPenaltyInfo = nullptr;
  MoneyAmount _mslAmount = 0.0;

public:
  virtual ~PaxType() = default;
  bool operator==(const PaxType& paxType) const;

  // Access

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  PaxTypeCode& requestedPaxType() { return _requestedPaxType; }
  const PaxTypeCode& requestedPaxType() const { return _requestedPaxType; }

  uint16_t& number() { return _number; }
  const uint16_t& number() const { return _number; }

  uint16_t& age() { return _age; }
  const uint16_t& age() const { return _age; }

  DateTime& birthDate() { return _birthDate; }
  const DateTime& birthDate() const { return _birthDate; }

  StateCode& stateCode() { return _stateCode; }
  const StateCode& stateCode() const { return _stateCode; }

  uint16_t& totalPaxNumber() { return _totalPaxNumber; }
  const uint16_t& totalPaxNumber() const { return _totalPaxNumber; }

  uint16_t& inputOrder() { return _inputOrder; }
  const uint16_t& inputOrder() const { return _inputOrder; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  const PaxTypeInfo*& paxTypeInfo() { return _paxTypeInfo; }
  const PaxTypeInfo& paxTypeInfo() const { return *_paxTypeInfo; }

  std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxType() { return _actualPaxType; }
  const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxType() const
  {
    return _actualPaxType;
  }

  std::vector<FreqFlyerTierWithCarrier*>& freqFlyerTierWithCarrier()
  {
    return _freqFlyerTiersWithCarriers;
  }
  const std::vector<FreqFlyerTierWithCarrier*>& freqFlyerTierWithCarrier() const
  {
    return _freqFlyerTiersWithCarriers;
  }

  std::vector<TktInfo*>& psgTktInfo() { return _psgTktInfo; }
  const std::vector<TktInfo*>& psgTktInfo() const { return _psgTktInfo; }

  MaxPenaltyInfo*& maxPenaltyInfo() { return _maxPenaltyInfo; }
  const MaxPenaltyInfo* maxPenaltyInfo() const { return _maxPenaltyInfo; }

  MoneyAmount& mslAmount() { return _mslAmount; }
  const MoneyAmount& mslAmount() const { return _mslAmount; }

  /**
   *  For sorting the PaxType by inputOrder
   **/
  class InputOrder : public std::binary_function<PaxType*, PaxType*, bool>
  {
  public:
    InputOrder() {}
    bool operator()(const PaxType* pt1, const PaxType* pt2) const
    {
      if (UNLIKELY(pt1->inputOrder() == pt2->inputOrder()))
        return pt1 < pt2;

      return pt1->inputOrder() < pt2->inputOrder();
    }
  };
};

} // tse namespace
