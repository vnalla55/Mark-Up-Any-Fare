#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag877Collector.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TaxResponse.h"
#include "Common/TseEnums.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/MockTseServer.h"

using namespace std;

namespace tse
{

class Diag877CollectorTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag877CollectorTaxTest);
  CPPUNIT_TEST(testTaxResponse_OneOcFees_OneTax);
  CPPUNIT_TEST(testTaxResponse_TwoOcFees_TwoTax);
  CPPUNIT_TEST(testTaxResponse_TwoGroups_OneOcFees_OneTax);
  CPPUNIT_TEST(testTaxResponse_TwoMarkets_OneOcFees_OneTax);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag877Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

  TseServer* _server;
  PricingTrx* _trx;
  PricingRequest* _req;
  Agent* _agent;
  PricingOptions* _opt;
  TaxResponse* _taxResponse;
  FarePath* _farePath;
  Itin* _itin;
  ServiceFeesGroup* _sGroup;
  PaxType* _pt;
  std::map<const FarePath*, std::vector<OCFees*> >* _ocFeesMap;
  std::vector<OCFees*>* _ocFeesVector;
  OCFees* _ocFees;
  OptionalServicesInfo* _optFee;

public:
  void setUp()
  {
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic877));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag877Collector(*_diagroot));
      _diag->enable(Diagnostic877);
      _server = _memHandle.create<MockTseServer>();
      _trx = _memHandle.create<PricingTrx>();
      _diag->trx() = _trx;
      _req = _memHandle.create<PricingRequest>();
      _trx->setRequest(_req);
      _agent = _memHandle.create<Agent>();
      _req->ticketingAgent() = _agent;
      _opt = _memHandle.create<PricingOptions>();
      _trx->setOptions(_opt);
      _taxResponse = _memHandle.create<TaxResponse>();
      _farePath = _memHandle.create<FarePath>();
      _itin = _memHandle.create<Itin>();
      _sGroup = _memHandle.create<ServiceFeesGroup>();
      _taxResponse->farePath() = _farePath;
      _farePath->itin() = _itin;
      _pt = _memHandle.create<PaxType>();
      _farePath->paxType() = _pt;
      _ocFeesMap = _memHandle.create<std::map<const FarePath*, std::vector<OCFees*> > >();
      _ocFeesVector = _memHandle.create<std::vector<OCFees*> >();
      _ocFees = _memHandle.create<OCFees>();
      _optFee = _memHandle.create<OptionalServicesInfo>();
      _ocFees->optFee() = _optFee;
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void testTaxResponse_OneOcFees_OneTax()
  {
    _agent->currencyCodeAgent() = "USD";
    _optFee->serviceSubTypeCode() = "AA";
    Loc originLoc, destinationLoc;
    originLoc.loc() = "DFW";
    destinationLoc.loc() = "KTW";
    AirSeg seg1, seg2;
    seg1.origin() = &originLoc;
    seg2.destination() = &destinationLoc;
    _ocFees->travelStart() = &seg1;
    _ocFees->travelEnd() = &seg2;
    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("OCB");
    taxItem.setTaxAmount(10);
    taxItem.setNumberOfDec(2);
    _ocFees->addTax(taxItem);

    _pt->paxType() = "ADT";
    _itin->ocFeesGroup().push_back(_sGroup);
    (*_ocFeesMap)[_farePath] = *_ocFeesVector;
    (*_ocFeesMap)[_farePath].push_back(_ocFees);
    _sGroup->ocFeesMap() = *_ocFeesMap;
    _sGroup->groupCode() = "BB";

    (*_diag) << *_taxResponse;

    CPPUNIT_ASSERT_EQUAL(string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                                "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n"
                                "*************************** TAXES *****************************\n"
                                "------------ PORTION OF TRAVEL : DFW - KTW -----------------\n"
                                "**************************************************************\n"
                                " GROUP    : BB \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"),
                         _diag->str());
  }

  void testTaxResponse_TwoOcFees_TwoTax()
  {
    _agent->currencyCodeAgent() = "USD";
    _optFee->serviceSubTypeCode() = "AA";
    Loc originLoc, destinationLoc;
    originLoc.loc() = "DFW";
    destinationLoc.loc() = "KTW";
    AirSeg seg1, seg2;
    seg1.origin() = &originLoc;
    seg2.destination() = &destinationLoc;
    _ocFees->travelStart() = &seg1;
    _ocFees->travelEnd() = &seg2;
    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("OCB");
    taxItem.setTaxAmount(10);
    taxItem.setNumberOfDec(2);
    _ocFees->addTax(taxItem);
    taxItem.setTaxCode("OCH");
    taxItem.setTaxAmount(20);
    _ocFees->addTax(taxItem);

    _pt->paxType() = "ADT";
    _itin->ocFeesGroup().push_back(_sGroup);
    (*_ocFeesMap)[_farePath] = *_ocFeesVector;
    (*_ocFeesMap)[_farePath].push_back(_ocFees);
    (*_ocFeesMap)[_farePath].push_back(_ocFees);
    _sGroup->ocFeesMap() = *_ocFeesMap;
    _sGroup->groupCode() = "BB";

    (*_diag) << *_taxResponse;

    CPPUNIT_ASSERT_EQUAL(string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                                "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n"
                                "*************************** TAXES *****************************\n"
                                "------------ PORTION OF TRAVEL : DFW - KTW -----------------\n"
                                "**************************************************************\n"
                                " GROUP    : BB \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"
                                "                                 20.00USD OCH\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"
                                "                                 20.00USD OCH\n"),
                         _diag->str());
  }

  void testTaxResponse_TwoGroups_OneOcFees_OneTax()
  {
    _agent->currencyCodeAgent() = "USD";
    _optFee->serviceSubTypeCode() = "AA";
    Loc originLoc, destinationLoc;
    originLoc.loc() = "DFW";
    destinationLoc.loc() = "KTW";
    AirSeg seg1, seg2;
    seg1.origin() = &originLoc;
    seg2.destination() = &destinationLoc;
    _ocFees->travelStart() = &seg1;
    _ocFees->travelEnd() = &seg2;
    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("OCB");
    taxItem.setTaxAmount(10);
    taxItem.setNumberOfDec(2);
    _ocFees->addTax(taxItem);

    ServiceFeesGroup* _sGroup2 = _memHandle.create<ServiceFeesGroup>();
    _pt->paxType() = "ADT";
    _itin->ocFeesGroup().push_back(_sGroup);
    _itin->ocFeesGroup().push_back(_sGroup2);
    (*_ocFeesMap)[_farePath] = *_ocFeesVector;
    (*_ocFeesMap)[_farePath].push_back(_ocFees);
    _sGroup->ocFeesMap() = *_ocFeesMap;
    _sGroup->groupCode() = "BB";
    _sGroup2->ocFeesMap() = *_ocFeesMap;
    _sGroup2->groupCode() = CARRY_ON_CHARGE;

    (*_diag) << *_taxResponse;

    CPPUNIT_ASSERT_EQUAL(string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                                "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n"
                                "*************************** TAXES *****************************\n"
                                "------------ PORTION OF TRAVEL : DFW - KTW -----------------\n"
                                "**************************************************************\n"
                                " GROUP    : BB \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"
                                "**************************************************************\n"
                                " GROUP    : CC \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"),
                         _diag->str());
  }

  void testTaxResponse_TwoMarkets_OneOcFees_OneTax()
  {
    OCFees* _ocFees2 = _memHandle.create<OCFees>();
    _ocFees2->optFee() = _optFee;
    _agent->currencyCodeAgent() = "USD";
    _optFee->serviceSubTypeCode() = "AA";
    Loc originLoc, destinationLoc, dest2Loc;
    originLoc.loc() = "DFW";
    destinationLoc.loc() = "KTW";
    dest2Loc.loc() = "FCO";
    AirSeg seg1, seg2, seg3;
    seg1.origin() = &originLoc;
    seg2.destination() = &destinationLoc;
    seg3.destination() = &dest2Loc;
    _ocFees->travelStart() = &seg1;
    _ocFees->travelEnd() = &seg2;
    _ocFees2->travelStart() = &seg1;
    _ocFees2->travelEnd() = &seg3;
    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("OCB");
    taxItem.setTaxAmount(10);
    taxItem.setNumberOfDec(2);
    _ocFees->addTax(taxItem);

    _pt->paxType() = "ADT";
    _itin->ocFeesGroup().push_back(_sGroup);
    (*_ocFeesMap)[_farePath] = *_ocFeesVector;
    (*_ocFeesMap)[_farePath].push_back(_ocFees);
    (*_ocFeesMap)[_farePath].push_back(_ocFees2);
    _sGroup->ocFeesMap() = *_ocFeesMap;
    _sGroup->groupCode() = "BB";

    (*_diag) << *_taxResponse;

    CPPUNIT_ASSERT_EQUAL(string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                                "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n"
                                "*************************** TAXES *****************************\n"
                                "------------ PORTION OF TRAVEL : DFW - FCO -----------------\n"
                                "**************************************************************\n"
                                " GROUP    : BB \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "------------ PORTION OF TRAVEL : DFW - KTW -----------------\n"
                                "**************************************************************\n"
                                " GROUP    : BB \n"
                                " REQUESTED PAX TYPE : ADT\n"
                                "-------------------------------------------------------------- \n"
                                "                  S7 RECORDS DATA PROCESSING\n"
                                "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n"
                                "   AA             0                  FREE PASS\n"
                                "                                 10.00USD OCB\n"),
                         _diag->str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag877CollectorTaxTest);
} // tse
