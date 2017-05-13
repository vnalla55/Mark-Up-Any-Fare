//----------------------------------------------------------------------------
//  File:        Diag983Collector.cpp
//  Created:     2010-05-19
//
//  Description: Diagnostic 983 formatter
//
//  Updates:
//
//  Copyright Sabre 2010
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
#include "Diagnostic/Diag983Collector.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleConst.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

#include <iomanip>
#include <vector>

namespace tse
{
Diag983Collector&
Diag983Collector::operator<<(const PricingTrx& pricingTrx)
{
  std::string separator(54, '-');
  separator += "\n";

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::endl;
  dc << "**************************************************************\n"
     << "* 983 : SPLIT ITINERARIES ASSOCIATED WITH ORIGINAL ITINERARY *\n"
     << "**************************************************************\n";

  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL_ITIN")
  {
    dc << std::endl;
    dc << "ORIGINAL ITIN:\n" << separator;

    if (pricingTrx.itin().size())
      displayItinVec(pricingTrx.itin(), separator);
    else
      dc << "EMPTY\n" << separator;

    dc << std::endl;
    dc << "FIRST CXR ITIN VECTOR:\n" << separator;

    if (pricingTrx.subItinVecFirstCxr().size())
      displayItinVec(pricingTrx.subItinVecFirstCxr(), separator);
    else
      dc << "EMPTY\n" << separator;

    dc << std::endl;
    dc << "SECOND CXR ITIN VECTOR:\n" << separator;

    if (pricingTrx.subItinVecSecondCxr().size())
      displayItinVec(pricingTrx.subItinVecSecondCxr(), separator);
    else
      dc << "EMPTY\n" << separator;

    dc << std::endl;
    dc << "OUTBOUND ITIN VECTOR:\n" << separator;

    if (pricingTrx.subItinVecOutbound().size())
      displayItinVec(pricingTrx.subItinVecOutbound(), separator);
    else
      dc << "EMPTY\n" << separator;

    dc << std::endl;
    dc << "INBOUND ITIN VECTOR:\n" << separator;

    if (pricingTrx.subItinVecInbound().size())
      displayItinVec(pricingTrx.subItinVecInbound(), separator);
    else
      dc << "EMPTY\n" << separator;
  } // if(... == "ALL_ITIN")

  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ITIN_MAP")
  {
    std::map<Itin*, PricingTrx::SubItinValue>::const_iterator mapIt =
        pricingTrx.primeSubItinMap().begin();
    std::map<Itin*, PricingTrx::SubItinValue>::const_iterator mapItEnd =
        pricingTrx.primeSubItinMap().end();

    for (; mapIt != mapItEnd; mapIt++)
    {
      dc << std::endl;
      dc << "ORIGINAL:\n" << separator;

      if (mapIt->first)
        dc << printItinMIP(*(mapIt->first)) << separator;
      else
        dc << "EMPTY\n" << separator;

      dc << "SUB-ITIN1 VALUE:\n" << separator;

      if (mapIt->second.firstCxrItin)
        dc << printItinMIP(*(mapIt->second.firstCxrItin)) << separator;
      else
        dc << "EMPTY\n" << separator;

      dc << "SUB-ITIN2 VALUE:\n" << separator;

      if (mapIt->second.secondCxrItin)
        dc << printItinMIP(*(mapIt->second.secondCxrItin)) << separator;
      else
        dc << "EMPTY\n" << separator;

      dc << "OUTBOUND ITIN VALUE:\n" << separator;

      if (mapIt->second.outboundItin)
        dc << printItinMIP(*(mapIt->second.outboundItin)) << separator;
      else
        dc << "EMPTY\n" << separator;

      dc << "INBOUND ITIN VALUE:\n" << separator;

      if (mapIt->second.inboundItin)
        dc << printItinMIP(*(mapIt->second.inboundItin)) << separator;
      else
        dc << "EMPTY\n" << separator;
    }
  } // if(... == "ITIN_MAP")

  bool farePaths = false;
  bool taxes = false;

  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FARE_PATHS")
  {
    farePaths = true;
  }

  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "TAXES")
  {
    taxes = true;
  }

