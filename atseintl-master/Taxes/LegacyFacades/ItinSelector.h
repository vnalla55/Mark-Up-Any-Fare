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

#include <vector>

namespace tse
{
class PricingTrx;

class ItinSelector
{
  PricingTrx& _trx;
  bool _isExcItin;
  bool _isExcangeTrx;
  bool _isRefundTrx;
  bool _isFullRefund;
  bool _isTaxInfoReq;

  ItinSelector(const ItinSelector&) = delete;
  ItinSelector& operator=(const ItinSelector&) = delete;

public:
  ItinSelector(PricingTrx& trx);
  virtual std::vector<Itin*> get() const;
  virtual std::vector<Itin*> getItin() const;
  virtual bool isExcItin() const;
  virtual bool isExchangeTrx() const;
  virtual bool isRefundTrx() const;
  virtual bool isCat33FullRefund() const;
  virtual bool isNewItin() const;
  bool isTaxInfoReq() const;
};
}
