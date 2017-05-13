//----------------------------------------------------------------------------
//   2012, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/MarkupCalculate.h"

#include <vector>

#define MARKUP_CONTROL_MEMBERS                                                                     \
  (_vendor)(_carrier)(_ruleTariff)(_rule)(_seqNo)(_creatorPseudoCity)(_markupType)(                \
      _ownerPseudoCityType)(_ownerPseudoCity)(_createDate)(_expireDate)(_requestDate)(             \
      _secondarySellerId)(_accountCode)(_ownerId)(_redistributeTag)(_updateTag)(_sellTag)(         \
      _tktTag)(_viewNetInd)(_status)(_calcs)

namespace tse
{
class MarkupControl
{
public:
  MarkupControl()
    : _ruleTariff(0),
      _seqNo(0),
      _markupType(' '),
      _ownerPseudoCityType(' '),
      _secondarySellerId(0),
      _redistributeTag(' '),
      _updateTag(' '),
      _sellTag(' '),
      _tktTag(' '),
      _viewNetInd(' '),
      _status(' ')
  {
  }

  ~MarkupControl()
  {
    std::vector<MarkupCalculate*>::iterator calcsIt;
    for (calcsIt = _calcs.begin(); calcsIt != _calcs.end(); calcsIt++)
    { // Nuke the Children
      delete *calcsIt;
    }
  }

  // Primary Key
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  PseudoCityCode& creatorPseudoCity() { return _creatorPseudoCity; }
  const PseudoCityCode& creatorPseudoCity() const { return _creatorPseudoCity; }

  Indicator& markupType() { return _markupType; }
  const Indicator& markupType() const { return _markupType; }

  Indicator& ownerPseudoCityType() { return _ownerPseudoCityType; }
  const Indicator& ownerPseudoCityType() const { return _ownerPseudoCityType; }

  PseudoCityCode& ownerPseudoCity() { return _ownerPseudoCity; }
  const PseudoCityCode& ownerPseudoCity() const { return _ownerPseudoCity; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  // Non-PK fields
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& requestDate() { return _requestDate; }
  const DateTime& requestDate() const { return _requestDate; }

  long& secondarySellerId() { return _secondarySellerId; }
  const long& secondarySellerId() const { return _secondarySellerId; }

  AccountCode& accountCode() { return _accountCode; }
  const AccountCode& accountCode() const { return _accountCode; }

  std::string& ownerId() { return _ownerId; }
  const std::string& ownerId() const { return _ownerId; }

  Indicator& redistributeTag() { return _redistributeTag; }
  const Indicator& redistributeTag() const { return _redistributeTag; }

  Indicator& updateTag() { return _updateTag; }
  const Indicator& updateTag() const { return _updateTag; }

  Indicator& sellTag() { return _sellTag; }
  const Indicator& sellTag() const { return _sellTag; }

  Indicator& tktTag() { return _tktTag; }
  const Indicator& tktTag() const { return _tktTag; }

  Indicator& viewNetInd() { return _viewNetInd; }
  const Indicator& viewNetInd() const { return _viewNetInd; }

  Indicator& status() { return _status; }
  const Indicator& status() const { return _status; }

  std::vector<MarkupCalculate*>& calcs() { return _calcs; }
  const std::vector<MarkupCalculate*>& calcs() const { return _calcs; }

  bool operator==(const MarkupControl& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_ruleTariff == rhs._ruleTariff) && (_rule == rhs._rule) && (_seqNo == rhs._seqNo) &&
            (_creatorPseudoCity == rhs._creatorPseudoCity) && (_markupType == rhs._markupType) &&
            (_ownerPseudoCityType == rhs._ownerPseudoCityType) &&
            (_ownerPseudoCity == rhs._ownerPseudoCity) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_requestDate == rhs._requestDate) &&
            (_secondarySellerId == rhs._secondarySellerId) && (_accountCode == rhs._accountCode) &&
            (_ownerId == rhs._ownerId) && (_redistributeTag == rhs._redistributeTag) &&
            (_updateTag == rhs._updateTag) && (_sellTag == rhs._sellTag) &&
            (_tktTag == rhs._tktTag) && (_viewNetInd == rhs._viewNetInd) &&
            (_status == rhs._status) && (_calcs.size() == rhs._calcs.size()));

    for (size_t i = 0; (eq && (i < _calcs.size())); ++i)
    {
      eq = (*(_calcs[i]) == *(rhs._calcs[i]));
    }

    return eq;
  }

  static void dummyData(MarkupControl& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._ruleTariff = 1;
    obj._rule = "HIJK";
    obj._seqNo = 2;
    obj._creatorPseudoCity = "LMNOP";
    obj._markupType = 'Q';
    obj._ownerPseudoCityType = 'R';
    obj._ownerPseudoCity = "STUVW";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._requestDate = time(nullptr);
    obj._secondarySellerId = 3;
    obj._accountCode = "aaaaaaaa";
    obj._ownerId = "bbbbbbbb";
    obj._redistributeTag = 'X';
    obj._updateTag = 'Y';
    obj._sellTag = 'Z';
    obj._tktTag = 'a';
    obj._viewNetInd = 'b';
    obj._status = 'c';

    MarkupCalculate* mc1 = new MarkupCalculate;
    MarkupCalculate* mc2 = new MarkupCalculate;

    MarkupCalculate::dummyData(*mc1);
    MarkupCalculate::dummyData(*mc2);

    obj._calcs.push_back(mc1);
    obj._calcs.push_back(mc2);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  // Primary Key
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  int _seqNo;
  PseudoCityCode _creatorPseudoCity;
  Indicator _markupType;
  Indicator _ownerPseudoCityType;
  PseudoCityCode _ownerPseudoCity;
  DateTime _createDate;

  // Non-PK fields
  DateTime _expireDate;
  DateTime _requestDate;
  long _secondarySellerId;
  AccountCode _accountCode;
  std::string _ownerId;
  Indicator _redistributeTag;
  Indicator _updateTag;
  Indicator _sellTag;
  Indicator _tktTag;
  Indicator _viewNetInd;
  Indicator _status;

  // Children
  std::vector<MarkupCalculate*> _calcs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
#define READ_WRITE(r, d, member) FLATTENIZE(archive, member);
    BOOST_PP_SEQ_FOR_EACH(READ_WRITE, BOOST_PP_EMPTY(), MARKUP_CONTROL_MEMBERS)
#undef READ_WRITE
  }

protected:
private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_carrier & ptr->_ruleTariff & ptr->_rule & ptr->_seqNo &
           ptr->_creatorPseudoCity & ptr->_markupType & ptr->_ownerPseudoCityType &
           ptr->_ownerPseudoCity & ptr->_createDate & ptr->_expireDate & ptr->_requestDate &
           ptr->_secondarySellerId & ptr->_accountCode & ptr->_ownerId & ptr->_redistributeTag &
           ptr->_updateTag & ptr->_sellTag & ptr->_tktTag & ptr->_viewNetInd & ptr->_status &
           ptr->_calcs;
  }
  MarkupControl(const MarkupControl&);
  MarkupControl& operator=(const MarkupControl&);
};
}