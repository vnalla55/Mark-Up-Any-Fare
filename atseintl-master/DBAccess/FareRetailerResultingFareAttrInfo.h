#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"


namespace tse
{

class FareRetailerResultingFareAttrInfo
{
 public:

  FareRetailerResultingFareAttrInfo()
    : _resultingFareAttrItemNo(0)
    , _redistributeInd(' ')
    , _updateInd(' ')
    , _sellInd(' ')
    , _ticketInd(' ')
    , _viewNetInd(' ')
    , _accountCdType(' ')
  {
  }

  uint64_t&     resultingFareAttrItemNo() { return _resultingFareAttrItemNo; }
  uint64_t     resultingFareAttrItemNo() const { return _resultingFareAttrItemNo; }

  Indicator&     redistributeInd() { return _redistributeInd; }
  Indicator     redistributeInd() const { return _redistributeInd; }

  Indicator&     updateInd() { return _updateInd; }
  Indicator     updateInd() const { return _updateInd; }

  Indicator&     sellInd() { return _sellInd; }
  Indicator     sellInd() const { return _sellInd; }

  Indicator&     ticketInd() { return _ticketInd; }
  Indicator     ticketInd() const { return _ticketInd; }

  Indicator&     viewNetInd() { return _viewNetInd; }
  Indicator     viewNetInd() const { return _viewNetInd; }

  TktDesignator&     tktDesignator() { return _tktDesignator; }
  const TktDesignator&     tktDesignator() const { return _tktDesignator; }

  Indicator&     accountCdType() { return _accountCdType; }
  Indicator     accountCdType() const { return _accountCdType; }

  AccountCode&     accountCd() { return _accountCd; }
  const AccountCode&     accountCd() const { return _accountCd; }

  DateTime& createDate() { return _createDate; }
  DateTime createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  DateTime expireDate() const { return _expireDate; }

  DateTime& lastModDate() { return _lastModDate; }
  DateTime lastModDate() const { return _lastModDate; }

  bool operator==(const FareRetailerResultingFareAttrInfo& rhs) const
  {
    return  _resultingFareAttrItemNo == rhs._resultingFareAttrItemNo
            && _redistributeInd == rhs._redistributeInd
            && _updateInd == rhs._updateInd
            && _sellInd == rhs._sellInd
            && _ticketInd == rhs._ticketInd
            && _viewNetInd == rhs._viewNetInd
            && _tktDesignator == rhs._tktDesignator
            && _accountCdType == rhs._accountCdType
            && _accountCd == rhs._accountCd
            && _createDate == rhs._createDate
            && _expireDate == rhs._expireDate
            && _lastModDate == rhs._lastModDate;
  }

  static void dummyData(FareRetailerResultingFareAttrInfo& obj)
  {
    obj._resultingFareAttrItemNo = 111;
    obj._redistributeInd = 'Y';
    obj._updateInd = 'Y';
    obj._sellInd = 'Y';
    obj._ticketInd = 'Y';
    obj._viewNetInd = 'Y';
    obj._tktDesignator = "ABC35";
    obj._accountCdType = 'A';
    obj._accountCd = "CC123456";
    obj._createDate = std::time(nullptr);
    obj._expireDate = std::time(nullptr);
    obj._lastModDate = std::time(nullptr);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _resultingFareAttrItemNo);
    FLATTENIZE(archive, _redistributeInd);
    FLATTENIZE(archive, _updateInd);
    FLATTENIZE(archive,  _sellInd);
    FLATTENIZE(archive, _ticketInd);
    FLATTENIZE(archive, _viewNetInd);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _accountCdType);
    FLATTENIZE(archive, _accountCd);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lastModDate);
  }

 private:

  uint64_t    _resultingFareAttrItemNo;
  Indicator   _redistributeInd;
  Indicator   _updateInd;
  Indicator   _sellInd;
  Indicator   _ticketInd;
  Indicator   _viewNetInd;
  TktDesignator _tktDesignator;
  Indicator   _accountCdType;
  AccountCode _accountCd;
  DateTime    _createDate;
  DateTime    _expireDate;
  DateTime    _lastModDate;

};

}// tse



