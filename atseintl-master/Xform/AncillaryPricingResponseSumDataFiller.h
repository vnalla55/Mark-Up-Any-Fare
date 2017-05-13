//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/ServiceFeeUtil.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/OCFeesPrice.h"
#include "Xform/AncillaryPricingResponseFormatter.h"

namespace tse
{

class AncillaryPricingResponseSumDataFiller
{
public:
  AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                        XMLConstruct* construct,
                                        const PaxOCFees* paxOcFees,
                                        OCFeesUsage* ocFees,
                                        AncillaryPricingResponseFormatter* amTaxLogicApplier);

  AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                        XMLConstruct* construct,
                                        const PaxOCFeesUsages* paxOcFeesUsages,
                                        AncillaryPricingResponseFormatter* amTaxLogicApplier);

  void fillSumElement();

private:
  AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                        XMLConstruct* construct,
                                        const OCFeesUsage &ocFees,
                                        const DateTime* ticketingDate,
                                        AncillaryPricingResponseFormatter* amTaxLogicApplier,
                                        MoneyAmount feeAmount,
                                        bool areProvidedDataPacked,
                                        PaxTypeCode paxType);

  void insertDataToSum();
  void insertPassengerData();
  void insertQuantityData();
  void calculateEquivPrice();
  void insertBasePriceData();
  void insertEquivPriceData();
  void insertTaxIndicatorData();
  void applyAmTaxLogic();
  void insertTotalPriceData();

  AncillaryPricingTrx* _ancTrx = nullptr;
  XMLConstruct*        _construct = nullptr;
  OCFeesUsage          _ocFees;
  const DateTime*      _ticketingDate = nullptr;
  AncillaryPricingResponseFormatter*  _amTaxLogicApplier = nullptr;
  bool                _isBaggageRequested = false;
  Money               _equivPrice{""};
  MoneyAmount         _feeAmount = 0.0;
  CurrencyCode        _currency;
  DataHandle          _dataHandle;
  OCFeesPrice*        _ocFeesPrice = nullptr;
  MoneyAmount         _basePrice = 0.0;
  CurrencyNoDec       _basePriceDecimalPlacesCount = 0;
  CurrencyNoDec       _equivalentPriceDecimalPlacesCount = 0;
  bool                _isEquivalentCurrencyAvailable = false;
  bool                _areProvidedDataPacked = false;
  PaxTypeCode         _paxType;
};

}
