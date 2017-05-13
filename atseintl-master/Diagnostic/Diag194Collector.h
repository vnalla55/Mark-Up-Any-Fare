//----------------------------------------------------------------------------
//  File:        Diag194Collector.h
//  Authors:     Grzegorz Cholewiak
//  Created:     Feb 2007
//
//  Description: Diagnostic 194 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
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

#include <string>

namespace tse
{
class Itin;
class RexPricingTrx;
class ExchangePricingTrx;
class RefundPricingTrx;
class BaseExchangeTrx;

class Diag194Collector : public DiagCollector
{
  friend class Diag194CollectorTest;

public:
  explicit Diag194Collector(Diagnostic& root)
    : DiagCollector(root),
      _processingExItin(false),
      _reshopTrx(false),
      _refundDiagnostic(false),
      _travelSeg(nullptr)
  {
  }

  Diag194Collector()
    : _processingExItin(false), _reshopTrx(false), _refundDiagnostic(false), _travelSeg(nullptr)
  {
  }

  Diag194Collector& operator<<(const Itin& itin) override;
  Diag194Collector& operator<<(const RexPricingTrx& trx);
  Diag194Collector& operator<<(const ExchangePricingTrx& trx);
  Diag194Collector& operator<<(const RefundPricingTrx& trx);

private:
  void prepareSegmentDescriptionRow(const TravelSeg* travelSeg);
  void addCarrierFlightCode();
  void addDate();
  void addCityPair();
  void addRefundCode();
  void addReshopCode();
  void addChangeStatus();
  void printDate(const std::string& dateTitle, const std::string& timeTitle, const DateTime& dt);
  void printTransactionDates(const BaseExchangeTrx& trx);

  bool _processingExItin;
  bool _reshopTrx;
  bool _refundDiagnostic;
  const TravelSeg* _travelSeg;
};

} // namespace tse

