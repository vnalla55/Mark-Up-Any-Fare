// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

namespace tax
{

struct TaxDetailsLevel // determines which details should be added in the response
{
  bool calc;
  bool geo;
  bool taxOnTax;
  bool taxOnFare;
  bool taxOnYqYr;
  bool taxOnOc;
  bool taxOnExchangeReissue;

  TaxDetailsLevel()
    : calc(false)
    , geo(false)
    , taxOnTax(false)
    , taxOnFare(false)
    , taxOnYqYr(false)
    , taxOnOc(false)
    , taxOnExchangeReissue(false)
  {
  }

  static TaxDetailsLevel all()
  {
    TaxDetailsLevel ans;
    ans.calc = true;
    ans.geo = true;
    ans.taxOnTax = true;
    ans.taxOnFare = true;
    ans.taxOnYqYr = true;
    ans.taxOnOc = true;
    ans.taxOnExchangeReissue = true;
    return ans;
  }
};

} // namespace tax

