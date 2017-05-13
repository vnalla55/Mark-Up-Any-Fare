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

#include "TaxDisplay/Response/ResponseFormatter.h"

namespace tax
{
class Services;

namespace display
{

class Entry;
class TaxDisplayRequest;

class TaxDisplay
{
  friend class TaxDisplayTest;

public:
  TaxDisplay(const TaxDisplayRequest& request, Services& services);
  ~TaxDisplay();

  bool buildResponse(std::ostringstream& response);

private:
  void buildEntry();
  void setEntry();

  Entry* _entry;
  ResponseFormatter _formatter;
  const TaxDisplayRequest& _request;
  Services& _services;
};

} /* namespace display */
} /* namespace tax */
