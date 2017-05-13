//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#ifndef PRICING_MOCK_DATA_BUILDER__H
#define PRICING_MOCK_DATA_BUILDER_H


namespace tse
{
class PricingTrx;
class Itin;
class Loc;
class PaxType;
class FareMarket;
class MergedFareMarket;
class TravelSeg;
class PU;
class PUPath;

class PricingMockDataBuilder
{
public:
  static tse::PricingTrx* getPricingTrx();
  static tse::Itin* addItin(tse::PricingTrx& trx);

  static tse::PaxType* addPaxType(tse::PricingTrx& trx, const std::string& paxTypeCode);

  static tse::Loc* getLoc(tse::PricingTrx& trx, const std::string& code, const std::string& nation);
  static tse::Loc* getLoc(tse::PricingTrx& trx, const std::string& code)
  {
    return getLoc(trx, code, "US");
  }

  static tse::TravelSeg* addTravelSeg(tse::PricingTrx& trx,
                                      tse::Itin& itin,
                                      const std::string& carrier,
                                      Loc* orig,
                                      Loc* dest,
                                      uint32_t segmentOrder);
  static tse::FareMarket* addFareMarket(
      tse::PricingTrx& trx, tse::Itin& itin, const std::string& carrier, Loc* orig, Loc* dest);

  static void addFareToFareMarket(tse::PricingTrx& trx,
                                  tse::Itin& itin,
                                  const std::string carrier,
                                  FareMarket& fareMarket);
  static tse::PUPath* getPUPath(tse::PricingTrx& trx);

  static tse::PU*
  addOWPUToMainTrip(tse::PricingTrx& trx, tse::PUPath& puPath, FareMarket* fm1, Directionality dir);

  static tse::PU* addOJPUToMainTrip(tse::PricingTrx& trx,
                                    tse::PUPath& puPath,
                                    FareMarket* fm1,
                                    FareMarket* fm2,
                                    Directionality dir1,
                                    Directionality dir2);

  static tse::PU*
  addRTPUToMainTrip(tse::PricingTrx& trx, tse::PUPath& puPath, FareMarket* fm1, FareMarket* fm2);

  static tse::PU* addCTPUToMainTrip(
      tse::PricingTrx& trx, tse::PUPath& puPath, FareMarket* fm1, FareMarket* fm2, FareMarket* fm3);

  static MergedFareMarket* getMergedFareMarket(tse::PricingTrx& trx, FareMarket* fm);

  static void addTraveSegToFareMarket(tse::TravelSeg* tseg, FareMarket& fm);
};
}

#endif
