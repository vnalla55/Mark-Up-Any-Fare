//-------------------------------------------------------------------
//
//  File:        FareSelector.h
//  Created:     April 26, 2005
//  Authors:     LeAnn Perez
//
//  Updates:
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

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "FareDisplay/WebICValidator.h"

#include <set>

namespace tse
{
const double RD_EPSILON = 0.0001;

class FareDisplayTrx;
class FareMarket;
class PaxTypeFare;
class QualifierChain;

/**
*   @class FareSelector
*
*   Description:
*   Retrieves All Fares
*
*/
class FareSelector
{
  friend class FareSelectorTest;

public:
  virtual ~FareSelector() = default;

  virtual bool setup(FareDisplayTrx& trx);
  virtual bool setupChain(FareDisplayTrx& trx);
  virtual bool selectFares(FareDisplayTrx& trx);

  static uint32_t getActiveThreads();

  /**
   * Determines if the PaxTypeFare qualifies for display.
   */
  bool qualifyThisFare(FareDisplayTrx& trx, PaxTypeFare& ptFare);

  void qualifyOutboundCurrency(FareDisplayTrx& trx,
                               FareMarket& fareMarket,
                               CurrencyCode& alternateCurrency);

protected:
  const static Indicator UNAVAILABLE;
  QualifierChain* _qualifierChain = nullptr;
  DataHandle _dh;

  static Logger _logger;

  /**
   * Check if fare is for Display Only
   **/
  void checkInhibitIndicator(FareDisplayTrx& trx, PaxTypeFare& ptFare);

  /**
   * Check Unavailable Tag in Record 1
   **/
  void checkUnavailTag(PaxTypeFare& ptFare);

  /**
   * Check unsupported SITA pax types (SITA unique pax types)
   **/
  void checkUnsupportedSITAPaxTypes(PaxTypeFare& ptFare);

  void getBookingCode(FareDisplayTrx& trx, PaxTypeFare& ptFare);

  bool isEqual(double q1, double q2) { return (fabs(q1 - q2) <= RD_EPSILON); }

  static std::set<PaxTypeCode> _sitaUniquePaxTypes;
  static std::set<PaxTypeCode> initSitaUniquePaxTypesSet();
};

} // namespace tse

