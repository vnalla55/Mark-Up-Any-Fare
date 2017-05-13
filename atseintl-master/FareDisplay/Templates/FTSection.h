//-------------------------------------------------------------------
//
//  File:        FTSection.h
//  Authors:     Abu
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Tax Display that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Templates/FTTax.h"
#include "FareDisplay/Templates/Section.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{
class FareDisplayTrx;

class FTSection : public Section
{
public:
  FTSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void addToStream(std::ostringstream& oss, int16_t size, const std::string& str);

  void addFareBasisLine(std::string& fareBasis);

  void addNoTaxAppliesLine(PaxTypeFare& ptFare,
                           const std::string& trip,
                           const MoneyAmount fare,
                           const MoneyAmount total,
                           const std::string& str);

  bool populateTaxVec(const PaxTypeFare& paxTypeFare,
                      std::vector<FTTax*>& ftTaxVecOW,
                      std::vector<FTTax*>& ftTaxVecRT);

  bool populateSurchargesInTaxVec(const PaxTypeFare& paxTypeFare,
                                  std::vector<FTTax*>& ftTaxVecOW,
                                  std::vector<FTTax*>& ftTaxVecRT);

  bool populateFTTax(std::vector<FTTax*>& ftTaxVector,
                     const TaxRecord& taxRecord,
                     const Indicator feeInd,
                     MoneyAmount taxAmount);

  MoneyAmount getTotalAmount(const std::vector<FTTax*>& ftTaxVec);

  void addTaxLine(PaxTypeFare& ptFare,
                  const std::string& trip,
                  const MoneyAmount fareAmount,
                  const MoneyAmount totalTaxAmount,
                  const std::vector<FTTax*> ftTaxVec);

  std::string formatMoneyAmount(const MoneyAmount amount);

  void displayMsgForNUC(PaxTypeFare& ptFare, const MoneyAmount fareAmount, const Indicator& owRT);

  void displayMsgForNOTAXApplies(PaxTypeFare& ptFare,
                                 const MoneyAmount fareAmount,
                                 const Indicator& owRT);

  Indicator getFeeInd(const std::vector<TaxItem*>& taxItemVector, const TaxCode& taxCode);

  CurrencyCode _displayCurrency;
  bool isSupplementFeeTrailer = false;
};
} // namespace tse
