//----------------------------------------------------------------------------
//  File:        Diag957Collector.h
//  Created:     2008-04-29
//
//  Description: Diagnostic 957: Pricing ESV Final diversity results
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

#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag956Collector.h"

namespace tse
{
class Diag957Collector : public Diag956Collector
{
public:
  Diag957Collector& displayHeader();
  Diag957Collector& displaySolutions(const std::vector<ESVPQItem*>& pqItemVec);
  Diag957Collector&
  clasifyAndDisplaySolutions(const std::vector<ESVPQItem*>& pqItemVec, CarrierCode carrier);
  Diag957Collector&
  displayClasifiedSolutions(std::string, const std::vector<ESVPQItem*>& pqItemVec);
};
}

