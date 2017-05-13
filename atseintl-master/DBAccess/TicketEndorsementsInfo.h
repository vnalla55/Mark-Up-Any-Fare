//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class TicketEndorsementsInfo : public RuleItemInfo
{
public:
  TicketEndorsementsInfo() : _unavailTag(' '), _priorityCode(0), _tktLocInd(' '), _inhibit(' ') {}

  virtual ~TicketEndorsementsInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& priorityCode() { return _priorityCode; }
  const int& priorityCode() const { return _priorityCode; }

  std::string& endorsementTxt() { return _endorsementTxt; }
  const std::string& endorsementTxt() const { return _endorsementTxt; }

  Indicator& tktLocInd() { return _tktLocInd; }
  const Indicator& tktLocInd() const { return _tktLocInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const TicketEndorsementsInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
            (_priorityCode == rhs._priorityCode) && (_endorsementTxt == rhs._endorsementTxt) &&
            (_tktLocInd == rhs._tktLocInd) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(TicketEndorsementsInfo& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._priorityCode = 1;
    obj._endorsementTxt = "aaaaaaaa";
    obj._tktLocInd = 'B';
    obj._inhibit = 'C';
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
  int _priorityCode;
  std::string _endorsementTxt;
  Indicator _tktLocInd;
  Indicator _inhibit;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _priorityCode);
    FLATTENIZE(archive, _endorsementTxt);
    FLATTENIZE(archive, _tktLocInd);
    FLATTENIZE(archive, _inhibit);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailTag
           & ptr->_priorityCode
           & ptr->_endorsementTxt
           & ptr->_tktLocInd
           & ptr->_inhibit;
  }
};
}
