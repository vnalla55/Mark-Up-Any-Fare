// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/XMLConstruct.h"

namespace tse
{
class AbstractTaxBreakdownModel;
class AbstractTaxInformationModel;
class AbstractPreviousTaxInformationModel;

class PreviousTaxInformationFormatter
{
  XMLConstruct& _construct;

  PreviousTaxInformationFormatter(const PreviousTaxInformationFormatter&) = delete;
  PreviousTaxInformationFormatter& operator=(const PreviousTaxInformationFormatter&) = delete;

public:
  PreviousTaxInformationFormatter(XMLConstruct& construct);
  void formatTBD(const AbstractTaxBreakdownModel& model);
  void formatTaxInfoTBD(const AbstractTaxBreakdownModel& model);
  void formatTAX(const AbstractTaxInformationModel& model);
  void formatPTI(const AbstractPreviousTaxInformationModel& model);
};

} // end of tse namespace
