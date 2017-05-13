//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include <vector>
#include <map>

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"

#include "Common/Assert.h"

#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "DBAccess/InterlineTicketCarrierStatus.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

namespace
{

class InterlineTicketDataHandle : public DataHandleMock
{
  typedef std::vector<InterlineTicketCarrierInfo*> CarriersInfo;
  typedef std::map<CarrierCode, CarriersInfo> CarriersInfoMap;
  typedef std::map<std::pair<CarrierCode, CrsCode>, const InterlineTicketCarrierStatus*>
  CarriersStatusMap;

public:
  InterlineTicketDataHandle()
  {
    {
      _carriersInfoMap["NO"] = CarriersInfo();
      addStatus("NO", "1S", 'A');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["AA"];
      carriersInfo.push_back(&create("AA", "BA"));
      addStatus("AA", "1S", 'A');
      addStatus("AA", "DS", 'D');
      addStatus("AA", "NS", 'N');
      addStatus("AA", "BS", 'B');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["CC"];
      carriersInfo.push_back(&create("CC", "AA"));
      carriersInfo.push_back(&create("CC", "BA"));
      addStatus("CC", "1S", 'A');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["DD"];
      carriersInfo.push_back(&create("DD", "AA", 'Y'));
      carriersInfo.push_back(&create("DD", "BA", 'Y'));
      carriersInfo.push_back(&create("DD", "DL", 'Y'));
      addStatus("DD", "1S", 'A');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["EE"];
      carriersInfo.push_back(&create("EE", "AA", 'N', 'Y'));
      carriersInfo.push_back(&create("EE", "BA", 'N', 'Y'));
      carriersInfo.push_back(&create("EE", "DL", 'N', 'Y'));
      addStatus("EE", "1S", 'A');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["FF"];
      carriersInfo.push_back(&create("FF", "AA", 'N', 'N', 'Y'));
      carriersInfo.push_back(&create("FF", "BA"));
      carriersInfo.push_back(&create("FF", "DL"));
      addStatus("FF", "1S", 'A');
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["GG"];
      carriersInfo.push_back(&create("GG", "AA", 'N', 'Y'));
      carriersInfo.push_back(&create("GG", "BA", 'N', 'Y'));
      carriersInfo.push_back(&create("GG", "DL", 'N', 'Y'));
    }

    {
      CarriersInfo& carriersInfo = _carriersInfoMap["ZZ"];
      carriersInfo.push_back(&create("ZZ", "A1", 'N', 'N', 'N'));
      carriersInfo.push_back(&create("ZZ", "A2", 'N', 'N', 'Y'));
      carriersInfo.push_back(&create("ZZ", "A3", 'N', 'Y', 'N'));
      carriersInfo.push_back(&create("ZZ", "A4", 'Y', 'N', 'N'));
    }
  }

  const std::vector<InterlineTicketCarrierInfo*>&
  getInterlineTicketCarrier(const CarrierCode& carrier, const DateTime& date)
  {
    CarriersInfoMap::const_iterator carrierIt = _carriersInfoMap.find(carrier);
    TSE_ASSERT(carrierIt != _carriersInfoMap.end());
    return carrierIt->second;
  }

  const InterlineTicketCarrierStatus*
  getInterlineTicketCarrierStatus(const CarrierCode& carrier, const CrsCode& crsCode)
  {
    CarriersStatusMap::const_iterator carrierIt =
        _carriersStatusMap.find(std::make_pair(carrier, crsCode));
    const InterlineTicketCarrierStatus* status;
    if (carrierIt != _carriersStatusMap.end())
    {
      status = carrierIt->second;
    }
    else
    {
      status = 0;
    }

    return status;
  }

private:
  InterlineTicketCarrierInfo& create(const CarrierCode& carrier,
                                     const CarrierCode& interlineCarrier,
                                     const Indicator& hostInterline = 'N',
                                     const Indicator& pseudoInterline = 'N',
                                     const Indicator& superPseudoInterline = 'N')
  {
    InterlineTicketCarrierInfo& carrierInfo = *_memHandle.create<InterlineTicketCarrierInfo>();
    carrierInfo.carrier() = carrier;
    carrierInfo.interlineCarrier() = interlineCarrier;
    carrierInfo.hostInterline() = hostInterline;
    carrierInfo.pseudoInterline() = pseudoInterline;
    carrierInfo.superPseudoInterline() = superPseudoInterline;
    return carrierInfo;
  }

