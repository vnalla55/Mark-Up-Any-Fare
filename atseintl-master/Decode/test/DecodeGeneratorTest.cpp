//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Decode/DecodeGenerator.h"

#include "DataModel/DecodeTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "DBAccess/State.h"
#include "DBAccess/ZoneInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  const std::vector<AirlineAllianceCarrierInfo*>&
  getGenericAllianceCarrier(const GenericAllianceCode& genericAllianceCode)
  {
    std::vector<AirlineAllianceCarrierInfo*>* aacVec =
        _memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();
    AirlineAllianceCarrierInfo* aacInfo1 = _memHandle.create<AirlineAllianceCarrierInfo>();
    AirlineAllianceCarrierInfo* aacInfo2 = _memHandle.create<AirlineAllianceCarrierInfo>();
    aacVec->push_back(aacInfo1);
    aacVec->push_back(aacInfo2);

    aacInfo1->genericName() = "STAR ALLIANCE";
    aacInfo1->carrier() = "AA";

    aacInfo2->genericName() = "STAR ALLIANCE";
    aacInfo2->carrier() = "UA";

    return *aacVec;
  }

  std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
  {
    std::vector<TaxSpecConfigReg*>* tsConfigVec =
        _memHandle.create<std::vector<TaxSpecConfigReg*> >();
    TaxSpecConfigReg* tsConfig = _memHandle.create<TaxSpecConfigReg>();
    tsConfigVec->push_back(tsConfig);

    std::string desc =
        "WCC UNITED STATES WEST COAST CITIES/AIRORTS INCLUDES THE CITIES/AIRPORTS IN";
    tsConfig->setDescription(desc);

    TaxSpecConfigReg::TaxSpecConfigRegSeq* tsSeq1 = new TaxSpecConfigReg::TaxSpecConfigRegSeq();
    tsSeq1->paramValue() = ": CALIFORNIA-CA";
    tsConfig->seqs().push_back(tsSeq1);

    TaxSpecConfigReg::TaxSpecConfigRegSeq* tsSeq2 = new TaxSpecConfigReg::TaxSpecConfigRegSeq;
    tsSeq2->paramValue() = ", OREGON-OR.";
    tsConfig->seqs().push_back(tsSeq2);

    return *tsConfigVec;
  }

  const ZoneInfo*
  getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
  {
    ZoneInfo* zInfo = _memHandle.create<ZoneInfo>();
    ZoneInfo::ZoneSeg zSeg1;
    ZoneInfo::ZoneSeg zSeg2;
    ZoneInfo::ZoneSeg zSeg3;

    std::string desc = "CONTIGUOUS U.S.A.";
    zInfo->setDescription(desc);

    zSeg1.inclExclInd() = 'I';
    zSeg1.locType() = LOCTYPE_NATION;
    zSeg1.loc() = "US";

    zSeg2.inclExclInd() = 'E';
    zSeg2.locType() = LOCTYPE_STATE;
    zSeg2.loc() = "USAK";

    zSeg3.inclExclInd() = 'E';
    zSeg3.locType() = LOCTYPE_STATE;
    zSeg3.loc() = "USHI";

    zInfo->sets().resize(2);

    zInfo->sets()[0].push_back(zSeg1);
    zInfo->sets()[0].push_back(zSeg2);
    zInfo->sets()[1].push_back(zSeg3);

    return zInfo;
  }

  const State*
  getState(const NationCode& nationCode, const StateCode& stateCode, const DateTime& date)
  {
    State* state = _memHandle.create<State>();

    if (stateCode == "AK")
      state->description() = "ALASKA,USA";
    else if (stateCode == "HI")
      state->description() = "HAWAII,USA";

    return state;
  }

private:
  TestMemHandle _memHandle;
};
} // empty namespace

class DecodeGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DecodeGeneratorTest);

  CPPUNIT_TEST(testCrx);
  CPPUNIT_TEST(testCity);
  CPPUNIT_TEST(testZone);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  DecodeTrx* _trx;
  DecodeGenerator* _genCrx;
  DecodeGenerator* _genCity;
  DecodeGenerator* _genZone;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    LocCode locCode = "*S";
    _trx = _memHandle.create<DecodeTrx>();
    _genCrx = _memHandle.insert(new DecodeGenerator(*_trx, locCode));
    locCode = WestCoastCode;
    _genCity = _memHandle.insert(new DecodeGenerator(*_trx, locCode));
    locCode = "001";
    _genZone = _memHandle.insert(new DecodeGenerator(*_trx, locCode));
  }

  void tearDown() { _memHandle.clear(); }

  void testCrx()
  {
    _genCrx->generateAlianceCarrierList();

    CPPUNIT_ASSERT_EQUAL(std::string("*S STAR ALLIANCE\nINCLUDES AIRLINES AA, UA"),
                         _trx->getResponse());
  }

  void testCity()
  {
    _genCity->generateGenericCityList();

    CPPUNIT_ASSERT_EQUAL(std::string("WCC UNITED STATES WEST COAST CITIES/AIRORTS\n"
                                     "INCLUDES THE CITIES/AIRPORTS IN: CALIFORNIA-CA, OREGON-OR."),
                         _trx->getResponse());
  }

  void testZone()
  {
    _genZone->generateZoneList();

    CPPUNIT_ASSERT_EQUAL(
        std::string("001 ZONE - CONTIGUOUS U.S.A.\n"
                    "INCLUDES NATION OF UNITED STATES, EXCLUDES STATES OF ALASKA, HAWAII"),
        _trx->getResponse());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DecodeGeneratorTest);
}
