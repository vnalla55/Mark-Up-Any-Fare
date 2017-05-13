//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"

#include <vector>

namespace tse
{
class FarePath;
class FareUsage;
class Itin;
class PaxTypeFare;
class PricingTrx;
class SvcFeesDiagCollector;
class SvcFeesTktDesignatorInfo;
class TravelSeg;

class SvcFeesTktDesigValidator
{
  friend class SvcFeesTktDesigValidatorTest;

public:
  SvcFeesTktDesigValidator(PricingTrx& trx, SvcFeesDiagCollector* diag);
  virtual ~SvcFeesTktDesigValidator() = default;

  bool validate(int itemNo) const;

protected:
  virtual bool getTicketDesignators(std::set<TktDesignator>& tktDesignators) const = 0;
  virtual bool validate(const std::set<TktDesignator>& tktDesignators,
                        const std::vector<SvcFeesTktDesignatorInfo*>& svcFeeesTktDesignators) const;

  bool
  compare(const std::set<TktDesignator>& inputTktDesignator, const TktDesignator& tktDesig) const;

  bool wildCardMatch(const TktDesignator& sTable, const TktDesignator& sInput) const;
  bool wildCardMatchLeft(const TktDesignator& sTable, const TktDesignator& sInput) const;
  bool wildCardMatchMiddle(const TktDesignator& sTable,
                           const TktDesignator& sInput,
                           size_t strBegin) const;
  bool wildCardMatchRight(const TktDesignator& sTable,
                          const TktDesignator& sInput,
                          size_t strBegin) const;
  // deprecated (tktDesignatorCoreFix):
  bool wildCardPassOnDigit(const TktDesignator& sInput,
                           const TktDesignator& pattern,
                           size_t startPoint,
                           bool right = false) const;
  // ^^^

  // Database overrides
  virtual const std::vector<SvcFeesTktDesignatorInfo*>&
  getSvcFeesTicketDesignator(const int itemNo) const;

protected:
  PricingTrx& _trx;
  SvcFeesDiagCollector* _diag;
  static constexpr char ASTERISK = '*';
};

class SvcFeesInputTktDesigValidator : public SvcFeesTktDesigValidator
{
  friend class SvcFeesInputTktDesigValidatorTest;

public:
  SvcFeesInputTktDesigValidator(PricingTrx& trx, const Itin& itin, SvcFeesDiagCollector* diag);

protected:
  void getInputTktDesignators(std::set<TktDesignator>& inputTktDesignators) const;
  bool getTicketDesignators(std::set<TktDesignator>& tktDesignators) const override;

  const Itin& _itin;
};

class SvcFeesOutputTktDesigValidator : public SvcFeesTktDesigValidator
{
  friend class SvcFeesOutputTktDesigValidatorTest;

public:
  SvcFeesOutputTktDesigValidator(PricingTrx& trx,
                                 const PaxTypeFare& ptf,
                                 SvcFeesDiagCollector* diag);
  SvcFeesOutputTktDesigValidator(PricingTrx& trx,
                                 const FarePath& farePath,
                                 const std::vector<TravelSeg*>::const_iterator segI,
                                 const std::vector<TravelSeg*>::const_iterator segIE,
                                 SvcFeesDiagCollector* diag);

protected:
  bool getTicketDesignators(std::set<TktDesignator>& tktDesignators) const override;
  bool getTktDesigFromPtf(const PaxTypeFare& ptf, std::set<TktDesignator>& tktDesignators) const;
  bool
  validate(const std::set<TktDesignator>& tktDesignators,
           const std::vector<SvcFeesTktDesignatorInfo*>& svcFeeesTktDesignators) const override;
  bool isFareProcessed(const PaxTypeFare& ptf) const;

  virtual std::string getFareBasis(const PaxTypeFare& paxTypeFare) const;

private:
  const PaxTypeFare* _ptf = nullptr;
  const FarePath* _farePath = nullptr;
  const std::vector<TravelSeg*>::const_iterator _segI;
  const std::vector<TravelSeg*>::const_iterator _segIE;
};
}
