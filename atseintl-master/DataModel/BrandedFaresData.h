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

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{

struct BrandedFareInfo
{
  Code<10> brandId; // brand ID
  std::string programId; // program ID
  std::vector<BookingCode> fareBookingCode; // save for BRN
  std::vector<BookingCode> fareSecondaryBookingCode; // save for secondary BRN
  std::vector<BookingCode> fareBookingCodeExclude; // save for BRN with PXY = 'T'
  std::vector<BookingCode> fareSecondaryBookingCodeExclude; // save for BRN with PXY = 'T'
  std::map<BookingCode, char> fareBookingCodeData; // bookingcode and fare selector indicator
  std::map<BookingCode, char> fareSecondaryBookingCodeData;

  std::vector<BookingCode> fareBookingCodeDispDiag499; // list of BKCodes already displayed by
                                                       // diagnopstic 499

  std::vector<FareClassCode> fareFamily; // BRN
  std::vector<FareClassCode> fareFamilyExclude; // save for BRN with PXY = 'T'
  std::map<FareClassCode, char> fareFamilyData; // fare family and fare selector indicator

  std::vector<FareClassCode> fareBasisCode; // BRN
  std::vector<FareClassCode> fareBasisCodeExclude; // save for BRN with PXY = 'T'
  std::map<FareClassCode, char> fareBasisCodeData; // fare basis code and fare selector indicator

  BrandedFareInfo() {}
};

class BrandedFaresData : private std::map<uint16_t, BrandedFareInfo>
{
public:
  using std::map<uint16_t, BrandedFareInfo>::const_iterator;
  using std::map<uint16_t, BrandedFareInfo>::iterator;
  using std::map<uint16_t, BrandedFareInfo>::begin;
  using std::map<uint16_t, BrandedFareInfo>::end;
  using std::map<uint16_t, BrandedFareInfo>::value_type;

  const uint16_t getSize() const { return static_cast<uint16_t>(size()); }

  Code<10>& brandId(const uint16_t index = 0) { return at(index).brandId; }
  const Code<10>& brandId(const uint16_t index = 0) const { return at(index).brandId; }

  std::string& programId(const uint16_t index = 0) { return at(index).programId; }
  const std::string& programId(const uint16_t index = 0) const { return at(index).programId; }

  std::vector<BookingCode>& fareBookingCode(const uint16_t index = 0)
  {
    return at(index).fareBookingCode;
  }
  const std::vector<BookingCode>& fareBookingCode(const uint16_t index = 0) const
  {
    return at(index).fareBookingCode;
  }

  std::vector<BookingCode>& fareSecondaryBookingCode(const uint16_t index = 0)
  {
    return at(index).fareSecondaryBookingCode;
  }
  const std::vector<BookingCode>& fareSecondaryBookingCode(const uint16_t index = 0) const
  {
    return at(index).fareSecondaryBookingCode;
  }

  std::vector<BookingCode>& fareBookingCodeExclude(const uint16_t index = 0)
  {
    return at(index).fareBookingCodeExclude;
  }
  const std::vector<BookingCode>& fareBookingCodeExclude(const uint16_t index = 0) const
  {
    return at(index).fareBookingCodeExclude;
  }

  std::vector<BookingCode>& fareSecondaryBookingCodeExclude(const uint16_t index = 0)
  {
    return at(index).fareSecondaryBookingCodeExclude;
  }
  const std::vector<BookingCode>& fareSecondaryBookingCodeExclude(const uint16_t index = 0) const
  {
    return at(index).fareSecondaryBookingCodeExclude;
  }

  std::map<BookingCode, char>& fareBookingCodeData(const uint16_t index = 0)
  {
    return at(index).fareBookingCodeData;
  }
  const std::map<BookingCode, char>& fareBookingCodeData(const uint16_t index = 0) const
  {
    return at(index).fareBookingCodeData;
  }
  const char getFareBookingCodeIndicator(const BookingCode&, const uint16_t index = 0) const;

  std::map<BookingCode, char>& fareSecondaryBookingCodeData(const uint16_t index = 0)
  {
    return at(index).fareSecondaryBookingCodeData;
  }
  const std::map<BookingCode, char>& fareSecondaryBookingCodeData(const uint16_t index = 0) const
  {
    return at(index).fareSecondaryBookingCodeData;
  }

  std::vector<BookingCode>& fareBookingCodeDispDiag499(const uint16_t index = 0)
  {
    return at(index).fareBookingCodeDispDiag499;
  }
  const std::vector<BookingCode>& fareBookingCodeDispDiag499(const uint16_t index = 0) const
  {
    return at(index).fareBookingCodeDispDiag499;
  }

  std::vector<FareClassCode>& fareFamily(const uint16_t index = 0) { return at(index).fareFamily; }
  const std::vector<FareClassCode>& fareFamily(const uint16_t index = 0) const
  {
    return at(index).fareFamily;
  }

  std::vector<FareClassCode>& fareFamilyExclude(const uint16_t index = 0)
  {
    return at(index).fareFamilyExclude;
  }
  const std::vector<FareClassCode>& fareFamilyExclude(const uint16_t index = 0) const
  {
    return at(index).fareFamilyExclude;
  }

  std::map<FareClassCode, char>& fareFamilyData(const uint16_t index = 0)
  {
    return at(index).fareFamilyData;
  }
  const std::map<FareClassCode, char>& fareFamilyData(const uint16_t index = 0) const
  {
    return at(index).fareFamilyData;
  }
  const char getFareFamilyIndicator(const FareClassCode&, const uint16_t index = 0) const;

  std::vector<FareClassCode>& fareBasisCode(const uint16_t index = 0)
  {
    return at(index).fareBasisCode;
  }
  const std::vector<FareClassCode>& fareBasisCode(const uint16_t index = 0) const
  {
    return at(index).fareBasisCode;
  }

  std::vector<FareClassCode>& fareBasisCodeExclude(const uint16_t index = 0)
  {
    return at(index).fareBasisCodeExclude;
  }
  const std::vector<FareClassCode>& fareBasisCodeExclude(const uint16_t index = 0) const
  {
    return at(index).fareBasisCodeExclude;
  }

  std::map<FareClassCode, char>& fareBasisCodeData(const uint16_t index = 0)
  {
    return at(index).fareBasisCodeData;
  }
  const std::map<FareClassCode, char>& fareBasisCodeData(const uint16_t index = 0) const
  {
    return at(index).fareBasisCodeData;
  }
  const char getFareBasisCodeIndicator(const FareClassCode&, const uint16_t index = 0) const;

  bool hasConsistentBookingCodes() const;

private:
  const BrandedFareInfo& at(const uint16_t index) const;
  BrandedFareInfo& at(const uint16_t index) { return (*this)[index]; }
};

} // tse

