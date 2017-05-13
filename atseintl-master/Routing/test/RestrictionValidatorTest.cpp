//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/include/GtestHelperMacros.h"
#include "Routing/RestrictionValidator.cpp"

#include <memory>

namespace tse
{
using namespace ::testing;

class RestrictionValidatorTestableMock : public RestrictionValidator
{
public:
    MOCK_METHOD2(getState, const StateCode(DataHandle&, const LocCode&));
    MOCK_METHOD2(getZoneInfo, const ZoneInfo*(DataHandle&, const LocCode&));

    bool validate(const TravelRoute&, const RoutingRestriction&, const PricingTrx&)
    {
        return true;
    }

    bool locateBetween(DataHandle& data,
                        const TravelRouteList& tvlRouteList,
                        const MarketData& marketData1,
                        const MarketData& marketData2,
                        TravelRouteItr& boardPoint,
                        TravelRouteItr& offPoint)
    {
        return RestrictionValidator::locateBetween(data, tvlRouteList, marketData1, marketData2, boardPoint, offPoint);
    }

    bool searchViaPoint(DataHandle& data,
                        const MarketData& viaMarketData,
                        TravelRouteItr city1I,
                        TravelRouteItr city2I)
    {
        return RestrictionValidator::searchViaPoint(data, viaMarketData, city1I, city2I);
    }
};

class RestrictionValidatorTest : public Test
{
public:
    void SetUp()
    {
        _zoneInfo.reset(new ZoneInfo());

        ZoneInfo::ZoneSeg zoneSeg1;
        _zoneInfo->sets().resize(1);
        _zoneInfo->sets()[0].resize(2);

        _zoneInfo->sets()[0][0] = prepareZoneSeg("GB");
        _zoneInfo->sets()[0][1] = prepareZoneSeg("US");

        _market1 = std::make_pair('C', "XYZ");
        _market2 = std::make_pair('C', "ZYX");

        _tvlRoute.travelRoute().push_back(prepareCity("ABC", "AAA", "DE", "PL"));
        _tvlRoute.travelRoute().push_back(prepareCity("ABC", "DEF", "LP", "US"));
        _tvlRoute.travelRoute().push_back(prepareCity("ABC", "GHI", "GG", "US"));
        _tvlRoute.travelRoute().push_back(prepareCity("DEF", "JKL", "RD", "AA"));
        _tvlRoute.travelRoute().push_back(prepareCity("GHI", "MNO", "RU", "GE"));
        _tvlRoute.travelRoute().push_back(prepareCity("JKL", "PRS", "UR", "XY"));
    }
    void TearDown() {}

    TravelRoute::CityCarrier prepareCity(LocCode board, LocCode off,
                                         NationCode boardNation, NationCode offNation)
    {
        TravelRoute::CityCarrier cityCarrier;
        cityCarrier.boardNation() = boardNation;
        cityCarrier.offNation() = offNation;

        TravelRoute::City city1, city2;
        city1.loc() = board;
        city2.loc() = off;

        cityCarrier.boardCity() = city1;
        cityCarrier.offCity() = city2;

        return cityCarrier;
    }

    ZoneInfo::ZoneSeg prepareZoneSeg(const LocCode& nation)
    {
        ZoneInfo::ZoneSeg zoneSeg;
        zoneSeg.locType() = LOCTYPE_NATION;
        zoneSeg.loc() = nation;
        return zoneSeg;
    }

