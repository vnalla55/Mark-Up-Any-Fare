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

#include "Common/TseCodeTypes.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/RequestResponse/InputYqYrPath.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/FacadesUtils.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
class FacadesUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FacadesUtilsTest);

  CPPUNIT_TEST(testIsYqYr_is);
  CPPUNIT_TEST(testIsYqYr_isNot);

  CPPUNIT_TEST(testFindYqyr_found);
  CPPUNIT_TEST(testFindYqyr_notFound);

  CPPUNIT_TEST(testFindGeoPathMapping_found);
  CPPUNIT_TEST(testFindGeoPathMapping_notFound);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testIsYqYr_is()
  {
    CPPUNIT_ASSERT(FacadesUtils::isYqYr(TaxCode("YQI")));
    CPPUNIT_ASSERT(FacadesUtils::isYqYr(TaxCode("YQF")));
    CPPUNIT_ASSERT(FacadesUtils::isYqYr(TaxCode("YRI")));
    CPPUNIT_ASSERT(FacadesUtils::isYqYr(TaxCode("YRF")));
  }

  void testIsYqYr_isNot()
  {
    CPPUNIT_ASSERT(!FacadesUtils::isYqYr(TaxCode("")));
    CPPUNIT_ASSERT(!FacadesUtils::isYqYr(TaxCode("YQ")));
    CPPUNIT_ASSERT(!FacadesUtils::isYqYr(TaxCode("YQY")));
    CPPUNIT_ASSERT(!FacadesUtils::isYqYr(TaxCode("YQR")));
    CPPUNIT_ASSERT(!FacadesUtils::isYqYr(TaxCode("ABC")));
  }

  void testFindYqyr_notFound()
  {
    tax::InputRequest::InputYqYrs yqYrs;
    createInputYqYrs(yqYrs);

    tax::InputYqYr newYqYr;
    newYqYr._amount = tax::doubleToAmount(4);
    newYqYr._code = "5_";
    newYqYr._type = '7';

    tax::type::Index foundId;
    CPPUNIT_ASSERT_EQUAL(false, FacadesUtils::findYqyr(newYqYr, yqYrs, foundId));
  }

  void testFindYqyr_found()
  {
    tax::InputRequest::InputYqYrs yqYrs;
    createInputYqYrs(yqYrs);

    tax::InputYqYr newYqYr;
    newYqYr._amount = tax::doubleToAmount(4);
    newYqYr._code = "5_";
    newYqYr._type = '6';

    tax::type::Index foundId;
    CPPUNIT_ASSERT_EQUAL(true, FacadesUtils::findYqyr(newYqYr, yqYrs, foundId));
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), foundId);
  }

  void testFindGeoPathMapping_found()
  {
    tax::InputRequest::InputGeoPathMappings geoPathMappings;
    createInputGeoPathMappings(geoPathMappings);

    tax::InputGeoPathMapping newGeoPathMapping;
    tax::InputMapping* mapping = new tax::InputMapping;
    newGeoPathMapping._mappings.push_back(mapping);
    addMap(7, *mapping);

    tax::type::Index foundId;
    CPPUNIT_ASSERT(FacadesUtils::findGeoPathMapping(newGeoPathMapping, geoPathMappings, foundId));
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), foundId);
  }

  void testFindGeoPathMapping_notFound()
  {
    tax::InputRequest::InputGeoPathMappings geoPathMappings;
    createInputGeoPathMappings(geoPathMappings);

    tax::InputGeoPathMapping newGeoPathMapping;
    tax::InputMapping* mapping = new tax::InputMapping;
    newGeoPathMapping._mappings.push_back(mapping);
    addMap(8, *mapping);

    tax::type::Index foundId;
    CPPUNIT_ASSERT(!FacadesUtils::findGeoPathMapping(newGeoPathMapping, geoPathMappings, foundId));
  }

private:
  void createInputYqYrs(tax::InputRequest::InputYqYrs& yqYrs)
  {
    tax::InputYqYr* yqYr;

    yqYr = new tax::InputYqYr;
    yqYrs.push_back(yqYr);
    yqYr->_id = 0;
    yqYr->_amount = tax::doubleToAmount(0);
    yqYr->_code = "0_";
    yqYr->_type = '0';

    yqYr = new tax::InputYqYr;
    yqYrs.push_back(yqYr);
    yqYr->_id = 1;
    yqYr->_amount = tax::doubleToAmount(4);
    yqYr->_code = "5_";
    yqYr->_type = '6';
  }

  void createInputGeoPathMappings(tax::InputRequest::InputGeoPathMappings& geoPathMappings)
  {
    tax::InputMapping* mapping;
    tax::InputGeoPathMapping* geoPathMapping;

    //
    geoPathMapping = new tax::InputGeoPathMapping;
    geoPathMappings.push_back(geoPathMapping);
    geoPathMapping->_id = 0;

    mapping = new tax::InputMapping;
    geoPathMapping->_mappings.push_back(mapping);
    addMap(4, *mapping);
    addMap(3, *mapping);

    mapping = new tax::InputMapping;
    geoPathMapping->_mappings.push_back(mapping);
    addMap(0, *mapping);
    addMap(1, *mapping);
    addMap(2, *mapping);

    //
    geoPathMapping = new tax::InputGeoPathMapping;
    geoPathMappings.push_back(geoPathMapping);
    geoPathMapping->_id = 1;

    mapping = new tax::InputMapping;
    geoPathMapping->_mappings.push_back(mapping);
    addMap(7, *mapping);
  }

  void addMap(const tax::type::Index geoRefId, tax::InputMapping& mapping)
  {
    tax::InputMap* map = new tax::InputMap;
    map->_geoRefId = geoRefId;
    mapping.maps().push_back(map);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FacadesUtilsTest);
} // namespace tax
