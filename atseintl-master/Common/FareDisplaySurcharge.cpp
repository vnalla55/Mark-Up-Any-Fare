//----------------------------------------------------------------------------
//
//  File:        FareDisplaySurcharge.cpp
//  Created:     05/17/2006
//  Authors:     Hitha Alex
//
//  Description:    Common functions required for Surcharge Calculation of ATSE fare display
//
//  Copyright Sabre 2006
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
#include "Common/FareDisplaySurcharge.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/SurchargeData.h"

namespace tse
{
static Logger
logger("atseintl.Common.FareDisplaySurcharge");

namespace FareDisplaySurcharge
{
// ---------------------------------------------
// getTotalOWSurcharge
// Desc: Returns total of all OW surcharges for a specific PaxTypeFare.
// ---------------------------------------------
bool
getTotalOWSurcharge(const FareDisplayTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    MoneyAmount& owSurchargeAmount)
{
  owSurchargeAmount = 0;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
  {
    LOG4CXX_ERROR(logger, "Null FareDisplayInfo pointer");
    return false;
  }

  // Add Surcharges
  std::vector<SurchargeData*>::const_iterator si = fareDisplayInfo->outboundSurchargeData().begin();
  std::vector<SurchargeData*>::const_iterator sEnd = fareDisplayInfo->outboundSurchargeData().end();

  for (; si != sEnd; si++)
  {
    owSurchargeAmount += convertCurrency(&trx, paxTypeFare, *si);
  }
  return true;
}

// ---------------------------------------------
// getTotalRTSurcharge
// Desc: Returns total of all RT surcharges for a specific PaxTypeFare.
// ---------------------------------------------
bool
getTotalRTSurcharge(const FareDisplayTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    MoneyAmount& rtSurchargeAmount)
{
  rtSurchargeAmount = 0;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
  {
    LOG4CXX_ERROR(logger, "FareDisplaySurcharge::Null FareDisplayInfo pointer");
    return false;
  }

  // Add inbound Surcharges
  std::vector<SurchargeData*>::const_iterator si = fareDisplayInfo->inboundSurchargeData().begin();
  std::vector<SurchargeData*>::const_iterator sEnd = fareDisplayInfo->inboundSurchargeData().end();

  for (; si != sEnd; si++)
  {
    rtSurchargeAmount += convertCurrency(&trx, paxTypeFare, *si);
  }
  // Add outbound Surcharges
  si = fareDisplayInfo->outboundSurchargeData().begin();
  sEnd = fareDisplayInfo->outboundSurchargeData().end();

  for (; si != sEnd; si++)
  {
    rtSurchargeAmount += convertCurrency(&trx, paxTypeFare, *si);
  }
  return true;
}

//------------------------------------------------------
// Desc: Convert an amount from published currency to
// DisplayCurrency using NUC or BSR currency conversion
// utilities.
// -----------------------------------------------------
MoneyAmount
convertCurrency(const FareDisplayTrx* trx, const PaxTypeFare& paxTypeFare, SurchargeData* surcharge)
{
  Itin* itin = trx->itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();
  if (displayCurrency == surcharge->currSelected())
  {
    MoneyAmount convertedAmount = surcharge->amountSelected();
    return convertedAmount;
  }

  Money nucMoney(0, NUC);
  Money surchargeCurrMoney(surcharge->amountSelected(), surcharge->currSelected());
  Money displayCurrMoney(displayCurrency);
  Money fareCurrMoney(paxTypeFare.currency());

  // Convert from surcharge currency to NUC
  convertNUC(trx, paxTypeFare, surchargeCurrMoney, nucMoney);

  if (displayCurrency == NUC)
  {
    return nucMoney.value();
  }
  if (surcharge->currSelected() == paxTypeFare.currency())
  {
    fareCurrMoney.value() = surcharge->amountSelected();
  }
  else
  {
    // Convert from NUC to fare currency
    convertNUC(trx, paxTypeFare, nucMoney, fareCurrMoney);
  }

  // Now convert BSR from fare currency to display currency
  CurrencyConversionFacade ccFacade;
  if (!ccFacade.convert(displayCurrMoney,
                        fareCurrMoney,
                        *(const_cast<FareDisplayTrx*>(trx)),
                        true,
                        CurrencyConversionRequest::TAXES))
  {
    LOG4CXX_ERROR(logger, "FareDisplaySurcharge::convertCurrency(): Error in BSR conversion");
    return 0;
  }

  roundAmount(trx, displayCurrMoney.value());
  return displayCurrMoney.value();
}
//------------------------------------------------------------
// Desc: Use nuc converter to convert to and from NUC currency
//------------------------------------------------------------
void
convertNUC(const FareDisplayTrx* trx,
           const PaxTypeFare& paxTypeFare,
           const Money& source,
           Money& destination)
{
  NUCCurrencyConverter nucConverter;
  NUCCollectionResults nucResults;
  nucResults.collect() = true;

  CurrencyConversionRequest curConvReq(destination,
                                       source,
                                       trx->getRequest()->ticketingDT(),
                                       *(trx->getRequest()),
                                       trx->dataHandle(),
                                       paxTypeFare.isInternational(),
                                       CurrencyConversionRequest::TAXES,
                                       false,
                                       const_cast<FareDisplayOptions*>(trx->getOptions()),
                                       true);

  bool convertRC = nucConverter.convert(curConvReq, &nucResults);
  if (!convertRC)
  {
    LOG4CXX_ERROR(logger,
                  "FareDisplaySurcharge::convertNUC(): Error converting currency from NUC");
  }
}

//------------------------------------------------------
// Desc: Round an amount using the currency rules
// -----------------------------------------------------
void
roundAmount(const FareDisplayTrx* trx, MoneyAmount& surchargeAmount)
{
  CurrencyCode displayCurrency = trx->itin().front()->calculationCurrency();
  Money roundedAmount(surchargeAmount, displayCurrency);
  if (roundedAmount.value() == 0)
  {
    LOG4CXX_DEBUG(logger, "FareDisplaySurcharge::roundAmount:Error in rounding");
    return;
  }

  CurrencyRoundingUtil currRoundingUtil;
  const bool useIntlRounding = trx->itin().front()->useInternationalRounding();
  currRoundingUtil.round(roundedAmount, *(const_cast<FareDisplayTrx*>(trx)), useIntlRounding);
  surchargeAmount = roundedAmount.value();
}
}
}
