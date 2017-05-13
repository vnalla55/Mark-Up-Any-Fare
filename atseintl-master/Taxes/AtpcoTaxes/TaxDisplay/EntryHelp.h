// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Entry.h"

namespace tax
{
namespace display
{

class ResponseFormatter;

class EntryHelp : public Entry
{
public:
  EntryHelp(const TaxDisplayRequest& request,
            ResponseFormatter& formatter) :
              Entry(formatter),
              _request(request) {}

  bool buildBody() override;

private:
  void printHelp();
  void printHelpCalculationBreakdown();

  const TaxDisplayRequest& _request;
};

} /* namespace display */
} /* namespace tax */
