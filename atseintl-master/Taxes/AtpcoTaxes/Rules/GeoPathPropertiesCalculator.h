// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
struct GeoPathProperties;
class Itin;
class Request;
class MileageService;

class GeoPathPropertiesCalculator
{
public:
  GeoPathPropertiesCalculator(const Request&, const MileageService&);

  void calculate(const Itin&, GeoPathProperties&) const;

private:
  const Request& _request;
  const MileageService& _mileageService;
};
}