  if (farePaths || taxes)
  {
    std::map<Itin*, PricingTrx::SubItinValue>::const_iterator mapIt =
        pricingTrx.primeSubItinMap().begin();
    std::map<Itin*, PricingTrx::SubItinValue>::const_iterator mapItEnd =
        pricingTrx.primeSubItinMap().end();

    for (; mapIt != mapItEnd; mapIt++)
    {
      dc << std::endl << "ORIGINAL ITINERARY:" << std::endl;

      if (mapIt->first != NULL)
      {
        dc << printItinMIP(*(mapIt->first)) << separator;
      }

      if (mapIt->second.firstCxrItin != NULL)
      {
        if (mapIt->second.firstCxrItin)
        {
          dc << std::endl << "FIRST CARRIER ITINERARY:" << std::endl;
          dc << printItinMIP(*(mapIt->second.firstCxrItin)) << separator;
        }

        if (farePaths)
        {
          dc << std::endl << "FIRST CARRIER FARE PATHS:" << std::endl;
          displayFarePaths(*(mapIt->second.firstCxrItin));
        }

        if (taxes)
        {
          dc << std::endl << "FIRST CARRIER TAXES:" << std::endl;
          displayTaxResponse(*(mapIt->second.firstCxrItin));
        }
      }

      if (mapIt->second.secondCxrItin != NULL)
      {
        if (mapIt->second.secondCxrItin)
        {
          dc << std::endl << "SECOND CARRIER ITINERARY:" << std::endl;
          dc << printItinMIP(*(mapIt->second.secondCxrItin)) << separator;
        }

        if (farePaths)
        {
          dc << std::endl << "SECOND CARRIER FARE PATHS:" << std::endl;
          displayFarePaths(*(mapIt->second.secondCxrItin));
        }

        if (taxes)
        {
          dc << std::endl << "SECOND CARRIER TAXES:" << std::endl;
          displayTaxResponse(*(mapIt->second.secondCxrItin));
        }
      }

      if (mapIt->second.outboundItin != NULL)
      {
        if (mapIt->second.outboundItin)
        {
          dc << std::endl << "OUTBOUND ITINERARY:" << std::endl;
          dc << printItinMIP(*(mapIt->second.outboundItin)) << separator;
        }

        if (farePaths)
        {
          dc << std::endl << "OUTBOUND FARE PATHS:" << std::endl;
          displayFarePaths(*(mapIt->second.outboundItin));
        }

        if (taxes)
        {
          dc << std::endl << "OUTBOUND TAXES:" << std::endl;
          displayTaxResponse(*(mapIt->second.outboundItin));
        }
      }

      if (mapIt->second.inboundItin != NULL)
      {
        if (mapIt->second.inboundItin)
        {
          dc << std::endl << "INBOUND ITINERARY:" << std::endl;
          dc << printItinMIP(*(mapIt->second.inboundItin)) << separator;
        }

        if (farePaths)
        {
          dc << std::endl << "INBOUND FARE PATHS:" << std::endl;
          displayFarePaths(*(mapIt->second.inboundItin));
        }

        if (taxes)
        {
          dc << std::endl << "INBOUND TAXES:" << std::endl;
          displayTaxResponse(*(mapIt->second.inboundItin));
        }
      }
    }
  } // if(farePaths || taxes)

  dc << std::endl << std::endl;

  return (*this);
}

void
Diag983Collector::displayItinVec(const std::vector<Itin*>& itinVec, const std::string& separator)
{
  std::vector<Itin*>::const_iterator itinIter = itinVec.begin();
  std::vector<Itin*>::const_iterator itinIterEnd = itinVec.end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);
    if (itin)
      *this << printItinMIP(*itin) << separator;
  }
}

void
Diag983Collector::displayFarePaths(const Itin& itin)
{
  DiagCollector& dc(*this);

  std::vector<FarePath*>::const_iterator farePathIter = itin.farePath().begin();
  std::vector<FarePath*>::const_iterator farePathIterEnd = itin.farePath().end();

  uint16_t farePathNo = 1;

  for (; farePathIter != farePathIterEnd; ++farePathIter, ++farePathNo)
  {
    dc << std::endl << "FARE PATH " << farePathNo << ":" << std::endl;
    const FarePath* farePath = (*farePathIter);

    dc << (*farePath);
  }
}

void
Diag983Collector::displayTaxResponse(const Itin& itin)
{
  DiagCollector& dc(*this);

  std::vector<TaxResponse*>::const_iterator taxRespIter = itin.getTaxResponses().begin();
  std::vector<TaxResponse*>::const_iterator taxRespIterEnd = itin.getTaxResponses().end();

  for (; taxRespIter != taxRespIterEnd; ++taxRespIter)
  {
    const TaxResponse* taxResp = (*taxRespIter);

    dc << (*taxResp);
  }
}

DiagCollector& Diag983Collector::operator<<(const TaxItem& x)
{
  setPrecision(x.paymentCurrency());
  return this->DiagCollector::operator<<(x);
}

DiagCollector& Diag983Collector::operator<<(const PfcItem& x)
{
  setPrecision(x.pfcCurrencyCode());
  return this->DiagCollector::operator<<(x);
}

DiagCollector& Diag983Collector::operator<< (const TaxRecord& x)
{
  setPrecision(x.taxCurrencyCode());
  return this->DiagCollector::operator<<(x);
}

void
Diag983Collector::setPrecision(const CurrencyCode& currencyCode)
{
  Money moneyPayment(currencyCode);
  this->precision(moneyPayment.noDec());
  this->setf(std::ios::fixed, std::ios::floatfield);
}
}