    std::shared_ptr<ZoneInfo> _zoneInfo;
    TravelRoute _tvlRoute;
    RestrictionValidator::MarketData _market1;
    RestrictionValidator::MarketData _market2;
    DataHandle _data;
    RestrictionValidatorTestableMock::TravelRouteItr _city1I;
    RestrictionValidatorTestableMock::TravelRouteItr _city2I;
    RestrictionValidatorTestableMock _restrictionValidator;
};

TEST_F(RestrictionValidatorTest, testLocateBetween_emptyTravelRouteList)
{
    _tvlRoute.travelRoute().clear();
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_incorrectOneMarket)
{
    _market1 = std::make_pair('C', "JKL");
    _market2 = std::make_pair('C', "XXX");
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_incorrectBothMarkets)
{
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_missingCityBetween2Markets)
{
    _market1 = std::make_pair('C', "JKL");
    _market2 = std::make_pair('C', "PRS");
    ASSERT_TRUE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
    ASSERT_EQ(_market1.second, _city1I->boardCity().loc());
}

TEST_F(RestrictionValidatorTest, testLocateBetween_correctValidation)
{
    _market1 = std::make_pair('C', "DEF");
    _market2 = std::make_pair('C', "MNO");
    ASSERT_TRUE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
    ASSERT_EQ(_market1.second, _city1I->boardCity().loc());
    ASSERT_EQ("PRS", _city2I->offCity().loc());
    ASSERT_EQ(_market2.second, (--_city2I)->offCity().loc());
}

TEST_F(RestrictionValidatorTest, testLocateBetween_emptyTravelRouteListNation)
{
    _market1 = std::make_pair('N', "XX");
    _market2 = std::make_pair('N', "YY");
    _tvlRoute.travelRoute().clear();
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_incorrectOneMarketNation)
{
    _market1 = std::make_pair('N', "US");
    _market2 = std::make_pair('N', "YY");
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_incorrectBothMarketsNation)
{
    _market1 = std::make_pair('N', "XX");
    _market2 = std::make_pair('N', "YY");
    ASSERT_FALSE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_missingCityBetween2MarketsNation)
{
    _market1 = std::make_pair('N', "UR");
    _market2 = std::make_pair('N', "XY");
    ASSERT_TRUE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testLocateBetween_correctValidationNation)
{
    _market1 = std::make_pair('N', "RD");
    _market2 = std::make_pair('N', "GE");
    ASSERT_TRUE(_restrictionValidator.locateBetween(_data, _tvlRoute.travelRoute(), _market1, _market2, _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCityNotFound)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "XXX");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsBoardCity)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "ABC");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsOffCity)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "AAA");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_correctValidation)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "PRS");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCityNotFoundNation)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('N', "GB");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsBoardCityNation)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('N', "DE");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsOffCityNation)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('N', "PL");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_correctValidationNation)
{
    RestrictionValidator::MarketData viaMarket = std::make_pair('N', "XY");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCityNotFoundGenericCityCode)
{
    EXPECT_CALL(_restrictionValidator, getState(_, _)).WillRepeatedly(Return("WCC"));

    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "USXX");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsBoardCityGenericCityCode)
{
    EXPECT_CALL(_restrictionValidator, getState(_, _)).WillRepeatedly(Return("WCC"));

    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "USYY");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsOffCityGenericCityCode)
{
    _tvlRoute.travelRoute()[0].offCity().loc() = "WCC";
    EXPECT_CALL(_restrictionValidator, getState(_, _))
            .WillOnce(Return("USWA"));

    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "WCC");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_correctValidationGenericCityCode)
{
    _tvlRoute.travelRoute()[2].offCity().loc() = "ECC";
    EXPECT_CALL(_restrictionValidator, getState(_, _))
            .WillOnce(Return("USYY"))
            .WillOnce(Return("USYY"))
            .WillOnce(Return("USNC"));

    RestrictionValidator::MarketData viaMarket = std::make_pair('C', "ECC");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCityZone)
{
    _zoneInfo.reset();
    EXPECT_CALL(_restrictionValidator, getZoneInfo(_, _)).WillRepeatedly(Return(_zoneInfo.get()));

    RestrictionValidator::MarketData viaMarket = std::make_pair('Z', "210");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsBoardCityZone)
{
    EXPECT_CALL(_restrictionValidator, getZoneInfo(_, _)).WillOnce(Return(_zoneInfo.get()));

    RestrictionValidator::MarketData viaMarket = std::make_pair('Z', "210");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_FALSE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_viaCitySameAsOffCityZone)
{
    _zoneInfo->sets()[0][0] = prepareZoneSeg("GB");
    _zoneInfo->sets()[0][1] = prepareZoneSeg("PL");
    EXPECT_CALL(_restrictionValidator, getZoneInfo(_, _)).WillRepeatedly(Return(_zoneInfo.get()));

    RestrictionValidator::MarketData viaMarket = std::make_pair('Z', "210");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = ++_tvlRoute.travelRoute().begin();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

TEST_F(RestrictionValidatorTest, testSearchViaPoint_correctValidationZone)
{
    EXPECT_CALL(_restrictionValidator, getZoneInfo(_, _)).WillRepeatedly(Return(_zoneInfo.get()));

    RestrictionValidator::MarketData viaMarket = std::make_pair('Z', "210");
    _city1I = _tvlRoute.travelRoute().begin();
    _city2I = _tvlRoute.travelRoute().end();
    ASSERT_TRUE(_restrictionValidator.searchViaPoint(_data, viaMarket,  _city1I, _city2I));
}

} // namespace tse
