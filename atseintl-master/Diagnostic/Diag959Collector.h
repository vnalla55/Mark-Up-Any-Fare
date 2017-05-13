//----------------------------------------------------------------------------
//  File:        Diag959Collector.h
//  Created:     2008-07-05
//
//  Description: Diagnostic 959 formatter
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

#include "Diagnostic/Diag953Collector.h"

namespace tse
{
class Diag959Collector : public Diag953Collector
{
public:
  Diag959Collector() : _printSVOutput(false) {}

  virtual Diag959Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

private:
  virtual Diag959Collector& operator<<(const ESVSolution& esvSolution);
  virtual Diag959Collector& operator<<(const ESVOption& esvOption);
  virtual Diag959Collector& operator<<(const PaxTypeFare& paxTypeFare) override;
  void printBkgCodes(const PaxTypeFare* paxTypeFare);
  void printSabreViewCommnads(const ESVSolution& esvSolution);
  void printSabreViewBooking(const ESVOption& esvOption);
  void printSabreViewFares(const ESVOption& esvOption, std::string& wpString, int& segIndex);

  void printGroupingSummaryInfo(const ShoppingTrx& shoppingTrx);

  bool _printSVOutput;
};

} // namespace tse

