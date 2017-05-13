//-------------------------------------------------------------------
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

#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/CommonSolutionValidator.h"
#include "RexPricing/GenericRexMapper.h"

#include <boost/function.hpp>

#include <tuple>

namespace tse
{

class Diag689Collector;

class RexFareBytesValidator : public CommonSolutionValidator
{
  friend class RexFareBytesValidatorTest;

  using FareAmountComparisonResults = std::tuple<MoneyAmount, MoneyAmount, CurrencyCode>;

public:
  explicit RexFareBytesValidator(RexPricingTrx& trx, const FarePath* excFarePath);

  RexFareBytesValidator(RexPricingTrx& trx,
                        const std::vector<const PaxTypeFare*>& allRepricePTFs,
                        const uint16_t& itinIndex,
                        const GenericRexMapper* grm,
                        const FarePath* excFarePath);
  void setDiag(Diag689Collector* dc) { _dc = dc; }
  virtual ~RexFareBytesValidator() {};

  bool validate(const ProcessTagPermutation& permutation);

  bool validate(FareBytesData& data, PaxTypeFare& ptf);

  static VendorCode getVendorCodeForFareTypeTbl(const ProcessTagInfo& pti,
                                                bool changeVendorForDFFFares);

  typedef boost::function<bool(RexFareBytesValidator*, const ProcessTagInfo&, const PaxTypeFare&)>
  FareByteChecker;
  typedef std::pair<FareByteChecker, RepriceFareValidationResult> CheckerResultPair;
  typedef std::vector<CheckerResultPair> CheckersVector;

private:
  RepriceFareValidationResult allPtisFailed(FareBytesData& data, const PaxTypeFare& newPtf);

  bool validate(const ProcessTagInfo& pti);
  bool validate(const ProcessTagInfo& pti, const PaxTypeFare& ptf);

  bool anyFarePassed(const ProcessTagInfo& pti, std::vector<const PaxTypeFare*> ptfs);
  bool allPtisFailed(const std::set<ProcessTagInfo*>& ptis,
                     const PaxTypeFare& newPtf,
                     FareByteChecker checkFareByte);

  bool checkFareRules(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareCxrApplTbl(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareTrfNumber(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareClassCode(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareTypeCode(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareTypeTable(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareAmount(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  FareAmountComparisonResults
  getFaresAmountForComparison(const ProcessTagInfo& pti, const PaxTypeFare& ptf, double epsilon);
  bool checkSameInd(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkFareNormalSpecial(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkExcludePrivateIndicator(const ProcessTagInfo& pti, const PaxTypeFare& ptf);
  bool checkOWRT(const ProcessTagInfo& pti, const PaxTypeFare& ptf);

  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo);

  void printDiag(RepriceFareValidationResult result,
                 PaxTypeFare& ptf,
                 std::set<ProcessTagInfo*>& ptiList);
  bool hasDiagAndFilterPassed() const;

  bool checkIfOverrideNeeded(const FarePath* farePath) const;

  virtual void updateCaches(const ProcessTagInfo& pti, RepriceFareValidationResult result);
  virtual bool checkCaches(const ProcessTagInfo& pti, RepriceFareValidationResult& cachedResult);

  RexPricingTrx& _trx;

  const std::vector<const PaxTypeFare*>* _allRepricePTFs;
  std::map<const ProcessTagInfo*, RepriceFareValidationResult> _faresCheckCache;

  static const CheckersVector _fareByteCheckers;

  Diag689Collector* _dc;
  const uint16_t& _itinIndex;
  const GenericRexMapper* _mapper;
  bool _changeVendorForDFFFares;
};
}

