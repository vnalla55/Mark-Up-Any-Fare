//----------------------------------------------------------------------------
//
//    File:           BookingCodeConv.h
//    Description:    Rec 6 processing data
//    Created:        3/29/2004
//      Authors:        Roger Kelly
//
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#ifndef BOOKINGCODECONV_H
#define BOOKINGCODECONV_H

#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <iostream>

namespace tse
{
class BookingCodeConv
{
public:
  BookingCodeConv() : _ruleTariff(0), _bookingCodetblItemNo(0), _conventionNo(' '), _inhibit(' ') {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  BookConvSeq& seqNo() { return _seqNo; }
  const BookConvSeq& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& bookingCodetblItemNo() { return _bookingCodetblItemNo; }
  const int& bookingCodetblItemNo() const { return _bookingCodetblItemNo; }

  Indicator& conventionNo() { return _conventionNo; }
  const Indicator& conventionNo() const { return _conventionNo; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const BookingCodeConv& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_ruleTariff == rhs._ruleTariff) && (_rule == rhs._rule) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_bookingCodetblItemNo == rhs._bookingCodetblItemNo) &&
            (_conventionNo == rhs._conventionNo) && (_inhibit == rhs._inhibit));

#if 0

          std::cout << "[BookingCodeConv] *this = " << *this << std::endl ;
          std::cout << "[BookingCodeConv] rhs   = " << rhs   << std::endl ;
          std::cout << "[BookingCodeConv] Leaving operator== with " << ( eq ? "TRUE" : "FALSE" ) << std::endl ;

#endif

    return eq;
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const BookingCodeConv& obj)
  {
    return os << "[" << obj._vendor << "|" << obj._carrier << "|" << obj._ruleTariff << "|"
              << obj._rule << "|" << obj._seqNo << "|" << obj._expireDate << "|" << obj._createDate
              << "|" << obj._effDate << "|" << obj._discDate << "|" << obj._bookingCodetblItemNo
              << "|" << obj._conventionNo << "|" << obj._inhibit << "]";
  }

  static void dummyData(BookingCodeConv& obj)
  {
    obj.vendor() = "ABCD";
    obj.carrier() = "EFG";
    obj.ruleTariff() = 1;
    obj.rule() = "HIJK";
    obj.seqNo() = "0001";
    obj.expireDate() = time(nullptr);
    obj.createDate() = time(nullptr);
    obj.effDate() = time(nullptr);
    obj.discDate() = time(nullptr);
    obj.bookingCodetblItemNo() = 3;
    obj.conventionNo() = '2';
    obj.inhibit() = 'M';
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
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  BookConvSeq _seqNo;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  int _bookingCodetblItemNo;
  Indicator _conventionNo;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _bookingCodetblItemNo);
    FLATTENIZE(archive, _conventionNo);
    FLATTENIZE(archive, _inhibit);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_ruleTariff
           & ptr->_rule
           & ptr->_seqNo
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_bookingCodetblItemNo
           & ptr->_conventionNo
           & ptr->_inhibit;
  }

};

}// tse

#endif
