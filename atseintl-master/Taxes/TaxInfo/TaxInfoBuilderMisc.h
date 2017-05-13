//----------------------------------------------------------------------------
//  File:           TaxInfoBuilderMisc.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    PfcInfoBuilderMisc header file for ATSE V2 Tax Info Project.
//                  Misc transportation taxes builder.
//
//  Copyright Sabre 2008
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

#include "Taxes/TaxInfo/TaxInfoBuilder.h"

namespace tse
{

class TaxInfoBuilderMisc : public TaxInfoBuilder
{
  friend class TaxInfoBuilderMiscTest;

public:
  typedef TaxInfoResponse Response;

  TaxInfoBuilderMisc();
  virtual ~TaxInfoBuilderMisc();

protected:
  void buildDetails(TaxTrx& trx) override;
  bool validateTax(TaxTrx& trx) override;
  bool validateOriginRestriction(TaxTrx& trx, TaxCodeReg& taxCodeReg);
  bool validatePosPoi(TaxTrx& trx, Indicator& exclInd, LocType& locType, LocCode& loc);
  bool validateAirport(TaxTrx& trx) override;
  bool isInZone(TaxTrx& trx, LocCode& loc);

private:
  LocCode getCustomerCity(TaxTrx& trx);
};

} // namespace tse

