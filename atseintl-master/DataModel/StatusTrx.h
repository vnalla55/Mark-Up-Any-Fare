//-------------------------------------------------------------------
//
//  File:        StatusTrx.h
//  Created:     June 2, 2004
//  Design:      Mark Kasprowicz
//  Authors:
//
//  Description: Status Transaction object
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/Trx.h"
#include "Service/Service.h"

#include <iosfwd>

namespace tse
{

class StatusTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  bool convert(std::string& response) override;

  std::ostringstream& response() { return _response; }

private:
  std::ostringstream _response;

}; // class StatusTrx
} // tse namespace
