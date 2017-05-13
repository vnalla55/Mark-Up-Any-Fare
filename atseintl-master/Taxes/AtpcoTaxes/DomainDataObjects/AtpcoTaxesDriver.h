// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

namespace tax
{

class AtpcoTaxesDriver
{
public:
  AtpcoTaxesDriver() {};
  virtual ~AtpcoTaxesDriver() {};

  virtual bool buildRequest() = 0;
  virtual void setServices() = 0;
  virtual bool processTaxes() = 0;
  virtual bool convertResponse() = 0;
};
}
