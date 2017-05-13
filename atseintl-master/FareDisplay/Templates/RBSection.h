//-------------------------------------------------------------------
//
//  File:        RBSection.h
//  Copyright Sabre 2005
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

#include "DataModel/Itin.h"
#include "FareDisplay/Templates/Section.h"

#include <string>

namespace tse
{
class FareDisplayTrx;

class RBSection : public Section
{
public:
  RBSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  bool isDomestic()
  {
    const GeoTravelType type = _trx.itin().front()->geoTravelType();
    return (type == GeoTravelType::Domestic || type == GeoTravelType::Transborder);
  }
  static const std::string HARD_CODED_DOMESTIC_RB_RESTRICTION;
};
}
