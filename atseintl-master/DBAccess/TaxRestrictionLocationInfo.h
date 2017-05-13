//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxRestrictionLocationInfo
{
public:
  TaxRestrictionLocationInfo() {}

  struct TaxRestrictionLocationInfoSeq
  {
  public:
    int& seqNo() { return _seqNo; }
    const int& seqNo() const { return _seqNo; }

    Indicator& saleIssueInd() { return _saleIssueInd; }
    const Indicator& saleIssueInd() const { return _saleIssueInd; }

    Indicator& inclExclInd() { return _inclExclInd; }
    const Indicator& inclExclInd() const { return _inclExclInd; }

    Indicator& locType() { return _locType; }
    const Indicator& locType() const { return _locType; }

    LocCode& loc() { return _loc; }
    const LocCode& loc() const { return _loc; }

    bool operator==(const TaxRestrictionLocationInfoSeq& rhs) const
    {
      return ((_seqNo == rhs._seqNo) && (_saleIssueInd == rhs._saleIssueInd) &&
              (_inclExclInd == rhs._inclExclInd) && (_locType == rhs._locType) &&
              (_loc == rhs._loc));
    }

    static void dummyData(TaxRestrictionLocationInfoSeq& obj)
    {
      obj._seqNo = 1;
      obj._saleIssueInd = 'N';
      obj._inclExclInd = 'I';
      obj._locType = 'J';
      obj._loc = "KGHHYH";
    }

    template <typename B, typename T>
    static B& convert(B& buffer, T ptr)
    {
      return buffer & ptr->_seqNo & ptr->_saleIssueInd & ptr->_inclExclInd & ptr->_locType &
             ptr->_loc;
    }

  private:
    int _seqNo;
    Indicator _saleIssueInd;
    Indicator _inclExclInd;
    Indicator _locType;
    LocCode _loc;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _seqNo);
      FLATTENIZE(archive, _saleIssueInd);
      FLATTENIZE(archive, _inclExclInd);
      FLATTENIZE(archive, _locType);
      FLATTENIZE(archive, _loc);
    }
  };

  TaxRestrictionLocation& location() { return _location; }
  const TaxRestrictionLocation& location() const { return _location; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::vector<TaxRestrictionLocationInfoSeq>& seqs() { return _seqs; }
  const std::vector<TaxRestrictionLocationInfoSeq>& seqs() const { return _seqs; }

  bool operator==(const TaxRestrictionLocationInfo& rhs) const
  {
    return ((_location == rhs._location) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxRestrictionLocationInfo& obj)
  {
    obj._location = "EFGHIJK";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

private:
  TaxRestrictionLocation _location;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  std::vector<TaxRestrictionLocationInfoSeq> _seqs;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_location & ptr->_expireDate & ptr->_createDate & ptr->_effDate &
           ptr->_discDate;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _location);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _seqs);
  }
};
}

