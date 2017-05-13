//----------------------------------------------------------------------------
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

namespace tse
{
class FareDisplayTrx;

class InclusionCodePaxType
{
public:
  virtual ~InclusionCodePaxType() {}
  virtual void getPaxType(FareDisplayTrx& trx) = 0;
  static InclusionCodePaxType* getInclusionCodePaxType(FareDisplayTrx& trx);

  void getChildPaxTypes(FareDisplayTrx& trx) const;
  void getInfantPaxTypes(FareDisplayTrx& trx) const;
  void getDiscountPaxTypes(FareDisplayTrx& trx) const;
  void getDefaultPaxCode(FareDisplayTrx& trx) const;
  void addRequestedPaxTypes(FareDisplayTrx& trx) const;
  void getAdditionalPaxType(FareDisplayTrx& trx) const;
};
}

