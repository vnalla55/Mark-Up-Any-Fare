// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

namespace tse
{
class PricingTrx;
class ItinSelector;

class CopyTaxResponse
{
public:
  CopyTaxResponse(PricingTrx& trx, const ItinSelector& itinSelector);
  CopyTaxResponse(const CopyTaxResponse&) = delete;
  CopyTaxResponse& operator=(const CopyTaxResponse&) = delete;
  CopyTaxResponse(CopyTaxResponse&&) = delete;
  CopyTaxResponse& operator=(CopyTaxResponse&&) = delete;
  bool isCopyToExcItin() const;
  bool isCopyToNewItin() const;
  void copyToExcItin();
  void copyToNewItin();

private:
  PricingTrx& _trx;
  const ItinSelector& _itinSelector;

  bool checkItin(Itin*) const;
};

} // end of tse namespace
