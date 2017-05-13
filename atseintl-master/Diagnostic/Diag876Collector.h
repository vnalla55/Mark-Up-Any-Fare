//----------------------------------------------------------------------------
//  File:        Diag876Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 876 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2009
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
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PricingTrx;
class OptionalServicesConcur;
class SubCodeInfo;

class Diag876Collector : public DiagCollector
{
  friend class Diag876CollectorTest;

public:
  explicit Diag876Collector(Diagnostic& root) : DiagCollector(root), _trx(nullptr) {}
  Diag876Collector() : _trx(nullptr) {}

  void printHeader(const std::set<CarrierCode>& marketingCarriers,
                   const std::set<CarrierCode>& operatingCarriers,
                   const TravelSeg* begin,
                   const TravelSeg* end,
                   bool needValidation,
                   const FarePath& farePath);
  void printNoOCFeesFound(const CarrierCode& cxr, const ServiceGroup& grp);
  void printS6(const OptionalServicesConcur& conscur, const char* msg = nullptr);
  void printS5(const SubCodeInfo& sci, const char* msg = nullptr);
  void printS6Found(bool found);
  void printS5Concur(Indicator i);
  void printS6ValidationHeader(bool marketing);
  void printS6Validation(const SubCodeInfo& sci, const char* status);
  void printS6ValidationNoPass();
  void printNoS6InDB();
  void printS6PassSeqNo(const SubCodeInfo& sci, const OptionalServicesConcur& conscur);

  bool shouldDisplay(const OptionalServicesConcur& conscur) const;
  bool shouldDisplay(const SubCodeInfo& sci) const;
  bool shouldDisplay(const TravelSeg* begin, const TravelSeg* end) const;
  bool shouldDisplay(const ServiceFeesGroup* srvFeesGrp) const;
  bool shouldDisplay(const CarrierCode& cxr) const;
  void initTrx(PricingTrx& trx) { _trx = &trx; }
  void addStarLine(int LineLength = 63);

protected:
  bool isDdInfo();

private:
  PricingTrx* _trx;
};

} // namespace tse

