//----------------------------------------------------------------------------
//	File: FareDisplayTrxTest.cpp
//
//	Author: Jeff Hoffman
//  	Created:      10/26/2006
//  	Description:  This is a unit test class for FareDisplayTrxTest.cpp
//
//  Copyright Sabre 2005
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

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>

#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/FareDispTemplate.h"
#include "DBAccess/FareDispTemplateSeg.h"
#include "FareDisplay/Templates/TemplateEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

namespace tse
{

const char* LOADED_TMPL_ID = "999";
const char* DB_TMPL_ID = "11";
const int DB_TEMPLATE = 11;
const int OVERRIDE_TEMPLATE = 12;

// arbitrary seg res so that:
//     DEFAULT  is season template
//     DB       is 2Column template
//     OVERRIDE has both with filler recs
class TestableFDTrx : public FareDisplayTrx
{
  typedef std::vector<FareDispTemplate*> VecFDTemplate;
  typedef std::vector<FareDispTemplateSeg*> VecFDTemplateSeg;
  std::map<int, VecFDTemplate> resultFDTemplate;
  std::map<int, VecFDTemplateSeg> resultFDTemplateSeg;

  // elements in returned vectors
  FareDispTemplate fdTemplate;
  FareDispTemplateSeg fdTemplateSeg_season;
  FareDispTemplateSeg fdTemplateSeg_2col;
  FareDispTemplateSeg fdTemplateSeg_filler;

public:
  TestableFDTrx()
  {
    resultFDTemplate[DEFAULT_TEMPLATE].push_back(&fdTemplate);
    resultFDTemplate[DB_TEMPLATE].push_back(&fdTemplate);
    resultFDTemplate[OVERRIDE_TEMPLATE].push_back(&fdTemplate);

    fdTemplateSeg_2col.columnElement() = RT_FARE_AMOUNT;
    fdTemplateSeg_season.columnElement() = SEASONS;

    resultFDTemplateSeg[DEFAULT_TEMPLATE].push_back(&fdTemplateSeg_season);
    resultFDTemplateSeg[DB_TEMPLATE].push_back(&fdTemplateSeg_2col);
    resultFDTemplateSeg[OVERRIDE_TEMPLATE].push_back(&fdTemplateSeg_filler);
    resultFDTemplateSeg[OVERRIDE_TEMPLATE].push_back(&fdTemplateSeg_season);
    resultFDTemplateSeg[OVERRIDE_TEMPLATE].push_back(&fdTemplateSeg_filler);
    resultFDTemplateSeg[OVERRIDE_TEMPLATE].push_back(&fdTemplateSeg_2col);
    resultFDTemplateSeg[OVERRIDE_TEMPLATE].push_back(&fdTemplateSeg_filler);
  }
  const std::vector<FareDispTemplate*>&
  getFareDispTemplate(const int& templateID, const Indicator& templateType)
  {
    //     echo("-- getFareDispTemplate --", templateID, templateType);
    return resultFDTemplate[templateID];
  }
  const std::vector<FareDispTemplateSeg*>&
  getFareDispTemplateSeg(const int& templateID, const Indicator& templateType)
  {
    //    echo("-- getFareDispTemplateSeg --", templateID, templateType);
    return resultFDTemplateSeg[templateID];
  }
  /*void echo (char* funcName, const int& id, const Indicator& type)
   {
   std::cout<<funcName<<"\ttemplateID:"<<id<< "\ttemplateType:"<<type<<std::endl;
   } */
};

class FareDisplayTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayTrxTest);
  CPPUNIT_TEST(testInit_blank);
  CPPUNIT_TEST(testInit_default);
  CPPUNIT_TEST(testInit_specified);
  CPPUNIT_TEST(testInit_override);
  CPPUNIT_TEST_SUITE_END();

public:
  TestableFDTrx _trx;
  FareDisplayOptions _options;
  FareDisplayRequest _request;
  FareDisplayPref _pref;
  Itin _itin;
  FareMarket _fareMarket1;

  void setUp()
  {
    _trx.setOptions(&_options);
    _trx.setRequest(&_request);
    _trx.itin().push_back(&_itin);
    _itin.fareMarket().push_back(&_fareMarket1);

    _options.fareDisplayPref() = &_pref;
    _pref.singleCxrTemplateId() = LOADED_TMPL_ID;
  }
  void tearDown() {}

  static constexpr bool IS_SEASON = true;
  static constexpr bool IS_2COL = true;
  void assertTmplFlags(bool isSeason, bool is2Col)
  {
    _trx.initializeTemplate();
    CPPUNIT_ASSERT_EQUAL(isSeason, _trx.isSeasonTemplate());
    CPPUNIT_ASSERT_EQUAL(is2Col, _trx.isTwoColumnTemplate());
  }

  // TODO: logic to select template id duplicated in FareDisplayController
  void testInit_blank() { assertTmplFlags(!IS_SEASON, !IS_2COL); }

  void testInit_default()
  {
    _pref.singleCxrTemplateId() = "BOGUS";
    assertTmplFlags(IS_SEASON, !IS_2COL);
  }
  void testInit_specified()
  {
    _pref.singleCxrTemplateId() = DB_TMPL_ID;
    assertTmplFlags(!IS_SEASON, IS_2COL);
  }
  void testInit_override()
  {
    _options.templateOverride() = OVERRIDE_TEMPLATE;
    assertTmplFlags(IS_SEASON, IS_2COL);
  }
};
// end class

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayTrxTest);
} // end namespace
