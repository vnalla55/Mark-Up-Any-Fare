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

#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/Request.h"
#include "Processor/ExcItinTaxesSelector.h"
#include "Processor/IsOrigInCADestInCAOrUS.h"

#include "gmock/gmock.h"
#include <memory>

namespace tax
{
class IsOriginInCanadaAndDestinationInCanadaOrUs : public testing::Test
{
public:
  Request request;
  std::unique_ptr<IsOrigInCADestInCAOrUS> isInCanada;

  void SetUp() override
  {
    Itin itin;
    itin.geoPathRefId() = 0;
    GeoPath geoPath;
    request.geoPaths().push_back(geoPath);
    request.allItins().push_back(itin);

    isInCanada.reset(new IsOrigInCADestInCAOrUS(request, 0));
  }

  void addGeoByNation(const std::string& nation)
  {
    Geo newGeo;
    Loc newLoc;
    codeFromString(nation, newLoc.nation());
    newGeo.loc() = newLoc;
    request.geoPaths().front().geos().push_back(newGeo);
  }
};

TEST_F(IsOriginInCanadaAndDestinationInCanadaOrUs, IsFalseWhenGeoPathIsEmpty)
{
  ASSERT_FALSE(isInCanada->check());
}

TEST_F(IsOriginInCanadaAndDestinationInCanadaOrUs, IsTrueWhenOneGeoAndIsInCanada)
{
  addGeoByNation("CA");
  ASSERT_TRUE(isInCanada->check());
}

TEST_F(IsOriginInCanadaAndDestinationInCanadaOrUs, IsFalseWhenOneGeoAndIsInUS)
{
  addGeoByNation("US");
  ASSERT_FALSE(isInCanada->check());
}

TEST_F(IsOriginInCanadaAndDestinationInCanadaOrUs, IsFalseWhenAnyGeoIsNotInCanadaNorUS)
{
  addGeoByNation("CA");
  addGeoByNation("US");
  addGeoByNation("PL");
  addGeoByNation("US");

  ASSERT_FALSE(isInCanada->check());
}

} // end of tax namespace
