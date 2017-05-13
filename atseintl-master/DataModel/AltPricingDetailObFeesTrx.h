//-------------------------------------------------------------------
//
//  File:        AltPricingDetailTrxObFees.h
//  Authors:
//
//  Description: Alternative Pricing Detail(WPA*n) Transaction object with ObFees info
//
//
//  Copyright Sabre 2015
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

#include "DataModel/AltPricingTrxData.h"
#include "DataModel/TktFeesPricingTrx.h"

namespace tse
{
class PaxDetail;
class Billing;

class AltPricingDetailObFeesTrx : public TktFeesPricingTrx, public AltPricingTrxData
{
public:
  std::vector<PaxDetail*>& paxDetails() { return _paxDetails; }
  const std::vector<PaxDetail*>& paxDetails() const { return _paxDetails; }

  bool& rebook() { return _rebookRequested; }
  bool rebook() const { return _rebookRequested; }

  bool convert(std::string& response) override;

  std::string& vendorCrsCode() { return _vendorCrsCode; }
  const std::string& vendorCrsCode() const { return _vendorCrsCode; }

  using PricingTrx::billing;
  const Billing* billing() const override { return nullptr; }

private:
  bool _rebookRequested = false;
  std::string _vendorCrsCode;
  std::vector<PaxDetail*> _paxDetails;
};
} // tse namespace
