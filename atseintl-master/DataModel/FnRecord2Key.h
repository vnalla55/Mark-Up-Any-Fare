//----------------------------------------------------------------------------
////
////  Copyright Sabre 2005
////
////      The copyright to the computer program(s) herein
////      is the property of Sabre.
////      The program(s) may be used and/or copied only with
////      the written permission of Sabre or in accordance
////      with the terms and conditions stipulated in the
////      agreement/contract under which the program(s)
////      have been supplied.
////
////----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

/**
 *   @class FnRecord2Key
 *
 *   Description:
 *   FnRecord2Key is a key for matching record2 Footnote
 *       VendorCode,
 *       CarrierCode,
 *       TariffNumber;
 *       Footnote;
 *       uint32_t      _categoryNumber;
 *
 */

class FnRecord2Key
{
public:
  struct lessKey
  {
    bool operator()(FnRecord2Key* const& key1, FnRecord2Key* const& key2) const;
  };

public:
  FnRecord2Key()
    : _vendor(""),
      _carrier(""),
      _ruleTariff(0),
      _footnote(""),
      _categoryNumber(0),
      _travelDate(DateTime::emptyDate()),
      _returnDate(DateTime::emptyDate()),
      _ticketDate(DateTime::emptyDate())
  {
  }

  FnRecord2Key(const VendorCode& vendor,
               const CarrierCode& carrier,
               const TariffNumber& ruleTariff,
               const Footnote& footnote,
               uint32_t categoryNumber,
               const DateTime& travelDate,
               const DateTime& returnDate,
               const DateTime& ticketDate)
    : _vendor(vendor),
      _carrier(carrier),
      _ruleTariff(ruleTariff),
      _footnote(footnote),
      _categoryNumber(categoryNumber),
      _travelDate(travelDate),
      _returnDate(returnDate),
      _ticketDate(ticketDate)
  {
  }

  bool operator<(const FnRecord2Key& key) const
  {
    if (_categoryNumber < key._categoryNumber)
      return true;
    if (_categoryNumber > key._categoryNumber)
      return false;
    if (_ruleTariff < key._ruleTariff)
      return true;
    if (_ruleTariff > key._ruleTariff)
      return false;
    if (_footnote < key._footnote)
      return true;
    if (_footnote > key._footnote)
      return false;
    if (_carrier < key._carrier)
      return true;
    if (_carrier > key._carrier)
      return false;
    if (_vendor < key._vendor)
      return true;
    if (_vendor > key._vendor)
      return false;
    if (_travelDate.getIntRep() < key._travelDate.getIntRep())
      return true;
    if (_travelDate.getIntRep() > key._travelDate.getIntRep())
      return false;
    if (_returnDate.getIntRep() < key._returnDate.getIntRep())
      return true;
    if (_returnDate.getIntRep() > key._returnDate.getIntRep())
      return false;
    if (_ticketDate.getIntRep() < key._ticketDate.getIntRep())
      return true;
    if (_ticketDate.getIntRep() > key._ticketDate.getIntRep())
      return false;

    return false;
  }

  bool operator==(const FnRecord2Key& key) const
  {
    return _categoryNumber == key._categoryNumber && _ruleTariff == key._ruleTariff &&
           _footnote == key._footnote && _carrier == key._carrier && _vendor == key._vendor &&
           _travelDate.getIntRep() == key._travelDate.getIntRep() &&
           _returnDate.getIntRep() == key._returnDate.getIntRep() &&
           _ticketDate.getIntRep() == key._ticketDate.getIntRep();
  }

  struct Hash
  {
    size_t operator()(const FnRecord2Key& key) const
    {
      size_t hash(0);
      key.vendor().hash_combine(hash);
      key.carrier().hash_combine(hash);
      boost::hash_combine(hash, key.ruleTariff());
      key.footnote().hash_combine(hash);
      boost::hash_combine(hash, key.categoryNumber());
      boost::hash_combine(hash, key.travelDate().getIntRep());
      boost::hash_combine(hash, key.returnDate().getIntRep());
      boost::hash_combine(hash, key.ticketDate().getIntRep());

      return hash;
    }
  };

  const VendorCode& vendor() const { return _vendor; }
  const CarrierCode& carrier() const { return _carrier; }
  TariffNumber ruleTariff() const { return _ruleTariff; }
  const Footnote& footnote() const { return _footnote; }
  uint32_t categoryNumber() const { return _categoryNumber; }
  DateTime travelDate() const { return _travelDate; }
  DateTime returnDate() const { return _returnDate; }
  DateTime ticketDate() const { return _ticketDate; }

  VendorCode& vendor() { return _vendor; }
  CarrierCode& carrier() { return _carrier; }
  TariffNumber& ruleTariff() { return _ruleTariff; }
  Footnote& footnote() { return _footnote; }
  uint32_t& categoryNumber() { return _categoryNumber; }
  DateTime& travelDate() { return _travelDate; }
  DateTime& returnDate() { return _returnDate; }
  DateTime& ticketDate() { return _ticketDate; }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  Footnote _footnote;
  uint32_t _categoryNumber;
  DateTime _travelDate;
  DateTime _returnDate;
  DateTime _ticketDate;
};
}

