//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "DataModel/BrandedFaresData.h"


namespace tse
{

namespace
{

template <typename T>
inline char
getIndicator(const std::map<T, char>& collection, const T& code)
{
  typename std::map<T, char>::const_iterator i = collection.find(code);
  if (i != collection.end())
    return i->second;

  return ' ';
}

} // namespace

const char
BrandedFaresData::getFareBookingCodeIndicator(const BookingCode& bkkCode, const uint16_t index)
    const
{
  return getIndicator(at(index).fareBookingCodeData, bkkCode);
}

const char
BrandedFaresData::getFareFamilyIndicator(const FareClassCode& ff, const uint16_t index) const
{
  return getIndicator(at(index).fareFamilyData, ff);
}

const char
BrandedFaresData::getFareBasisCodeIndicator(const FareClassCode& fbc, const uint16_t index) const
{
  return getIndicator(at(index).fareBasisCodeData, fbc);
}

const BrandedFareInfo&
BrandedFaresData::at(const uint16_t index) const
{
  const_iterator bfi = find(index);
  if (bfi != end())
    return bfi->second;

  static const BrandedFareInfo empty;
  return empty;
}

bool
BrandedFaresData::hasConsistentBookingCodes() const
{
  for (const value_type& info : *this)
  {
    if (std::find_first_of(info.second.fareBookingCode.begin(),
                           info.second.fareBookingCode.end(),
                           info.second.fareSecondaryBookingCode.begin(),
                           info.second.fareSecondaryBookingCode.end()) !=
        info.second.fareBookingCode.end())
      return false;
  }
  return true;
}

} // tse
