//----------------------------------------------------------------------------
//
//  File:        FareCalcUtil.h
//  Created:     10/5/2004
//  Authors:     Mike Carroll
//
//  Description: Common functions required for ATSE shopping/pricing.
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/FareCalcConfigText.h"


#include <string>

namespace tse
{
class Agent;
class DataHandle;
class FareCalcCollector;
class FareCalcConfig;
class Itin;
class PaxType;
class PricingDetailTrx;
class PricingTrx;
class Trx;

namespace FareCalcUtil
{
  // -------------------------------------------------------------------
  // <PRE>
  //
  // @MethodName  FareCalcConfig::getFareCalcConfig()
  //
  // getFareCalcConfig() gets a FareCalcConfig object based on information
  // in a given transaction by reading configuration information from the
  // database.
  //
  // @param   trx - the transaction with which to retrieve the configuration
  //
  // @return  the address of a FareCalcConfig object containing the desired
  //          configuration information
  //
  // </PRE>
  // -------------------------------------------------------------------------
  const FareCalcConfig* getFareCalcConfig(PricingTrx& trx);

  // -------------------------------------------------------------------
  // <PRE>
  //
  // @MethodName  FareCalcConfig::getFareCalcConfig()
  //
  // getFareCalcConfig() gets a FareCalcConfig object based on information
  // in a given transaction by reading configuration information from the
  // database.
  //
  // @param   trx - the transaction with which to retrieve the configuration
  //
  // @return  the address of a FareCalcConfig object containing the desired
  //          configuration information
  //
  // </PRE>
  // -------------------------------------------------------------------------
  const FareCalcConfig* getFareCalcConfig(PricingDetailTrx& trx);

  // -------------------------------------------------------------------
  // <PRE>
  //
  // @MethodName  FareCalcConfig::getFareCalcConfigForAgent()
  //
  // getFareCalcConfigForAgent() gets a FareCalcConfig object based on
  // given ticketing agent information by reading configuration information
  // from the database.
  //
  // @param   ticketingAgent - the ticketing agent information with which to
  //                           retrieve the configuration
  //          dataHandle -     a handle for accessing the database
  //          isFareCalculationDisplay - used to determine userPCC default
  //
  // @return  the address of a FareCalcConfig object containing the desired
  //          configuration information
  //
  // </PRE>
  // -------------------------------------------------------------------------
  FareCalcConfig* getFareCalcConfigForAgent(const Trx& trx,
                                                   const Agent& ticketingAgent,
                                                   DataHandle& dataHandle,
                                                   const bool isFareCalculationDisplay);

  // -------------------------------------------------------------------
  // <PRE>
  //
  // @MethodName  FareCalcConfig::doubleToStringTruncate()
  //
  // Converts a double precision number to a string representation, truncating
  // to the given number of decimal places, regardless of the existance of
  // trailing zeroes.
  //
  // @param   doubleNum - double precision number to be converted
  //          strNum - reference to string object which will hold the string
  //                   representation of doubleNum
  //          numDec - number of decimal places in the string representation
  //
  // </PRE>
  // -------------------------------------------------------------------------
  void doubleToStringTruncate(double doubleNum, std::string& strNum, unsigned int noDec);

  bool getMsgAppl(FareCalcConfigText::TextAppl, std::string& msg, PricingTrx& pricingTrx);

  bool getMsgAppl(FareCalcConfigText::TextAppl,
                         std::string& msg,
                         PricingTrx& pricingTrx,
                         const FareCalcConfig& fcConfig);

  int getPtcRefNo(const PricingTrx& trx, const PaxType* paxType);

  void
  getUserAppl(const Trx& trx, const Agent& agent, Indicator& userApplType, std::string& applType);

  bool isSameDisplayLoc(const FareCalcConfig& fcConfig,
                               const LocCode& obAirport,
                               const LocCode& obMultiCity,
                               const LocCode& ibAirport,
                               const LocCode& ibMultiCity);

  LocCode
  getDisplayLoc(const FareCalcConfig& fcConfig, const LocCode& airport, const LocCode& multiCity);

  bool getFccDisplayLoc(const FareCalcConfig& fcConfig, const LocCode& loc, LocCode& displayLoc);

  FareCalcCollector* getFareCalcCollectorForItin(PricingTrx& trx, const Itin* itin);

  // Copy logic from LocalCurrencyDisplay::formatExchangeRate()
  // to convert the exchange rate to a string, so result will match conversion with DC' request
  std::string formatExchangeRate(const ExchRate exchangeRate, const CurrencyNoDec rateNoDec);
  bool isOneSolutionPerPaxType(const PricingTrx* trx);
  bool isOnlyOneMatchSolution(const PricingTrx* trx);

}

} // end tse namespace

