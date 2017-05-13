#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/FareDisplayController.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/FareDispTemplate.h"
#include "DBAccess/FareDispTemplateSeg.h"
#include "Common/ServiceFeeUtil.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  FareDispTemplateSeg* getSeg(int templateID,
                              Indicator templateType,
                              int16_t columnElement,
                              int16_t elementStart,
                              int16_t elementLenght,
                              Indicator elementJustify,
                              Indicator elementDateFormat,
                              std::string header,
                              int16_t headerStart)
  {
    FareDispTemplateSeg* ret = _memHandle.create<FareDispTemplateSeg>();
    ret->templateID() = templateID;
    ret->templateType() = templateType;
    ret->columnElement() = columnElement;
    ret->elementStart() = elementStart;
    ret->elementLength() = elementLenght;
    ret->elementJustify() = elementJustify;
    ret->elementDateFormat() = elementDateFormat;
    ret->header() = header;
    ret->headerStart() = headerStart;
    return ret;
  }

public:
  const std::vector<FareDispTemplate*>&
  getFareDispTemplate(const int& templateID, const Indicator& templateType)
  {
    if (templateID == 0)
      return *_memHandle.create<std::vector<FareDispTemplate*> >();
    else if (templateID == 1)
    {
      std::vector<FareDispTemplate*>* ret = _memHandle.create<std::vector<FareDispTemplate*> >();
      FareDispTemplate* fdt = _memHandle.create<FareDispTemplate>();
      fdt->templateID() = templateID;
      fdt->templateType() = templateType;
      fdt->lineStyle() = 1;
      ret->push_back(fdt);
      return *ret;
    }
    return DataHandleMock::getFareDispTemplate(templateID, templateType);
  }
  const std::vector<FareDispTemplateSeg*>&
  getFareDispTemplateSeg(const int& templateID, const Indicator& templateType)
  {
    if (templateID == 1)
    {
      std::vector<FareDispTemplateSeg*>* ret =
          _memHandle.create<std::vector<FareDispTemplateSeg*> >();
      ret->push_back(getSeg(1, 'M', 1, 1, 3, 'R', ' ', "", 0));
      ret->push_back(getSeg(1, 'M', 2, 4, 1, ' ', ' ', "", 0));
      ret->push_back(getSeg(1, 'M', 3, 5, 1, ' ', ' ', "V", 5));
      ret->push_back(getSeg(1, 'M', 4, 6, 1, ' ', ' ', "", 0));
      ret->push_back(getSeg(1, 'M', 5, 7, 15, ' ', ' ', "FARE BASIS", 7));
      ret->push_back(getSeg(1, 'M', 6, 22, 2, ' ', ' ', "BK", 22));
      ret->push_back(getSeg(1, 'M', 7, 24, 1, ' ', ' ', "", 0));
      ret->push_back(getSeg(1, 'M', 8, 25, 9, 'R', ' ', "FARE", 28));
      ret->push_back(getSeg(1, 'M', 10, 33, 1, ' ', ' ', "", 0));
      ret->push_back(getSeg(1, 'M', 11, 35, 1, ' ', ' ', "CX", 35));
      ret->push_back(getSeg(1, 'M', 23, 39, 12, 'X', '1', "TRAVEL-TICKET", 39));
      ret->push_back(getSeg(1, 'M', 14, 52, 5, 'X', ' ', "AP", 53));
      ret->push_back(getSeg(1, 'M', 15, 58, 6, 'X', ' ', "MINMAX", 58));
      return *ret;
    }
    return DataHandleMock::getFareDispTemplateSeg(templateID, templateType);
  }
};
}
class FareDisplayControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayControllerTest);
  CPPUNIT_TEST(testIsRuleDisplay);
  CPPUNIT_TEST(testDisplayFareInfoNoPrefs);
  CPPUNIT_TEST(testDisplayFareInfo);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void testIsRuleDisplay()
  {
    FareDisplayTrx fdTrx;
    FareDisplayRequest request;
    FareDisplayOptions options;
    FareInfo fareInfo1, fareInfo2, fareInfo3;
    Fare fare1, fare2, fare3;
    PaxTypeFare ptFare1, ptFare2, ptFare3;
    FareDisplayController fdc;

    fareInfo1.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo2.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED + 1;
    fareInfo3.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;

    fare1.setFareInfo(&fareInfo1);
    fare2.setFareInfo(&fareInfo2);
    fare3.setFareInfo(&fareInfo3);

    ptFare1.setFare(&fare1);
    ptFare2.setFare(&fare2);
    ptFare3.setFare(&fare3);

    fdTrx.allPaxTypeFare().push_back(&ptFare1);
    fdTrx.allPaxTypeFare().push_back(&ptFare2);
    fdTrx.allPaxTypeFare().push_back(&ptFare3);

    request.requestType() = FARE_RULES_REQUEST;
    fdTrx.setRequest(&request);
    options.lineNumber() = 1;
    fdTrx.setOptions(&options);

    bool ret = fdc.isRuleDisplay(fdTrx);
    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(fdTrx.allPaxTypeFare().size() == 1);
    CPPUNIT_ASSERT(fdTrx.allPaxTypeFare()[0] == &ptFare1);

    fdTrx.allPaxTypeFare().push_back(&ptFare1);
    fdTrx.allPaxTypeFare().push_back(&ptFare2);
    fdTrx.allPaxTypeFare().push_back(&ptFare3);

    options.lineNumber() = 0;
    ret = fdc.isRuleDisplay(fdTrx);
    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(fdTrx.allPaxTypeFare().size() == 1);
    CPPUNIT_ASSERT(fdTrx.allPaxTypeFare()[0] == &ptFare2);

    fdTrx.allPaxTypeFare().push_back(&ptFare1);
    fdTrx.allPaxTypeFare().push_back(&ptFare2);
    fdTrx.allPaxTypeFare().push_back(&ptFare3);

    CPPUNIT_ASSERT(fdc.validFares(fdTrx) == 4);

    ret = fdc.isRuleDisplay(fdTrx);
    CPPUNIT_ASSERT(ret == false);
  }

  void testDisplayFareInfoNoPrefs()
  {
    FareDisplayTrx fdTrx;
    FareDisplayOptions options;
    FareDisplayRequest request;
    FareDisplayController fdc;
    Agent agent;
    AirSeg seg;
    Itin itin;
    Billing billing;

    options.allCarriers() = 'Y';
    fdTrx.setOptions(&options);

    request.ticketingAgent() = &agent;
    fdTrx.setRequest(&request);

    fdTrx.travelSeg().push_back(&seg);
    fdTrx.billing() = &billing;
    fdTrx.itin().push_back(&itin);

    CPPUNIT_ASSERT(!fdc.displayFareInfo(fdTrx));
    CPPUNIT_ASSERT(fdTrx.response().str().size() > 0);
  }

  void testDisplayFareInfo()
  {
    MyDataHandle mdh;
    FareDisplayTrx fdTrx;
    FareDisplayOptions options;
    FareDisplayRequest request;
    FareDisplayController fdc;
    FareDisplayPref fdp;
    Agent agent;
    AirSeg seg;
    Itin itin;
    Billing billing;

    options.allCarriers() = 'Y';
    options.fareDisplayPref() = &fdp;
    fdTrx.setOptions(&options);

    request.ticketingAgent() = &agent;
    fdTrx.setRequest(&request);

    fdTrx.travelSeg().push_back(&seg);
    fdTrx.billing() = &billing;
    fdTrx.itin().push_back(&itin);

    CPPUNIT_ASSERT(fdc.displayFareInfo(fdTrx));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayControllerTest);
}
