//----------------------------------------------------------------------------
//
//  File:  NegotiatedFareCombinationValidator.h
//  Created:  July 10, 2009
//  Authors:  Nakamon Thamsiriboon/Konrad Koch
//
//  Description: Validate Negotiated Fares combination
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


namespace tse
{

class PricingTrx;
class PricingUnit;
class FarePath;
class FareUsage;
class PaxTypeFare;
class NegFareRest;
class Diag535Collector;

class NegotiatedFareCombinationValidator
{
  friend class NegotiatedFareCombinationValidatorTest;

private:
  enum WarningCode
  {
    NO_WARNING = 0,
    MIXED_FARES,
    MULTIPLE_BSP,
    MIXED_COMMISSION,
    MULTIPLE_TOUR_CODES,
    CONFLICTING_TOUR_CODES,
    CONFLICTING_TFD_BYTE101,
    TFD_RETRIEVE_FAIL,
    TFDPSC_MATCH_FAIL,
    MULTIPLE_VALUE_CODES,
    MULTIPLE_PRINT_OPTIONS,
    MIXED_FARE_BOX_AMT,
    METHOD_TYPE_1_REQ
  };

public:
  NegotiatedFareCombinationValidator(PricingTrx& trx);
  virtual ~NegotiatedFareCombinationValidator();

  bool validate(const PricingUnit& pricingUnit);
  bool validate(const FarePath& farePath);

  const char* getWarningMessage() const;

private:
  bool validate(const std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateNegFareCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateNetRemitCombination(bool isPTF1NetRemitFare, bool isPTF2NetRemitFare) const;
  bool validateMethodTypeCombination(const Indicator& method1, const Indicator& method2) const;
  bool validateCommissionCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateTFDCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateAllTFDData(const FarePath& farePath, std::vector<const PaxTypeFare*>& paxTypeFares);

  const NegFareRest* getCat35Record3(const PaxTypeFare& paxTypeFare) const;
  const NegFareRest* getCat35TktData(const PaxTypeFare& paxTypeFare) const;
  Indicator getBspMethodType(const NegFareRest* negFareRest) const;
  const char* getWarningMessage(const WarningCode warningCode) const;
  void getAllPaxTypeFares(const PricingUnit& pu, std::vector<const PaxTypeFare*>& paxTypeFares);
  void getAllPaxTypeFares(const FarePath& fp, std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateTourCodeCombination(std::vector<const PaxTypeFare*>& paxTypeFares);
  void doDiagnostic(std::vector<const PaxTypeFare*>& paxTypeFares,
                    bool result,
                    const FarePath* fp = nullptr);
  bool isNetRemitFareCombination(const PaxTypeFare* paxTypeFare) const;
  bool isNegotiatedFareCombination(const std::vector<const PaxTypeFare*>& paxTypeFares) const;
  bool validateValueCode(const FarePath& farePath);
  bool validatePrintOptionCombination(std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateFareBoxCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);
  bool validateNetRemitMethod1(std::vector<const PaxTypeFare*>& paxTypeFares);

  virtual bool processNetRemitFareSelection(PricingTrx& trx,
                                            const FarePath& farePath,
                                            PricingUnit& pricingUnit,
                                            FareUsage& fareUsage,
                                            const NegFareRest& negFareRest) const;

  PricingTrx& _trx;
  Diag535Collector* _dc;
  WarningCode _warningCode;
  bool _farePathScope;
  bool _abacusUser;
  bool _infiniUser;
  bool _shouldDisplayTfdpsc;

};
}
