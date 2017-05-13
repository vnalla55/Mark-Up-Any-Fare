//----------------------------------------------------------------------------
//
//  File:           FareClassAppSegInfoQualifier.h
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "FareDisplay/Qualifier.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;
class CarrierApplicationInfo;

class FareClassAppSegInfoQualifier : public Qualifier
{
public:
  FareClassAppSegInfoQualifier();
  virtual ~FareClassAppSegInfoQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplList(DataHandle& dh, const VendorCode& vendor, int itemNo) const;
  bool setup(FareDisplayTrx& trx) override;
};
} // namespace tse

