//-------------------------------------------------------------------
//
//  File:        AppConsoleStats.h
//  Created:     May 24, 2005
//  Authors:     Mark Kasprowicz
//
//  Copyright Sabre 2005
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

#include <stdint.h>

namespace tse
{

struct ServicePoint
{
  uint64_t totalOkCount; // Number of transactions successfully processed
  uint64_t totalErrorCount; // Number of transactions that resulted in errors
  double totalElapsedTime; // Total elapsed time for all successfull transactions
  uint64_t totalReqSize; // Total size of all transaction requests
  uint64_t totalRspSize; // Total size of all transaction responses
};

struct AppConsoleStats
{
  ServicePoint trx; // Trx counters
  ServicePoint ASv2; // ASv2 counters
  ServicePoint DSSv2; // DSSv2 counters
  ServicePoint baggage; // Baggage counters
  ServicePoint billing; // Billing counters
  ServicePoint requestResponse; // Request/Response counters
  ServicePoint RTG; // RTG counters
  ServicePoint ABCC; // Branding counters
  ServicePoint S8BRAND; // S8 Branding counters
  ServicePoint TicketingCxr; // Ticketing Carrier counters

  double totalCPUTime; // Total cpu time for all successfull transactions
  uint64_t totalItins; // Total number of itins processed
  time_t startTime; // Start time
  uint16_t concurrentTrx; // Total current concurrent transactions
  uint64_t socketClose; // Total times listening socket(s) was shutdown

  double loadLast1Min;
  double loadLast5Min;
  double loadLast15Min;
  uint64_t freeMemKB;

  double faresCollectionServiceElapsed;
  double faresValidationServiceElapsed;
  double pricingServiceElapsed;
  double shoppingServiceElapsed;
  double itinAnalyzerServiceElapsed;
  double taxServiceElapsed;
  double fareCalcServiceElapsed;
  double currencyServiceElapsed;
  double mileageServiceElapsed;
  double internalServiceElapsed;
  double fareDisplayServiceElapsed;
  double fareSelectorServiceElapsed;
  double rexFareSelectorServiceElapsed;
  double freeBagServiceElapsed;
  double serviceFeesServiceElapsed;
  double ticketingFeesServiceElapsed;
  double s8BrandServiceElapsed;
  double ticketingCxrServiceElapsed;
  double ticketingCxrDispServiceElapsed;

  uint32_t faresCollectionServiceThreads;
  uint32_t faresValidationServiceThreads;
  uint32_t pricingServiceThreads;
  uint32_t shoppingServiceThreads;
  uint32_t itinAnalyzerServiceThreads;
  uint32_t taxServiceThreads;
  uint32_t fareCalcServiceThreads;
  uint32_t currencyServiceThreads;
  uint32_t mileageServiceThreads;
  uint32_t internalServiceThreads;
  uint32_t fareDisplayServiceThreads;
  uint32_t fareSelectorServiceThreads;
  uint32_t rexFareSelectorServiceThreads;
  uint32_t serviceFeesServiceThreads;
  uint32_t ticketingFeesServiceThreads;
  uint32_t s8BrandServiceThreads;
  uint32_t ticketingCxrServiceThreads;
  uint32_t ticketingCxrDispServiceThreads;

  uint64_t dbOkCount;
  uint64_t dbErrorCount;
  double dbElapsedTime;
}; // End struct

} // End namespace

