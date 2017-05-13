//-------------------------------------------------------------------
//
//  File:        DecodeTrx.h
//  Created:     September 5, 2014
//  Authors:     Roland Kwolek
//
//  Description: Decode Transaction object
//
//
//  Copyright Sabre 2014
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Trx.h"

#include <sstream>
#include <string>

namespace tse
{
class Billing;
class Service;

class DecodeTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  const LocCode& getLocation() const { return _location; }
  void setLocation(const LocCode& zone) { _location = zone; }

  const std::string getResponse() const { return _response.str(); }

  template <typename T>
  inline void addToResponse(const T& text)
  {
    _response << text;
  }

  Billing* getBilling() { return _billing; }
  const Billing* billing() const override { return _billing; }
  void setBilling(Billing* billing) { _billing = billing; }

private:
  LocCode _location; // RTG
  std::ostringstream _response;
  Billing* _billing = nullptr;
};
template <>
inline void
DecodeTrx::addToResponse(const std::ostringstream& text)
{
  _response << text.str();
}
} // tse namespace
