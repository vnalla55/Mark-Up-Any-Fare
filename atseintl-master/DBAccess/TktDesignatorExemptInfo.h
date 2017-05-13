#pragma once

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TktDesignatorExemptTaxAInfo.h"

namespace tse
{
class TktDesignatorExemptInfo
{
public:
  TktDesignatorExemptInfo()
    : _sequenceNumber(-1),
      _validityInd('Y'),
      _exemptionType(' '),
      _taxExempt(' '),
      _pfcExempt(' '),
      _ruleTariff(-1),
      _ruleRelational(' '),
      _directionality(TO),
      _locType1(' '),
      _locType2(' ')
  {
  }

  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }
  const DateTime& versionDate() const { return _versionDate; }
  DateTime& versionDate() { return _versionDate; }
  SequenceNumber sequenceNumber() const { return _sequenceNumber; }
  SequenceNumber& sequenceNumber() { return _sequenceNumber; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& createDate() { return _createDate; }
  const DateTime& effDate() const { return _effDate; }
  DateTime& effDate() { return _effDate; }
  const DateTime& discDate() const { return _discDate; }
  DateTime& discDate() { return _discDate; }
  const DateTime& expireDate() const { return _expireDate; }
  DateTime& expireDate() { return _expireDate; }
  Indicator validityInd() const { return _validityInd; }
  Indicator& validityInd() { return _validityInd; }
  const Description& description() const { return _description; }
  Description& description() { return _description; }
  Indicator exemptionType() const { return _exemptionType; }
  Indicator& exemptionType() { return _exemptionType; }
  const TktDesignator& ticketDesignator() const { return _ticketDesignator; }
  TktDesignator& ticketDesignator() { return _ticketDesignator; }
  const PaxTypeCode& paxType() const { return _paxType; }
  PaxTypeCode& paxType() { return _paxType; }
  Indicator taxExempt() const { return _taxExempt; }
  Indicator& taxExempt() { return _taxExempt; }
  Indicator pfcExempt() const { return _pfcExempt; }
  Indicator& pfcExempt() { return _pfcExempt; }
  TariffNumber ruleTariff() const { return _ruleTariff; }
  TariffNumber& ruleTariff() { return _ruleTariff; }
  const RuleNumber& ruleNumber1() const { return _ruleNumber1; }
  RuleNumber& ruleNumber1() { return _ruleNumber1; }
  Indicator ruleRelational() const { return _ruleRelational; }
  Indicator& ruleRelational() { return _ruleRelational; }
  const RuleNumber& ruleNumber2() const { return _ruleNumber2; }
  RuleNumber& ruleNumber2() { return _ruleNumber2; }
  Directionality directionality() const { return _directionality; }
  Directionality& directionality() { return _directionality; }
  Indicator locType1() const { return _locType1; }
  Indicator& locType1() { return _locType1; }
  const LocCode& loc1() const { return _loc1; }
  LocCode& loc1() { return _loc1; }
  Indicator locType2() const { return _locType2; }
  Indicator& locType2() { return _locType2; }
  const LocCode& loc2() const { return _loc2; }
  LocCode& loc2() { return _loc2; }
  const std::vector<TktDesignatorExemptTaxAInfo*>& tktDesignatorExemptTaxA() const
  {
    return _tktDesignatorExemptTaxA;
  }
  std::vector<TktDesignatorExemptTaxAInfo*>& tktDesignatorExemptTaxA()
  {
    return _tktDesignatorExemptTaxA;
  }

  bool operator==(const TktDesignatorExemptInfo& rhs) const
  {
    return _carrier == rhs._carrier && _versionDate == rhs._versionDate &&
           _sequenceNumber == rhs._sequenceNumber && _createDate == rhs._createDate &&
           _effDate == rhs._effDate && _discDate == rhs._discDate &&
           _expireDate == rhs._expireDate && _validityInd == rhs._validityInd &&
           _description == rhs._description && _exemptionType == rhs._exemptionType &&
           _ticketDesignator == rhs._ticketDesignator && _paxType == rhs._paxType &&
           _taxExempt == rhs._taxExempt && _pfcExempt == rhs._pfcExempt &&
           _ruleTariff == rhs._ruleTariff && _ruleNumber1 == rhs._ruleNumber1 &&
           _ruleRelational == rhs._ruleRelational && _ruleNumber2 == rhs._ruleNumber2 &&
           _directionality == rhs._directionality && _locType1 == rhs._locType1 &&
           _loc1 == rhs._loc1 && _locType2 == rhs._locType2 && _loc2 == rhs._loc2 &&
           _tktDesignatorExemptTaxA == rhs._tktDesignatorExemptTaxA;
  }

  static void dummyData(TktDesignatorExemptInfo& obj)
  {
    obj._carrier = "AA";
    obj._versionDate = time(nullptr);
    obj._sequenceNumber = 22222;
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'Y';
    obj._description = "description";
    obj._exemptionType = '-';
    obj._ticketDesignator = "ID75";
    obj._paxType = "ADT";
    obj._taxExempt = 'X';
    obj._pfcExempt = 'P';
    obj._ruleTariff = 772;
    obj._ruleNumber1 = "MWK1";
    obj._ruleRelational = '/';
    obj._ruleNumber2 = "2001";
    obj._directionality = TO;
    obj._locType1 = 'Z';
    obj._loc1 = "DFW";
    obj._locType2 = 'N';
    obj._loc2 = "NYC";
    TktDesignatorExemptTaxAInfo* taxInfo(new TktDesignatorExemptTaxAInfo);
    TktDesignatorExemptTaxAInfo::dummyData(*taxInfo);
    obj._tktDesignatorExemptTaxA.push_back(taxInfo);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _sequenceNumber);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _exemptionType);
    FLATTENIZE(archive, _ticketDesignator);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _taxExempt);
    FLATTENIZE(archive, _pfcExempt);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _ruleNumber1);
    FLATTENIZE(archive, _ruleRelational);
    FLATTENIZE(archive, _ruleNumber2);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _locType1);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _locType2);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _tktDesignatorExemptTaxA);
  }

private:
  CarrierCode _carrier;
  DateTime _versionDate;
  SequenceNumber _sequenceNumber;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  Indicator _validityInd;
  Description _description;
  Indicator _exemptionType;
  TktDesignator _ticketDesignator;
  PaxTypeCode _paxType;
  Indicator _taxExempt;
  Indicator _pfcExempt;
  TariffNumber _ruleTariff;
  RuleNumber _ruleNumber1;
  Indicator _ruleRelational;
  RuleNumber _ruleNumber2;
  Directionality _directionality;
  Indicator _locType1;
  LocCode _loc1;
  Indicator _locType2;
  LocCode _loc2;
  std::vector<TktDesignatorExemptTaxAInfo*> _tktDesignatorExemptTaxA;
};

} // tse

