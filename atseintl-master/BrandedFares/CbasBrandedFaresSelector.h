//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.h
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "BrandedFares/BrandedFareDiagnostics.h"
#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/BrandedFaresValidator.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"


#include <vector>

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class BrandProgram;
class BrandInfo;

class FareBasisCodeStructure
{
public:
  FareBasisCodeStructure() {}
  FareBasisCodeStructure(const std::string& fareBasisCode)
  : _fareBasisCode(fareBasisCode)
  {
    std::string::size_type pos = fareBasisCode.find('/');
    if (pos == std::string::npos)
    {
      _fareBasis = fareBasisCode;
    }
    else
    {
      _ticketDesignator.assign(_fareBasisCode, pos + 1, _fareBasisCode.size() - (pos + 1));
      _fareBasis.assign(_fareBasisCode, 0, pos);
    }
  }
  FareBasisCodeStructure(const std::string& fareBasisCode, const std::string& fareBasisOnly, const std::string& ticketDesignator)
  : _fareBasisCode(fareBasisCode), _fareBasis(fareBasisOnly), _ticketDesignator(ticketDesignator)
  {}

  bool operator<(const FareBasisCodeStructure& other) const { return _fareBasisCode < other.getFareBasisCode(); }
  bool operator==(const FareBasisCodeStructure& other) const { return _fareBasisCode == other.getFareBasisCode(); }

  operator const std::string& () const { return _fareBasisCode;}
  const std::string& getFareBasisCode() const { return _fareBasisCode;}
  const std::string& getFareBasis() const { return _fareBasis; }
  const std::string& getTicketDesignator() const { return _ticketDesignator;}

private:
  std::string _fareBasisCode;
  std::string _fareBasis;
  std::string _ticketDesignator;
};

class CbasBrandedFareValidator : public BrandedFaresValidator
{
  friend class CbasBrandedFaresValidatorTest;
  PricingTrx& _trx;
  class BookingCodeComparator
  {
    const BookingCode _code;
  public:
    BookingCodeComparator(const BookingCode& code)
    : _code(std::string(1, code[0]))
    {}
    bool operator()(const BookingCode& code) const
    {
      if(!code.empty() > 0)
        return _code[0] == code[0];
      return false;
    }
  };
public:
  CbasBrandedFareValidator(PricingTrx& trx) : _trx(trx)
  {}
  virtual PaxTypeFare::BrandStatus validateFare(const PaxTypeFare* paxTypeFare,
                                                const BrandProgram* brandPr,
                                                const BrandInfo* brand,
                                                bool& needBrandSeparator,
                                                BrandedFareDiagnostics& diagnostics,
                                                bool skipHardPassValidation = false) const override;

protected:
  PaxTypeFare::BrandStatus validateFare(const FareBasisCodeStructure& ptfFareBasisCode,
                                        const std::vector<BookingCode>& bookingCodeVec,
                                        const BrandInfo& brand,
                                        BrandedFareDiagnostics& diagnostics,
                                        bool skipHardPassValidation = false) const;

  PaxTypeFare::BrandStatus
  matchBasedOnFareBasisCode(const FareBasisCodeStructure& ptfFareBasisCode,
                            const std::vector<FareBasisCode>& fareBasisCodes,
                            BrandedFareDiagnostics& diagnostics) const;

  bool isMatchedFareBasisCode(const FareBasisCodeStructure& ptfFareBasisCode,
                              const FareBasisCode& matchingFareBasisCode,
                              BrandedFareDiagnostics& diagnostics) const;

  PaxTypeFare::BrandStatus
  matchBasedOnBookingCode(const std::vector<BookingCode>& fareBookingCodes,
                          const std::vector<BookingCode>& primaryBookingCodes,
                          const std::vector<BookingCode>& secondaryBookingCodes,
                          BrandedFareDiagnostics& diagnostics,
                          bool skipHardPassValidation = false) const;

  bool isBookingCodeMatched(const std::vector<BookingCode>& primeBookingCodeVec,
                            const std::vector<BookingCode>& bookingCodes,
                            BrandedFareDiagnostics& diagnostics) const;

  bool isFamilyFareBasisCode(const std::string& matchingFareBasis) const;

  bool isMatchingNonExactFareBasis(const std::string& ptfFareBasis,
                                   const std::string& matchingFareBasis,
                                   BrandedFareDiagnostics& diagnostics ) const;

  void handleFareBasisCodeWorkaround(const BrandInfo& brand,
                                     std::vector<BookingCode>& primaryBookingCodes,
                                     std::vector<BookingCode>& secondaryBookingCodes,
                                     std::vector<FareBasisCode>& fareBasisCodes) const;
};

} // tse

