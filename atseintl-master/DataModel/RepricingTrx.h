//-------------------------------------------------------------------
//
//  File:        RepricingTrx.h
//  Created:     June 2, 2004
//  Design:      Mark Kasprowicz
//  Authors:
//
//  Description: Repricing Transaction object
//
//  Updates:
//
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Trx.h"
#include "Service/Service.h"

#include <iosfwd>

namespace tse
{
class PaxType;

class RepricingTrx : public PricingTrx
{
public:
  struct AddReqPaxType : std::unary_function<PaxType*, void>
  {
    AddReqPaxType(RepricingTrx& trx) : _trx(trx) {}

    void operator()(PaxType* paxType) { _trx.addReqPaxType(paxType); }

    RepricingTrx& _trx;
  };

  struct ComparePaxTypeCode : std::unary_function<FareMarket*, void>
  {
    void operator()(FareMarket* fm) { fm->setComparePaxTypeCode(true); }
  };

  RepricingTrx()
  {
    setTrxType(REPRICING_TRX);
  }

  virtual bool process(Service& srv) override { return srv.process(*this); }

  CarrierCode& carrierOverride() { return _carrierOverride; }
  const CarrierCode& carrierOverride() const { return _carrierOverride; }

  GlobalDirection& globalDirectionOverride() { return _globalDirectionOverride; }
  const GlobalDirection& globalDirectionOverride() const { return _globalDirectionOverride; }

  bool& validate() { return _validate; }
  const bool& validate() const { return _validate; }

  void addReqPaxType(PaxType* reqPaxType);

  bool retrieveFbrFares() const { return _retrieveFbrFares; }
  void setRetrieveFbrFares(bool retrieveFbrFares) { _retrieveFbrFares = retrieveFbrFares; }

  bool& retrieveNegFares() { return _retrieveNegFares; }
  const bool& retrieveNegFares() const { return _retrieveNegFares; }

  void setFMDirectionOverride(FMDirection direction) { _fmDirectionOverride = direction; }
  FMDirection getFMDirectionOverride() const { return _fmDirectionOverride; }

  void setSkipRuleValidation(bool skipRuleValidation) { _skipRuleValidation = skipRuleValidation; _footNotePrevalidationAllowed = !skipRuleValidation;}
  bool skipRuleValidation() const { return _skipRuleValidation; }

  void setOriginTrxType(TrxType trxType) { _originTrxType = trxType; }
  TrxType getOriginTrxType() const { return _originTrxType; }

  void setupFootNotePrevalidation() override;

private:
  CarrierCode _carrierOverride;
  GlobalDirection _globalDirectionOverride = GlobalDirection::XX;
  bool _validate = true;
  bool _retrieveFbrFares = false;
  bool _retrieveNegFares = false;
  bool _skipRuleValidation = false;
  FMDirection _fmDirectionOverride = FMDirection::UNKNOWN;
  TrxType _originTrxType = TrxType::PRICING_TRX;
}; // class RepricingTrx
} // tse namespace
