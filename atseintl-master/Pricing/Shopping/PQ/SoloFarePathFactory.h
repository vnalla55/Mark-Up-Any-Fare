// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         16-11-2011
//! \file         SoloFarePathFactory.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Pricing/FarePathFactory.h"
#include "Pricing/Shopping/PQ/SoloPUFactoryWrapper.h"

namespace tse
{
class PricingUnitFactoryBucket;

namespace shpq
{
class SoloTrxData;

class SoloFarePathFactory : public FarePathFactory
{
public:
  using PUFBucketVector = std::vector<PricingUnitFactoryBucket*>;
  using PUStruct = SoloPUFactoryWrapper::PUStruct;

  static SoloFarePathFactory* createFarePathFactory(SoloTrxData& soloTrxData,
                                                    PUFBucketVector& puFactoryBucketVector,
                                                    PUStruct& puStruct);

  bool initFarePathFactory(DiagCollector& diag);

  SoloFarePathFactory(const FactoriesConfig& factoriesConfig) : FarePathFactory(factoriesConfig) {}

  void setPrevalidateDirectFlights(bool validate) { _prevalidateDirectFlights = validate; }
  /**
   * might give more info why getNextFPPQItem failed, currently returns NULL in most cases
   */
  const char* getValidationMsg() { return _validationMsg; }

private:
  bool isFarePathValid(FPPQItem& fppqItem,
                       DiagCollector& diag,
                       std::list<FPPQItem*>& fppqItems) override;
  bool callbackFVO(FarePath& farePath, DiagCollector& diag);
  bool checkAltDates(FarePath& farePath, DiagCollector& diag);
  bool preValidateFarePath(FarePath& farePath);
  bool checkDirectFlights(FarePath& farePath);
  bool isValidSpanishDiscount(FarePath& farePath);

private:
  bool _prevalidateDirectFlights = false;
  const char* _validationMsg = nullptr;

  friend class SoloFarePathFactoryTest;
};

} /* namespace shpq */
} /* namespace tse */
