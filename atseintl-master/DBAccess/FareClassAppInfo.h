//----------------------------------------------------------------------------
//
//  File:           FareClassAppInfo.h
//  Created:        2/3/2004
//  Authors:
//
//  Description:    FareClassAppInfo class is cacheable object, which stores
//                  fields for one FareClassAppInfo record.
//
//  Updates:
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

/**
 * Record 1 - Fare Class Application
 * (Note: There are other places (besides DBAccess) where this object is instantiated.
 *        When adding new fields to this object, make sure those are updated as needed.)
 */
class FareClassAppInfo
{
public:
  FareClassAppInfo()
    : _ruleTariff(0),
      _MCN(0),
      _textTBLItemNo(0),
      _location1Type(' '),
      _location2Type(' '),
      _owrt(' '),
      _routingAppl(' '),
      _seqNo(0),
      _seasonType(' '),
      _dowType(' '),
      _pricingCatType(' '),
      _displayCatType(' '),
      _segCount(0),
      _unavailTag(' '),
      _inhibit(' ')
  {
  }

  ~FareClassAppInfo()
  {
    //  @TODO: put this code back after fixing all the places where
    //  the FareClassAppSegInfo pointers are being copied.
    //
    //  destroyChildren() ;
  }

  void destroyChildren()
  {
    for (auto& elem : _segs)
    {
      delete elem;
    }
    _segs.clear();
  }

  // mutant accessors
  const DateTime& createDate() const { return _createDate; }
  const DateTime& effDate() const { return _effectiveDate; }
  const DateTime& discDate() const { return _discDate; }
  const DateTime& expireDate() const { return _expirationDate; }

  // Added by QT: the template code for Inhibit/NotInhibit need this function
  Indicator inhibit() const { return _inhibit; }

  static const FareClassAppInfo* emptyFareClassApp()
  {
    return &_emptyFareClassApp;
  };

  VendorCode _vendor; // fare/rules vendor
  CarrierCode _carrier; // carrier code
  TariffNumber _ruleTariff; // rule tariff number
  RuleNumber _ruleNumber; // rule number
  Footnote _footnote1; // footnote #1, associated with the fare
  Footnote _footnote2; // footnote #2, associated with the fare
  FareClassCode _fareClass; // fare class code
  DateTime _effectiveDate; // first date on which record is effective.
  DateTime _discDate; // last date on which record is effective.
  DateTime _expirationDate; // last date on which record is effective.
  int _MCN;
  int _textTBLItemNo;
  LocCode _location1;
  LocCode _location2;
  LocTypeCode _location1Type;
  LocTypeCode _location2Type;
  Indicator _owrt; // one-way/round-trip indicator
  Indicator _routingAppl;
  int _seqNo; // sequence number
  RoutingNumber _routingNumber;
  FareType _fareType;
  Indicator _seasonType;
  Indicator _dowType;
  Indicator _pricingCatType;
  Indicator _displayCatType;

  int _segCount; // number of FareClassAppSegInfo records,
  Indicator _unavailTag; // segments availiability tag
  Indicator _inhibit; // Inhibit now checked at App Level
  DateTime _createDate; // Just for determining new parent
  FareClassAppSegInfoList _segs; // FareClassAppSegInfo segments, assotiated with this record

