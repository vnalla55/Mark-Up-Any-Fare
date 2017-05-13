//-------------------------------------------------------------------
//
//  File:        SurchargesPreValidator.h
//  Created:     Nov 14, 2011
//  Authors:     Grzegorz Fortuna
//
//  Description: Fare-Led MIP Surcharges
//
//  Updates:
//    Mar 07, 2013 Marek Markiewicz
//
//  Copyright Sabre 2013
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

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PrecalculatedTaxes.h"

#include <boost/optional.hpp>

namespace tse
{

class DiagCollector;
class FareMarket;
class FarePath;
class FarePathWrapper;
class Itin;
class PaxType;
class PaxTypeFare;
class PaxTypeBucket;
class PricingTrx;
class TaxResponse;
class FactoriesConfig;

class SurchargesPreValidator
{
  friend class SurchargesPreValidatorTest;

public:
  SurchargesPreValidator(PricingTrx& trx,
                         const FactoriesConfig& factoriesConfig,
                         const Itin& itin,
                         DiagCollector* diag,
                         size_t numFaresForCAT12Estimation);

  virtual ~SurchargesPreValidator() {}

  void process(FareMarket& fareMarket);

private:
  void processCortege(FareMarket& fareMarket, PaxTypeBucket& cortege);

  bool processPtf(PaxTypeFare& paxTypeFare,
                  PaxType* paxType,
                  PrecalculatedTaxes& taxes,
                  const CarrierCode& valCxr);

  TaxResponse* buildTaxResponse(FarePath& farePath);

  bool getPtfSurcharges(PaxTypeFare& paxTypeFare,
                        const FarePathWrapper& farePathWrapper,
                        PrecalculatedTaxes& taxes);
  bool getPtfYqyr(PaxTypeFare& paxTypeFare, FarePath& farePathWrapper, PrecalculatedTaxes& taxes);
  bool getPtfXFTax(PaxTypeFare& paxTypeFare, FarePath& farePath, PrecalculatedTaxes& taxes);

  virtual boost::optional<MoneyAmount>
  calculatePtfSurcharges(PaxTypeFare& paxTypeFare, const FarePathWrapper& farePathWrapper);

  virtual boost::optional<MoneyAmount>
  calculatePtfYqyr(PaxTypeFare& paxTypeFare, FarePath& farePathWrapper);

  virtual boost::optional<MoneyAmount>
  calculatePtfXFTax(PaxTypeFare& paxTypeFare, FarePath& farePath);

  void processDefaultAmount(PrecalculatedTaxes& taxes, PrecalculatedTaxes::Type type);

  PricingTrx& _trx;
  const FactoriesConfig& _factoriesConfig;
  const Itin& _itin;
  DiagCollector* _diag;
  size_t _numFaresForCAT12Estimation;

  static Logger _logger;
};
}
