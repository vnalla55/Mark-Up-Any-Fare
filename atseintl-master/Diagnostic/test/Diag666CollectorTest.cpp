//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Agent.h"
#include "Diagnostic/Diag666Collector.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/Loc.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/ATSEv2Test.h"
#include "test/include/TestDataBuilders.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tse
{

namespace
{

class MyDataHandle : public DataHandleMock
{
public:
  const Mileage*
  getMileage(const LocCode& origin,
             const LocCode& destination,
             Indicator mileageType,
             const GlobalDirection globalDirection,
             const DateTime& dateTime) override
  {
    return nullptr;
  }

  const std::vector<CustomerActivationControl*>&
  getCustomerActivationControl(const std::string& projectCode) override
  {
    if (projectCode != "SRF")
      return DataHandleMock::getCustomerActivationControl(projectCode);

    CustomerActivationControl::CarrierActivation* carrierAct =
        _memHandle.create<CustomerActivationControl::CarrierActivation>();
    carrierAct->cxr() = "IB";
    carrierAct->cxrActDate() = DateTime::localTime();

    std::vector<CustomerActivationControl::CarrierActivation*> carrierActVec;
    carrierActVec.push_back(carrierAct);

    CustomerActivationControl* custActControl = _memHandle.create<CustomerActivationControl>();
    custActControl->cxrActivation() = carrierActVec;
    custActControl->projActvInd() = 'X';

    std::vector<CustomerActivationControl*>* custActControlVec = _memHandle.create<std::vector<CustomerActivationControl*>>();
    custActControlVec->push_back(custActControl);

    return *custActControlVec;
  }

private:
  TestMemHandle _memHandle;
};

} // empty namespace

class Diag666CollectorTest : public ATSEv2Test
{
public:
  void SetUp()
  {
    ATSEv2Test::SetUp();
    _memHandle.create<MyDataHandle>();
    TestConfigInitializer::setValue("SPANISH_TERRITORIES", "RBP|RRC|RRM|RCE", "SRFE");
    _diag = getDiagCollector<Diag666Collector>();
    _pTrx->setRequest(_memHandle.create<PricingRequest>());
    _pTrx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _itin = _memHandle.create<Itin>();
  }

  const Loc* createLoc(LocCode locCode)
  {
    if (locCode == "IBZ" || locCode == "MLN" || locCode == "PMI" ||
        locCode == "TCI" || locCode == "TFN" || locCode == "TFS")
    {
      Loc* loc = _memHandle.create<Loc>();
      loc->loc() = locCode;
      loc->nation() = "ES";
      loc->area() = IATA_AREA2;
      loc->subarea() = IATA_SUB_AREA_21();
      return loc;
    }
    return ATSEv2Test::createLoc(locCode);
  }

  Itin*
  createRoundTripItin(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
    ItinBuilder2 itinBuilder(_memHandle);
    return itinBuilder.addSegment(origin, destination, carrierCode)
                      .addSegment(destination, origin, carrierCode)
                      .setPuType(PricingUnit::Type::ROUNDTRIP)
                      .build();
  }

  Itin*
  createDestinationOpenJawItin(const Loc* origin, const Loc* destination1,
                               const Loc* destination2, CarrierCode carrierCode)
  {
    ItinBuilder2 itinBuilder(_memHandle);
    return itinBuilder.addSegment(origin, destination1, carrierCode)
                      .addArunkSegment(destination1, destination2)
                      .addSegment(destination2, origin, carrierCode)
                      .setPuType(PricingUnit::Type::OPENJAW)
                      .build();
  }

  PUPath*
  createRoundTripPUPath(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
    PUPathBuilder puPathBuilder(_memHandle);
    return puPathBuilder.addSegment(origin, destination, carrierCode)
                        .addFareMarket(carrierCode)
                        .addSegment(destination, origin, carrierCode)
                        .addFareMarket(carrierCode)
                        .setPuType(PricingUnit::Type::ROUNDTRIP)
                        .build();
  }

  PUPath*
  createDestinationOpenJawPUPath(const Loc* origin, const Loc* destination1,
                                 const Loc* destination2, CarrierCode carrierCode)
  {
    PUPathBuilder puPathBuilder(_memHandle);
    return puPathBuilder.addSegment(origin, destination1, carrierCode)
                        .addArunkSegment(destination1, destination2, carrierCode)
                        .addFareMarket(carrierCode)
                        .addSegment(destination2, origin, carrierCode)
                        .addFareMarket(carrierCode)
                        .setPuType(PricingUnit::Type::OPENJAW)
                        .build();
  }

  PUPath*
  createTwoOneWaysPUPath(const Loc* origin, const Loc* destination1,
                         const Loc* destination2, CarrierCode carrierCode)
  {
    PUPathBuilder puPathBuilder(_memHandle);
    return puPathBuilder.addSegment(origin, destination1, carrierCode)
                        .addFareMarket(carrierCode)
                        .setPuType(PricingUnit::Type::ONEWAY)
                        .addSegment(destination1, destination2, carrierCode)
                        .addFareMarket(carrierCode)
                        .setPuType(PricingUnit::Type::ONEWAY)
                        .build();
  }

protected:
  Diag666Collector* _diag;
  Itin* _itin;
};

TEST_F(Diag666CollectorTest, testprintAgent)
{
  Agent agent;
  Loc loc;
  loc.nation() = "ES";
  agent.agentLocation() = &loc;
  *_diag << agent;

  ASSERT_EQ("************************* AGENT'S INFO ************************\n"
            "NATION: ES\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintPassengerValidationApplicable)
{
  LocCode residencyCode = "IBZ";
  _diag->printPassengerValidation(residencyCode);
  ASSERT_EQ("*********************** PASSENGER'S INFO **********************\n"
            "RESIDENCY: IBZ\n"
            "STATE: RBP\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintPassengerValidationApplicableTFN)
{
  LocCode residencyCode = "TFN";
  _diag->printPassengerValidation(residencyCode);
  ASSERT_EQ("*********************** PASSENGER'S INFO **********************\n"
            "RESIDENCY: TFN\n"
            "STATE: RRC\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintPassengerValidationApplicableTFS)
{
  LocCode residencyCode = "TFS";
  _diag->printPassengerValidation(residencyCode);
  ASSERT_EQ("*********************** PASSENGER'S INFO **********************\n"
            "RESIDENCY: TFS\n"
            "STATE: RRC\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintPassengerValidationNotApplicable)
{
  LocCode residencyCode = "KRK";
  _diag->printPassengerValidation(residencyCode);
  ASSERT_EQ("*********************** PASSENGER'S INFO **********************\n"
            "RESIDENCY: KRK\n"
            "APPLICABLE FOR DISCOUNT: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintItinValidationRoundTrip)
{
  _pTrx->itin().push_back(createRoundTripItin(createLoc("IBZ"), createLoc("MAD"), "IB"));
  _pTrx->itin()[0]->travelSeg()[0]->arrivalDT() = DateTime(2016, 05, 01, 06, 00);
  _pTrx->itin()[0]->travelSeg()[1]->departureDT() = DateTime(2016, 05, 01, 18, 01);
  _diag->printItinValidation(*_pTrx);
  ASSERT_EQ("************************** ITINERARY **************************\n"
            "FLIGHTS:\n"
            "  IBZ(ES)-MAD(ES) CONX: 721\n"
            "  MAD(ES)-IBZ(ES)\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintItinValidationArunk)
{
  _pTrx->itin().push_back(createDestinationOpenJawItin(createLoc("IBZ"), createLoc("PMI"), createLoc("MAD"), "IB"));
  _diag->printItinValidation(*_pTrx);
  ASSERT_EQ("************************** ITINERARY **************************\n"
            "FLIGHTS:\n"
            "  IBZ(ES)-PMI(ES) CONX: 0\n"
            "  PMI(ES)-MAD(ES) ARUNK\n"
            "  MAD(ES)-IBZ(ES)\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());

}

TEST_F(Diag666CollectorTest, testPrintItinValidationNonApplicable)
{
  _pTrx->itin().push_back(createRoundTripItin(createLoc("IBZ"), createLoc("LON"), "IB"));
  _diag->printItinValidation(*_pTrx);
  ASSERT_EQ("************************** ITINERARY **************************\n"
            "FLIGHTS:\n"
            "  IBZ(ES)-LON(GB) CONX: 0\n"
            "  LON(GB)-IBZ(ES)\n"
            "APPLICABLE FOR DISCOUNT: N\n",
            _diag->str());

}

TEST_F(Diag666CollectorTest, testPrintItinValidationMip)
{
  _pTrx->itin().push_back(createRoundTripItin(createLoc("IBZ"), createLoc("MAD"), "IB"));
  _pTrx->setTrxType(PricingTrx::TrxType::MIP_TRX);
  _pTrx->itin()[0]->itinNum() = 1;
  _diag->printItinValidation(*_pTrx);
  ASSERT_EQ("************************** ITINERARY **************************\n"
            "ITIN NO: 1\n"
            "FLIGHTS:\n"
            "  IBZ(ES)-MAD(ES) CONX: 0\n"
            "  MAD(ES)-IBZ(ES)\n"
            "APPLICABLE FOR DISCOUNT: Y\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoRoundTrip)
{
  PUPath* puPath = createRoundTripPUPath(createLoc("IBZ"), createLoc("MAD"), "IB");
  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";
  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  RT: IBZ(RBP)-IB-MAD MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: Y\n"
            "APPLICABLE FOR DISCOUNT: Y\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoBadCarrier)
{
  PUPath* puPath = createRoundTripPUPath(createLoc("IBZ"), createLoc("MAD"), "LH");
  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";
  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  RT: IBZ(RBP)-LH-MAD MAD-LH-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  LH: N\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: Y\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoBadResidency)
{
  PUPath* puPath = createRoundTripPUPath(createLoc("IBZ"), createLoc("MAD"), "IB");
  addDiagArg("SP");
  _pTrx->residencyState() = "RCE";
  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  RT: IBZ(RBP)-IB-MAD MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: N\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoSurfaces)
{
  PUPath* puPath = createDestinationOpenJawPUPath(createLoc("IBZ"),
                                                  createLoc("PMI"),
                                                  createLoc("MAD"), "IB");
  addDiagArg("SP");
  _diag->printSolutionPatternInfo(*_pTrx, *puPath,*_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  OJ: IBZ(RBP)-IB-MAD MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: Y\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: N\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoLongConnection)
{
  CarrierCode carrierCode = "IB";
  const Loc* ibz = createLoc("IBZ");
  const Loc* bcn = createLoc("BCN");
  const Loc* mad = createLoc("MAD");

  PUPathBuilder puPathBuilder(_memHandle);
  PUPath* puPath = puPathBuilder.addSegment(ibz, bcn, carrierCode)
                                .addSegment(bcn, mad, carrierCode)
                                .addFareMarket(carrierCode)
                                .addSegment(mad, ibz, carrierCode)
                                .addFareMarket(carrierCode)
                                .setPuType(PricingUnit::Type::ROUNDTRIP)
                                .build();
  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";

  puPath->fareMarketPath()->fareMarketPath()[0]->travelSeg()[0]->arrivalDT()
      = DateTime(2016, 05, 01, 06, 00);
  puPath->fareMarketPath()->fareMarketPath()[0]->travelSeg()[1]->departureDT()
      = DateTime(2016, 05, 01, 18, 00);


  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  RT: IBZ(RBP)-IB-MAD MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: Y\n"
            "APPLICABLE FOR DISCOUNT: Y\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());

  _diag->str("");
  puPath->fareMarketPath()->fareMarketPath()[0]->travelSeg()[0]->arrivalDT()
      = DateTime(2016, 05, 01, 06, 00);
  puPath->fareMarketPath()->fareMarketPath()[0]->travelSeg()[1]->departureDT()
      = DateTime(2016, 05, 01, 18, 01);

  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  RT: IBZ(RBP)-IB-MAD MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: Y\n"
            "ORG/DST VALID: Y\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());

}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoDifferentDiscountTypes)
{
  PUPath* puPath = createTwoOneWaysPUPath(createLoc("IBZ"),
                                          createLoc("MAD"),
                                          createLoc("MLN"), "IB");

  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";

  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  OW: IBZ(RBP)-IB-MAD\n"
            "  OW: MAD-IB-MLN(RRM)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: N\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoNoResidencyOnOnePU)
{
  PUPath* puPath = createTwoOneWaysPUPath(createLoc("IBZ"),
                                          createLoc("MAD"),
                                          createLoc("BCN"), "IB");

  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";

  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  OW: IBZ(RBP)-IB-MAD\n"
            "  OW: MAD-IB-BCN\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: N\n"
            "APPLICABLE FOR DISCOUNT: N\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

TEST_F(Diag666CollectorTest, testPrintSolutionPatternInfoTheSameResidenciesOnPU)
{
  PUPath* puPath = createTwoOneWaysPUPath(createLoc("IBZ"),
                                          createLoc("MAD"),
                                          createLoc("IBZ"), "IB");

  addDiagArg("SP");
  _pTrx->residencyState() = "RBP";

  _diag->printSolutionPatternInfo(*_pTrx, *puPath, *_itin);
  ASSERT_EQ("*********************** SOLUTION PATTERN **********************\n"
            "PU:\n"
            "  OW: IBZ(RBP)-IB-MAD\n"
            "  OW: MAD-IB-IBZ(RBP)\n"
            "CXR ACTIVATION:\n"
            "  IB: Y\n"
            "SURFACES: N\n"
            "LONG CONNECTIONS: N\n"
            "ORG/DST VALID: Y\n"
            "APPLICABLE FOR DISCOUNT: Y\n"
            "SPANISH REFERENCE DATA: N\n",
            _diag->str());
}

} // tse