  bool operator==(const FareClassAppInfo& rhs) const
  {
    bool eq =
        ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
         (_ruleTariff == rhs._ruleTariff) && (_ruleNumber == rhs._ruleNumber) &&
         (_footnote1 == rhs._footnote1) && (_footnote2 == rhs._footnote2) &&
         (_fareClass == rhs._fareClass) && (_effectiveDate == rhs._effectiveDate) &&
         (_discDate == rhs._discDate) && (_expirationDate == rhs._expirationDate) &&
         (_MCN == rhs._MCN) && (_textTBLItemNo == rhs._textTBLItemNo) &&
         (_location1Type == rhs._location1Type) && (_location1 == rhs._location1) &&
         (_location2Type == rhs._location2Type) && (_location2 == rhs._location2) &&
         (_owrt == rhs._owrt) && (_routingAppl == rhs._routingAppl) &&
         (_routingNumber == rhs._routingNumber) && (_fareType == rhs._fareType) &&
         (_seasonType == rhs._seasonType) && (_dowType == rhs._dowType) &&
         (_pricingCatType == rhs._pricingCatType) && (_displayCatType == rhs._displayCatType) &&
         (_seqNo == rhs._seqNo) && (_unavailTag == rhs._unavailTag) &&
         (_segCount == rhs._segCount) && (_createDate == rhs._createDate) &&
         (_inhibit == rhs._inhibit) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(FareClassAppInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._ruleTariff = 1;
    obj._ruleNumber = "HIJK";
    obj._footnote1 = "LM";
    obj._footnote2 = "NO";
    obj._fareClass = "1A2B3C4D";
    obj._effectiveDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expirationDate = time(nullptr);
    obj._MCN = 2;
    obj._textTBLItemNo = 3;
    obj._location1Type = 'P';
    obj._location1 = "LocCode1";
    obj._location2Type = 'Y';
    obj._location2 = "LocCode2";
    obj._owrt = 'Z';
    obj._routingAppl = 'a';
    obj._routingNumber = "bcde";
    obj._fareType = "efghijkl";
    obj._seasonType = 'm';
    obj._dowType = 'n';
    obj._pricingCatType = 'o';
    obj._displayCatType = 'p';
    obj._seqNo = 4;
    obj._unavailTag = 'q';
    obj._segCount = 5;
    obj._createDate = time(nullptr);
    obj._inhibit = 'r';

    FareClassAppSegInfo* fcasi1 = new FareClassAppSegInfo();
    FareClassAppSegInfo* fcasi2 = new FareClassAppSegInfo();

    FareClassAppSegInfo::dummyData(*fcasi1);
    FareClassAppSegInfo::dummyData(*fcasi2);

    obj._segs.push_back(fcasi1);
    obj._segs.push_back(fcasi2);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _ruleNumber);
    FLATTENIZE(archive, _footnote1);
    FLATTENIZE(archive, _footnote2);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _effectiveDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expirationDate);
    FLATTENIZE(archive, _MCN);
    FLATTENIZE(archive, _textTBLItemNo);
    FLATTENIZE(archive, _location1Type);
    FLATTENIZE(archive, _location1);
    FLATTENIZE(archive, _location2Type);
    FLATTENIZE(archive, _location2);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _routingAppl);
    FLATTENIZE(archive, _routingNumber);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _seasonType);
    FLATTENIZE(archive, _dowType);
    FLATTENIZE(archive, _pricingCatType);
    FLATTENIZE(archive, _displayCatType);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _segCount);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _segs);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  static const FareClassAppInfo _emptyFareClassApp;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_carrier & ptr->_ruleTariff & ptr->_ruleNumber &
           ptr->_footnote1 & ptr->_footnote2 & ptr->_fareClass & ptr->_effectiveDate &
           ptr->_discDate & ptr->_expirationDate & ptr->_MCN & ptr->_textTBLItemNo &
           ptr->_location1Type & ptr->_location1 & ptr->_location2Type & ptr->_location2 &
           ptr->_owrt & ptr->_routingAppl & ptr->_routingNumber & ptr->_fareType &
           ptr->_seasonType & ptr->_dowType & ptr->_pricingCatType & ptr->_displayCatType &
           ptr->_seqNo & ptr->_unavailTag & ptr->_segCount & ptr->_createDate & ptr->_inhibit &
           ptr->_segs;
  }

  FareClassAppInfo(const FareClassAppInfo&);
  FareClassAppInfo& operator=(const FareClassAppInfo&);
};

} // namespace tse