  void addStatus(const CarrierCode& carrierCode, const CrsCode& agentCode, const Indicator& status)
  {
    InterlineTicketCarrierStatus& carrierStatus =
        *_memHandle.create<InterlineTicketCarrierStatus>();
    carrierStatus.status() = status;
    _carriersStatusMap[std::make_pair(carrierCode, agentCode)] = &carrierStatus;
  }

private:
  TestMemHandle _memHandle;
  CarriersInfoMap _carriersInfoMap;
  CarriersStatusMap _carriersStatusMap;
};

} // unnamed namespace

class InterlineTicketCarrierDataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(InterlineTicketCarrierDataTest);
  CPPUNIT_TEST(testNoAgreement);
  CPPUNIT_TEST(testNoAgreementOperatingCarrier1);
  CPPUNIT_TEST(testNoAgreementOperatingCarrier2);
  CPPUNIT_TEST(testNoAgreementOperatingCarrier3);
  CPPUNIT_TEST(testAgreementValidateCarrierParticipate);
  CPPUNIT_TEST(testValidatingCoversJourney);
  CPPUNIT_TEST(testNoRelationshipAgreement);
  CPPUNIT_TEST(testAgreementHostInterline);
  CPPUNIT_TEST(testAgreementPseudoInterline);
  CPPUNIT_TEST(testAgreementSuperPseudoInterline);
  CPPUNIT_TEST(testCheckProfileFallback);
  CPPUNIT_TEST(testCollectAgreements);
  CPPUNIT_TEST(testGetInterlineCarriersForValidatingCarrier);
  CPPUNIT_TEST(testValidateAgreementBetweenValidatingAndInterlineCarrier);
  CPPUNIT_TEST(testGetInterlineCarriers);
  CPPUNIT_TEST(testValidateInterlineTicketCarrierStatusNotAvailable);
  CPPUNIT_TEST(testValidateInterlineTicketCarrierStatusDeactivated);
  CPPUNIT_TEST(testValidateInterlineTicketCarrierStatusNotInitialized);
  CPPUNIT_TEST(testValidateInterlineTicketCarrierStatusBeta);
  CPPUNIT_TEST_SUITE_END();

  void constructTravelSegments1(std::vector<TravelSeg*>& travelSegments)
  {
    return constructTravelSegments(travelSegments, "AA", "AA", "BA", "DL");
  }

  void constructTravelSegments2(std::vector<TravelSeg*>& travelSegments)
  {
    return constructTravelSegments(travelSegments, "AA", "", "BA", "");
  }

  void constructTravelSegments3(std::vector<TravelSeg*>& travelSegments)
  {
    return constructTravelSegments(travelSegments, "AA", "", "AA", "");
  }

  void constructTravelSegments4(std::vector<TravelSeg*>& travelSegments)
  {
    return constructTravelSegments(travelSegments, "AA", "9R", "AA", "");
  }

  void constructTravelSegments(std::vector<TravelSeg*>& travelSegments,
                               const char* carrier1,
                               const char* operatingCarrier1,
                               const char* carrier2,
                               const char* operatingCarrier2)
  {
    const char* const LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
    const char* const LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";

    AirSeg* airSeg1 = _memHandle.create<AirSeg>();
    AirSeg* airSeg2 = _memHandle.create<AirSeg>();

    airSeg1->origin() = TestLocFactory::create(LOC_DFW);
    airSeg1->destination() = TestLocFactory::create(LOC_LON);
    airSeg2->origin() = TestLocFactory::create(LOC_LON);
    airSeg2->destination() = TestLocFactory::create(LOC_DFW);
    airSeg1->segmentOrder() = 0;
    airSeg2->segmentOrder() = 1;
    airSeg1->carrier() = carrier1;
    airSeg1->setOperatingCarrierCode(operatingCarrier1);
    airSeg2->carrier() = carrier2;
    airSeg2->setOperatingCarrierCode(operatingCarrier2);
    travelSegments.push_back(airSeg1);
    travelSegments.push_back(airSeg2);
  }

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  Agent* _agent;
  TestMemHandle _memHandle;
  InterlineTicketCarrier _interlineTicketCarrier;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _agent->cxrCode() = "1S";
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _memHandle.create<InterlineTicketDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testNoAgreement()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments1(travelSegments);

    std::string validationMessage;
    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "NO", travelSegments, &validationMessage);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'NO' has ticketing agreement", !result);
    CPPUNIT_ASSERT_EQUAL(std::string("NO INTERLINE TICKETING AGREEMENT WITH AA"),
                         validationMessage);

    result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "NO", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'NO' has ticketing agreement", !result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "NO", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_AGREEMENT, status.status());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), status.noAgreementCarrier());
    CPPUNIT_ASSERT_EQUAL(std::string("NO INTERLINE TICKETING AGREEMENT WITH AA"),
                         status.toString());
  }

  void testNoAgreementOperatingCarrier1()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments1(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has ticketing agreement", !result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_AGREEMENT, status.status());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("DL"), status.noAgreementCarrier());
    CPPUNIT_ASSERT_EQUAL(std::string("NO INTERLINE TICKETING AGREEMENT WITH DL"),
                         status.toString());
  }

  void testNoAgreementOperatingCarrier2()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments1(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "CC", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'CC' has ticketing agreement", !result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "CC", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_AGREEMENT, status.status());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("DL"), status.noAgreementCarrier());
    CPPUNIT_ASSERT_EQUAL(std::string("NO INTERLINE TICKETING AGREEMENT WITH DL"),
                         status.toString());
  }

  void testNoAgreementOperatingCarrier3()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments4(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has ticketing agreement", !result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_AGREEMENT, status.status());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("9R"), status.noAgreementCarrier());
    CPPUNIT_ASSERT_EQUAL(std::string("NO INTERLINE TICKETING AGREEMENT WITH 9R"),
                         status.toString());
  }

  void testNoRelationshipAgreement()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "CC", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'CC' has ticketing agreement", !result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "CC", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_RELATIONSHIP_AGREEMENT,
                         status.status());
    CPPUNIT_ASSERT_EQUAL(std::string("NO VALID TICKETING AGREEMENT"), status.toString());
  }

  void testAgreementValidateCarrierParticipate()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has no ticketing agreement", result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(true, status.validatingCarrierParticipate());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testValidatingCoversJourney()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments3(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has no ticketing agreement", result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "AA", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testAgreementHostInterline()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "DD", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'DD' has no ticketing agreement", result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "DD", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'DD' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(false, status.validatingCarrierParticipate());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testAgreementPseudoInterline()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "EE", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'EE' has no ticketing agreement", result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "EE", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'EE' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(false, status.validatingCarrierParticipate());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testAgreementSuperPseudoInterline()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    bool result = _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "FF", travelSegments);
    CPPUNIT_ASSERT_MESSAGE("Carrier 'FF' has no ticketing agreement", result);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "FF", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'FF' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(false, status.validatingCarrierParticipate());
    CPPUNIT_ASSERT_EQUAL(true, status.superPseudoInterlineRelationship());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testCheckProfileFallback()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "GG", travelSegments, status);

    CPPUNIT_ASSERT_MESSAGE("Carrier 'GG' has no ticketing agreement", status.isOk());
    CPPUNIT_ASSERT_EQUAL(false, status.validatingCarrierParticipate());
    CPPUNIT_ASSERT_EQUAL(std::string("OK"), status.toString());
  }

  void testCollectAgreements()
  {
    std::vector<TravelSeg*> travelSegments;
    constructTravelSegments2(travelSegments);

    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierAgreement(
        *_trx, "CC", travelSegments, status);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), status.agreements().size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), status.agreements().at(0)->interlineCarrier());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), status.agreements().at(1)->interlineCarrier());
  }

  void testGetInterlineCarriersForValidatingCarrier()
  {
    const std::string& message =
        _interlineTicketCarrier.getInterlineCarriersForValidatingCarrier(*_trx, "ZZ");
    CPPUNIT_ASSERT_EQUAL(std::string("\n INTERLINE CARRIERS FOR VALIDATING CARRIER ZZ:\n   "
                                     "A1:N A2:S A3:P A4:H \n"
                                     " S - SUPER PSEUDO INTERLINE AGREEMENT\n"
                                     " P - PSEUDO INTERLINE AGREEMENT\n"
                                     " H - HOSTED INTERLINE AGREEMENT\n"
                                     " N - NORMAL INTERLINE AGREEMENT\n"),
                         message);
  }

  void testValidateAgreementBetweenValidatingAndInterlineCarrier()
  {
    bool result = _interlineTicketCarrier.validateAgreementBetweenValidatingAndInterlineCarrier(
        *_trx, "AA", "DL");
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has ticketing agreement with DL", !result);

    result = _interlineTicketCarrier.validateAgreementBetweenValidatingAndInterlineCarrier(
        *_trx, "AA", "BA");
    CPPUNIT_ASSERT_MESSAGE("Carrier 'AA' has no ticketing agreement with BA", result);
  }

  void testGetInterlineCarriers()
  {
    const InterlineTicketCarrier::CarrierInfoMap& carriersMap =
        _interlineTicketCarrier.getInterlineCarriers(*_trx, "CC");
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), carriersMap.size());
    InterlineTicketCarrier::CarrierInfoMap::const_iterator it = carriersMap.begin();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), it->first);
    ++it;
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), it->first);
  }

  void testValidateInterlineTicketCarrierStatusNotAvailable()
  {
    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierStatus(*_trx, "AA", "NA", status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_VALID_PROFILE, status.status());
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::PROFILE_NOT_AVAILABLE,
                         status.profileStatus());
    CPPUNIT_ASSERT_EQUAL(std::string("IET PROFILE NOT AVAILABLE"), status.profileStatusToString());
  }

  void testValidateInterlineTicketCarrierStatusDeactivated()
  {
    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierStatus(*_trx, "AA", "DS", status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_VALID_PROFILE, status.status());
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::PROFILE_DEACTIVATED,
                         status.profileStatus());
    CPPUNIT_ASSERT_EQUAL(std::string("CRS DS - DEACTIVATED"), status.profileStatusToString());
  }

  void testValidateInterlineTicketCarrierStatusNotInitialized()
  {
    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierStatus(*_trx, "AA", "NS", status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_VALID_PROFILE, status.status());
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::PROFILE_NOT_INITIALIZED,
                         status.profileStatus());
    CPPUNIT_ASSERT_EQUAL(std::string("CRS NS - NOT INITIALIZED"), status.profileStatusToString());
  }

  void testValidateInterlineTicketCarrierStatusBeta()
  {
    InterlineTicketCarrier::ValidateStatus status;
    _interlineTicketCarrier.validateInterlineTicketCarrierStatus(*_trx, "AA", "BS", status);
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::NO_VALID_PROFILE, status.status());
    CPPUNIT_ASSERT_EQUAL(InterlineTicketCarrier::ValidateStatus::PROFILE_BETA,
                         status.profileStatus());
    CPPUNIT_ASSERT_EQUAL(std::string("CRS BS - BETA"), status.profileStatusToString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(InterlineTicketCarrierDataTest);

} // namespace tse
