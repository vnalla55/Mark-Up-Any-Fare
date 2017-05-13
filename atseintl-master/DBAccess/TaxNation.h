//----------------------------------------------------------------------------
//
//  File:           TaxNation.h
//  Description:    TaxNation processing data
//  Created:        2/4/2004
//  Authors:        Roger Kelly
//
//  Updates:
//
// � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TaxOrderTktIssue.h"

#include <vector>

namespace tse
{

class TaxNation
{
public:
  TaxNation()
    : _memoNo(0),
      _roundingUnitNodec(0),
      _roundingUnit(0),
      _roundingRule(NONE),
      _taxRoundingOverrideInd(' '),
      _taxCollectionInd(' '),
      _taxFarequoteInd(' ')
  {
  }

  bool operator==(const TaxNation& rhs) const
  {
    return ((_taxOrderTktIssue == rhs._taxOrderTktIssue) &&
            (_taxCodeOrder == rhs._taxCodeOrder) && (_taxNationRound == rhs._taxNationRound) &&
            (_taxNationCollect == rhs._taxNationCollect) && (_nation == rhs._nation) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_lockDate == rhs._lockDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_memoNo == rhs._memoNo) &&
            (_roundingUnitNodec == rhs._roundingUnitNodec) &&
            (_roundingUnit == rhs._roundingUnit) && (_roundingRule == rhs._roundingRule) &&
            (_taxRoundingOverrideInd == rhs._taxRoundingOverrideInd) &&
            (_taxCollectionInd == rhs._taxCollectionInd) &&
            (_taxFarequoteInd == rhs._taxFarequoteInd) &&
            (_collectionNation1 == rhs._collectionNation1) &&
            (_collectionNation2 == rhs._collectionNation2) && (_message == rhs._message));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxNation& obj)
  {
    obj._taxOrderTktIssue.push_back(TaxOrderTktIssue("ABC", "000"));
    obj._taxOrderTktIssue.push_back(TaxOrderTktIssue("DEF", "AAA"));
    obj._taxCodeOrder.push_back("GHI");
    obj._taxCodeOrder.push_back("JKL");
    obj._taxNationRound.push_back("MNOP");
    obj._taxNationRound.push_back("QRST");
    obj._taxNationCollect.push_back("UVWX");
    obj._taxNationCollect.push_back("YZab");

    obj._nation = "cde";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._memoNo = 1;
    obj._roundingUnitNodec = 2;
    obj._roundingUnit = 3.333;
    obj._roundingRule = NEAREST;
    obj._taxRoundingOverrideInd = 'f';
    obj._taxCollectionInd = 'g';
    obj._taxFarequoteInd = 'h';
    obj._collectionNation1 = "ijk";
    obj._collectionNation2 = "lmn";
    obj._message = "aaaaaaaa";
  }

private:
  std::vector<TaxOrderTktIssue> _taxOrderTktIssue;
  std::vector<TaxCode> _taxCodeOrder;
  std::vector<NationCode> _taxNationRound;
  std::vector<NationCode> _taxNationCollect;
  NationCode _nation;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _lockDate;
  DateTime _effDate;
  DateTime _discDate;
  int _memoNo;
  CurrencyNoDec _roundingUnitNodec;
  RoundingFactor _roundingUnit;
  RoundingRule _roundingRule;
  char _taxRoundingOverrideInd;
  char _taxCollectionInd;
  char _taxFarequoteInd;
  NationCode _collectionNation1;
  NationCode _collectionNation2;
  TaxMessage _message;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_taxOrderTktIssue & ptr->_taxCodeOrder & ptr->_taxNationRound &
           ptr->_taxNationCollect & ptr->_nation & ptr->_createDate & ptr->_expireDate &
           ptr->_lockDate & ptr->_effDate & ptr->_discDate & ptr->_memoNo &
           ptr->_roundingUnitNodec & ptr->_roundingUnit & ptr->_roundingRule &
           ptr->_taxRoundingOverrideInd & ptr->_taxCollectionInd & ptr->_taxFarequoteInd &
           ptr->_collectionNation1 & ptr->_collectionNation2 & ptr->_message;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxOrderTktIssue);
    FLATTENIZE(archive, _taxCodeOrder);
    FLATTENIZE(archive, _taxNationRound);
    FLATTENIZE(archive, _taxNationCollect);
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _roundingUnitNodec);
    FLATTENIZE(archive, _roundingUnit);
    FLATTENIZE(archive, _roundingRule);
    FLATTENIZE(archive, _taxRoundingOverrideInd);
    FLATTENIZE(archive, _taxCollectionInd);
    FLATTENIZE(archive, _taxFarequoteInd);
    FLATTENIZE(archive, _collectionNation1);
    FLATTENIZE(archive, _collectionNation2);
    FLATTENIZE(archive, _message);
  }

  std::vector<TaxOrderTktIssue>& taxOrderTktIssue() { return _taxOrderTktIssue; }
  const std::vector<TaxOrderTktIssue>& taxOrderTktIssue() const { return _taxOrderTktIssue; }

  std::vector<TaxCode>& taxCodeOrder() { return _taxCodeOrder; }
  const std::vector<TaxCode>& taxCodeOrder() const { return _taxCodeOrder; }

  std::vector<NationCode>& taxNationRound() { return _taxNationRound; }
  const std::vector<NationCode>& taxNationRound() const { return _taxNationRound; }

  std::vector<NationCode>& taxNationCollect() { return _taxNationCollect; }
  const std::vector<NationCode>& taxNationCollect() const { return _taxNationCollect; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CurrencyNoDec& roundingUnitNodec() { return _roundingUnitNodec; }
  const CurrencyNoDec& roundingUnitNodec() const { return _roundingUnitNodec; }

  RoundingFactor& roundingUnit() { return _roundingUnit; }
  const RoundingFactor& roundingUnit() const { return _roundingUnit; }

  RoundingRule& roundingRule() { return _roundingRule; }
  const RoundingRule& roundingRule() const { return _roundingRule; }

  char& taxRoundingOverrideInd() { return _taxRoundingOverrideInd; }
  const char& taxRoundingOverrideInd() const { return _taxRoundingOverrideInd; }

  char& taxCollectionInd() { return _taxCollectionInd; }
  const char& taxCollectionInd() const { return _taxCollectionInd; }

  char& taxFarequoteInd() { return _taxFarequoteInd; }
  const char& taxFarequoteInd() const { return _taxFarequoteInd; }

  NationCode& collectionNation1() { return _collectionNation1; }
  const NationCode& collectionNation1() const { return _collectionNation1; }

  NationCode& collectionNation2() { return _collectionNation2; }
  const NationCode& collectionNation2() const { return _collectionNation2; }

  TaxMessage& message() { return _message; }
  const TaxMessage& message() const { return _message; }
};
}

