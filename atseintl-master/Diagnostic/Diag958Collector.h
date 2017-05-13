//----------------------------------------------------------------------------
//  File:        Diag958Collector.h
//  Created:     2009-03-25
//
//  Description: Diagnostic 958 formatter
//
//  Updates:
//
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class ShoppingTrx;
class ESVPQItem;
class VISOptions;

class Diag958Collector : public DiagCollector
{
public:
  Diag958Collector() : _eisLogic(false), _displayLevel(-1) {}

  virtual Diag958Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
  void initDiag(const ShoppingTrx& shoppingTrx);
  Diag958Collector& printDiagHeader(const ShoppingTrx& shoppingTrx);
  Diag958Collector& printInitialInfo(const Loc& origin,
                                     const Loc& destination,
                                     const short utcoffset,
                                     const DateTime& departureDate,
                                     const DateTime& bookingDate,
                                     const uint16_t mileage,
                                     const uint16_t roundMileage,
                                     const char AP);
  Diag958Collector& printParameterBeta(const int& timeDiff,
                                       const int& mileage,
                                       const char& direction,
                                       const char& APSatInd,
                                       const std::vector<double>* paramBeta);

  Diag958Collector&
  printESVPQItems(const std::vector<ESVPQItem*>& selectedFlights, const std::string descriptionStr);

  Diag958Collector& printVISOptions(VISOptions& visOptions);
  void logInfo(const std::vector<ESVPQItem*>& vec);
  void logInfo(const std::string& s);

  Diag958Collector& printFlightsList(const ShoppingTrx& shoppingTrx);

  Diag958Collector&
  printSelectedFlights(const std::vector<ESVPQItem*>& selectedFlights, const bool& bOutbound);
  Diag958Collector& printBucket(const std::vector<ESVPQItem*>& selectedFlights,
                                const bool& bOutbound,
                                const std::string& bucketName);

private:
  void logInfo(const int i, const ESVPQItem* const pqItem);

  bool _eisLogic;
  int _displayLevel;
};

} // namespace tse

