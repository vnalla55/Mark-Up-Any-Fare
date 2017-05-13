//----------------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include <boost/assign/std/vector.hpp>

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/AncillaryPricingResponseFormatterPostTkt.h"

namespace tse
{

using namespace boost::assign;

class AncillaryPricingResponseFormatterPostTktTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingResponseFormatterPostTktTest);
  CPPUNIT_SKIP_TEST(testGetOCFeesForR7);
  CPPUNIT_SKIP_TEST(testFormatAEFeesResponse);
  CPPUNIT_TEST(testCheckRequestedGroup_NotRequested);
  CPPUNIT_TEST(testCheckRequestedGroup_Requested_and_Match);
  CPPUNIT_TEST(testCheckRequestedGroup_Requested_not_Match);

  CPPUNIT_TEST_SUITE_END();

public:
  PaxType*
  createPaxType(const char* paxType, const char* tktRef, const char* tktNum, const char* tktName)
  {
    PaxType* ret = _memHandle.create<PaxType>();
    ret->paxType() = paxType;
    PaxType::TktInfo* ti = _memHandle.create<PaxType::TktInfo>();
    ti->tktRefNumber() = tktRef;
    ti->tktNumber() = tktNum;
    ti->psgNameNumber() = tktName;
    ret->psgTktInfo().push_back(ti);
    return ret;
  }
  TravelSeg*
  createTS(const char* from, const char* to, int segmentOrder, int pnrSegmentOrder, int coupon)
  {
    AirSeg* ret = _memHandle.create<AirSeg>();
    Loc* l = _memHandle.create<Loc>();
    l->loc() = from;
    ret->origAirport() = from;
    ret->origin() = l;
    l = _memHandle.create<Loc>();
    l->loc() = to;
    ret->destAirport() = to;
    ret->destination() = l;
    ret->segmentOrder() = segmentOrder;
    ret->pnrSegment() = pnrSegmentOrder;
    ret->ticketCouponNumber() = coupon;
    ret->setCheckedPortionOfTravelInd('T');
    return ret;
  }
  OCFees* createOCFess(ServiceFeesGroup* sg, FarePath* fp, TravelSeg* ts1, TravelSeg* ts2)
  {
    OCFees* ret = _memHandle.create<OCFees>();
    ret->subCodeInfo() = createSubCodeInfo(sg->groupCode());
    ret->carrierCode() = "BA";
    ret->travelStart() = ts1;
    ret->travelEnd() = ts2;
    ret->feeAmount() = 10;
    ret->feeCurrency() = "BGP";
    ret->displayAmount() = 20;
    ret->displayCurrency() = "USD";
    ret->farePath() = fp;
    sg->ocFeesMap()[fp].push_back(ret);
    OCFees::OCFeesSeg* seg = _memHandle.create<OCFees::OCFeesSeg>();
    ret->segments().push_back(seg);
    ret->setSeg(0);
    ret->optFee() = _memHandle.create<OptionalServicesInfo>();
    return ret;
  }
  SubCodeInfo* createSubCodeInfo(ServiceGroup& sg)
  {
    SubCodeInfo* ret = _memHandle.create<SubCodeInfo>();
    ret->vendor() = "VEND";
    ret->carrier() = "BA";
    ret->serviceTypeCode() = "OC";
    ret->serviceSubTypeCode() = "SUB";
    ret->serviceGroup() = sg;
    return ret;
  }
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("ANCILLARY_EMDS_PHASE_2_ENABLED", 'Y', "SERVICE_FEES_SVC", true);
    TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 10, "SERVICE_FEES_SVC");

    _formater = _memHandle.create<AncillaryPricingResponseFormatterPostTkt>();
    _trx = _memHandle.create<AncillaryPricingTrx>();
    _trx->billing() = _memHandle.create<Billing>();
    AncRequest* req = _memHandle.create<AncRequest>();
    _trx->setRequest(req);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    req->ticketingAgent() = _memHandle.create<Agent>();
    Itin* i1 = _memHandle.create<Itin>();
    Itin* i2 = _memHandle.create<Itin>();
    Itin* masterItin = _memHandle.create<Itin>();
    req->masterItin() = masterItin;
    masterItin->setItinOrderNum(1);
    i1->setItinOrderNum(2);
    i2->setItinOrderNum(3);
    _trx->itin().push_back(i1);
    _trx->itin().push_back(i2);
    PaxType* p1 = createPaxType("ADT", "1", "12121212121212", "1.1");
    PaxType* p2 = createPaxType("UNN", "2", "23232323232323", "2.2");
    req->ancRequestType() = AncRequest::PostTktRequest;
    req->paxTypesPerItin()[i1].push_back(p1);
    req->paxTypesPerItin()[i2].push_back(p2);
    req->paxToOriginalItinMap()[p1] = i1;
    req->paxToOriginalItinMap()[p2] = i2;
    ServiceFeesGroup* s1 = _memHandle.create<ServiceFeesGroup>();
    ServiceFeesGroup* s2 = _memHandle.create<ServiceFeesGroup>();
    ServiceFeesGroup* s3 = _memHandle.create<ServiceFeesGroup>();
    ServiceFeesGroup* s4 = _memHandle.create<ServiceFeesGroup>();
    s1->groupCode() = s2->groupCode() = "AA";
    s3->groupCode() = s4->groupCode() = "BG";
    s1->state() = s2->state() = ServiceFeesGroup::EMPTY;
    s3->state() = s4->state() = ServiceFeesGroup::VALID;
    i1->ocFeesGroup() += s1, s3;
    i2->ocFeesGroup() += s2, s4;
    FarePath* fp1 = _memHandle.create<FarePath>();
    FarePath* fp2 = _memHandle.create<FarePath>();
    fp1->paxType() = p1;
    fp2->paxType() = p2;
    fp1->itin() = i1;
    fp2->itin() = i2;
    masterItin->travelSeg() += createTS("KRK", "FRA", 1, 1, 0), createTS("FRA", "DFW", 2, 2, 0),
        createTS("DFW", "SFO", 3, 3, 0);
    i1->travelSeg() += createTS("KRK", "FRA", 1, 1, 11), createTS("FRA", "DFW", 2, 2, 12);
    i2->travelSeg() += createTS("FRA", "DFW", 1, 2, 21), createTS("DFW", "SFO", 2, 3, 22);
    createOCFess(s3, fp1, i1->travelSeg()[0], i1->travelSeg()[0]);
    createOCFess(s3, fp1, i1->travelSeg()[0], i1->travelSeg()[1]);
    createOCFess(s3, fp1, i1->travelSeg()[1], i1->travelSeg()[1]);
    createOCFess(s4, fp2, i2->travelSeg()[0], i2->travelSeg()[0]);
    createOCFess(s4, fp2, i2->travelSeg()[0], i2->travelSeg()[1]);
    createOCFess(s4, fp2, i2->travelSeg()[1], i2->travelSeg()[1]);
  }
  void tearDown() { _memHandle.clear(); }

  void testGetOCFeesForR7()
  {
    _formater->getOCFeesForR7(*_trx, false);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _formater->_groupFeesVector.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _formater->_groupFeesVector[0].second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(5), _formater->_groupFeesVector[1].second.size());
  }
  void testFormatAEFeesResponse()
  {
    XMLConstruct construct;
    _formater->formatAEFeesResponse(construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "<ITN Q00=\"1\">"
            "<OCG Q00=\"0\" SF0=\"AA\"><MSG N06=\"X\" Q0K=\"3\" S18=\"NOT FOUND\"/></OCG>"
            "<OCG Q00=\"3\" SF0=\"BG\">"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"KRK\" A02=\"FRA\">"
            "<Q00>01</Q00>"
            //"<TCN Q86=\"1\"><C7A>11</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"KRK\" A02=\"DFW\"><Q00>01</Q00><Q00>02</Q00>"
            //"<TCN Q86=\"1\"><C7A>11</C7A><C7A>12</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"FRA\" A02=\"DFW\"><Q00>02</Q00>"
            //"<TCN Q86=\"1\"><C7A>12</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "</OCG>"
            "</ITN>"
            "<ITN Q00=\"2\">"
            "<OCG Q00=\"0\" SF0=\"AA\"><MSG N06=\"X\" Q0K=\"3\" S18=\"NOT FOUND\"/></OCG>"
            "<OCG Q00=\"3\" SF0=\"BG\">"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"KRK\" A02=\"FRA\"><Q00>01</Q00>"
            "<TCN Q86=\"1\"><C7A>11</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"KRK\" A02=\"DFW\"><Q00>01</Q00><Q00>02</Q00>"
            "<TCN Q86=\"1\"><C7A>11</C7A><C7A>12</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"FRA\" A02=\"DFW\"><Q00>02</Q00>"
            "<TCN Q86=\"1\"><C7A>12</C7A></TCN>"
            "<SUM B70=\"ADT\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "</OCG>"
            "</ITN>"
            "<ITN Q00=\"3\">"
            "<OCG Q00=\"0\" SF0=\"AA\"><MSG N06=\"X\" Q0K=\"3\" S18=\"NOT FOUND\"/></OCG>"
            "<OCG Q00=\"3\" SF0=\"BG\">"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"FRA\" A02=\"DFW\"><Q00>02</Q00>"
            "<TCN Q86=\"2\"><C7A>21</C7A></TCN>"
            "<SUM B70=\"UNN\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"FRA\" A02=\"SFO\"><Q00>02</Q00><Q00>03</Q00>"
            "<TCN Q86=\"2\"><C7A>21</C7A><C7A>22</C7A></TCN>"
            "<SUM B70=\"UNN\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "<OSC SHK=\"SUB\" SFV=\"VEND\" B01=\"BA\" N02=\" \">"
            "<OOS A01=\"DFW\" A02=\"SFO\"><Q00>03</Q00>"
            "<TCN Q86=\"2\"><C7A>22</C7A></TCN>"
            "<SUM B70=\"UNN\" C51=\"10\" C5A=\"BGP\" C52=\"0.000000\" C50=\"0.000000\"/><SFQ/>"
            "<DTE D01=\"1980-01-01\" D02=\"0000-00-00\"/><FAT/></OOS></OSC>"
            "</OCG>"
            "</ITN>"
            "<APR>"
            "<AGC SF0=\"BG\">"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"BG-                                  CXR SEG/CPA       "
            "   FEE\"/>"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"1  T1 -                               BA  1-KRKFRA   "
            "0.000000  \"/>"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"2  T1 -                               BA  1-KRKDFW   "
            "0.000000  \"/>"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"3  ALL-                               BA  2-FRADFW   "
            "0.000000  \"/>"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"4  T2 -                               BA  2-FRASFO   "
            "0.000000  \"/>"
            "<MSG N06=\"X\" Q0K=\"3\" S18=\"5  T2 -                               BA  3-DFWSFO   "
            "0.000000  \"/>"
            "</AGC>"
            "</APR>"),
        construct.getXMLData());
  }

  void testCheckRequestedGroup_NotRequested()
  {
    ServiceFeesGroup sg;
    sg.groupCode() = "AA";
    _formater->checkRequestedGroup(*_trx, sg.groupCode());
    CPPUNIT_ASSERT_EQUAL(false, _formater->_allRequestedProcessed);
  }

  void testCheckRequestedGroup_Requested_and_Match()
  {
    ServiceFeesGroup sg;
    //    sg.groupCode() = "AA";
    RequestedOcFeeGroup requestedOcFeeGroup;
    requestedOcFeeGroup.groupCode() = sg.groupCode() = "AA";
    _trx->getOptions()->serviceGroupsVec().push_back(requestedOcFeeGroup);
    _formater->checkRequestedGroup(*_trx, sg.groupCode());
    CPPUNIT_ASSERT_EQUAL(false, _formater->_allRequestedProcessed);
  }

  void testCheckRequestedGroup_Requested_not_Match()
  {
    ServiceFeesGroup sg;
    sg.groupCode() = "AA";
    RequestedOcFeeGroup requestedOcFeeGroup;
    requestedOcFeeGroup.groupCode() = "BB";
    _trx->getOptions()->serviceGroupsVec().push_back(requestedOcFeeGroup);
    _formater->checkRequestedGroup(*_trx, sg.groupCode());
    CPPUNIT_ASSERT_EQUAL(true, _formater->_allRequestedProcessed);
  }

private:
  TestMemHandle _memHandle;
  AncillaryPricingResponseFormatterPostTkt* _formater;
  AncillaryPricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingResponseFormatterPostTktTest);
}
