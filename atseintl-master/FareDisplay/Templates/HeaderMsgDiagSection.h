//-------------------------------------------------------------------
//
//  File:        HeaderMessageDiagSection.h
//  Authors:     Konrad Koch
//  Created:     December 13, 2007
//  Description: Diagnostic for header messages in FQ
//
//
//  Copyright Sabre 2007
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

#include "FareDisplay/Templates/Section.h"

namespace tse
{
class FareDisplayTrx;

class HeaderMsgDiagSection : public Section
{
public:
  HeaderMsgDiagSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  void buildUserLine(const FDHeaderMsg* msg);
  void buildFDTypeLine(const FDHeaderMsg* msg);
  void buildMarketLine(const FDHeaderMsg* msg);
  void buildSaleValidityLine(const FDHeaderMsg* msg);
  void buildGlobalDirLine(const FDHeaderMsg* msg);
  void buildRoutingLine(const FDHeaderMsg* msg);
  void buildStartPositionLine(const FDHeaderMsg* msg);
  // Utility methods
  std::string formatLocation(const Indicator& locType, const LocCode& locCode);
  std::string formatDate(const DateTime& date);
};
} // namespace tse
