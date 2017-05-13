//-------------------------------------------------------------------
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
//-------------------------------------------------------------------

#pragma once

#include "DataModel/Trx.h"
#include "Service/Service.h"

namespace tse
{
class PricingTrx;

class MultiExchangeTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  PricingTrx*& newPricingTrx() { return _newPricingTrx; }
  PricingTrx* newPricingTrx() const { return _newPricingTrx; }

  PricingTrx*& excPricingTrx1() { return _excPricingTrx1; }
  PricingTrx* excPricingTrx1() const { return _excPricingTrx1; }

  PricingTrx*& excPricingTrx2() { return _excPricingTrx2; }
  PricingTrx* excPricingTrx2() const { return _excPricingTrx2; }

  bool& skipNewPricingTrx() { return _skipNewPricingTrx; }
  const bool skipNewPricingTrx() const { return _skipNewPricingTrx; }

  bool& skipExcPricingTrx1() { return _skipExcPricingTrx1; }
  const bool skipExcPricingTrx1() const { return _skipExcPricingTrx1; }

  bool& skipExcPricingTrx2() { return _skipExcPricingTrx2; }
  const bool skipExcPricingTrx2() const { return _skipExcPricingTrx2; }

  PricingTrx*& diagPricingTrx() { return _diagPricingTrx; }
  PricingTrx* diagPricingTrx() const { return _diagPricingTrx; }

  DateTime& cat5TktDT_Ex1() { return _cat5TktDT_Ex1; }
  const DateTime& cat5TktDT_Ex1() const { return _cat5TktDT_Ex1; }

  DateTime& cat5TktDT_Ex2() { return _cat5TktDT_Ex2; }
  const DateTime& cat5TktDT_Ex2() const { return _cat5TktDT_Ex2; }

  const Billing* billing() const override;

private:
  PricingTrx* _newPricingTrx = nullptr;
  PricingTrx* _excPricingTrx1 = nullptr;
  PricingTrx* _excPricingTrx2 = nullptr;

  bool _skipNewPricingTrx = true;
  bool _skipExcPricingTrx1 = true;
  bool _skipExcPricingTrx2 = true;

  PricingTrx* _diagPricingTrx = nullptr;

  DateTime _cat5TktDT_Ex1;
  DateTime _cat5TktDT_Ex2;
};
} // tse namespace
