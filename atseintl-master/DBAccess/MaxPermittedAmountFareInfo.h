//-------------------------------------------------------------------
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "Util/TaggedTuple.h"

namespace tse
{
namespace tag
{
struct OriginAirport : Tag<LocCode>
{
};
struct OriginCity : Tag<LocCode>
{
};
struct OriginNation : Tag<LocCode>
{
};
struct DestAirport : Tag<NationCode>
{
};
struct DestCity : Tag<NationCode>
{
};
struct DestNation : Tag<NationCode>
{
};
struct SSequence : Tag<SequenceNumber>
{
};
struct Loc1 : Tag<LocKey>
{
};
struct Loc2 : Tag<LocKey>
{
};
struct CreateDate : Tag<DateTime>
{
};
struct EffDate : Tag<DateTime>
{
};
struct ExpireDate : Tag<DateTime>
{
};
struct DiscDate : Tag<DateTime>
{
};
struct LastModDate : Tag<DateTime>
{
};
struct SCurrencyCode : Tag<CurrencyCode>
{
};
struct MarketPrice : Tag<MoneyAmount>
{
};
struct MarketPriceNoDec : Tag<int>
{
};
}

class MaxPermittedAmountFareInfo : public TaggedTuple<tag::OriginAirport,
                                                      tag::OriginCity,
                                                      tag::OriginNation,
                                                      tag::DestAirport,
                                                      tag::DestCity,
                                                      tag::DestNation,
                                                      tag::SSequence,
                                                      tag::Loc1,
                                                      tag::Loc2,
                                                      tag::CreateDate,
                                                      tag::EffDate,
                                                      tag::ExpireDate,
                                                      tag::DiscDate,
                                                      tag::LastModDate,
                                                      tag::SCurrencyCode,
                                                      tag::MarketPrice,
                                                      tag::MarketPriceNoDec>
{
public:
  MaxPermittedAmountFareInfo() {}

  const DateTime& createDate() const { return get<tag::CreateDate>(); }

  const DateTime& effDate() const { return get<tag::EffDate>(); }

  const DateTime& discDate() const { return get<tag::DiscDate>(); }

  const DateTime& expireDate() const { return get<tag::ExpireDate>(); }

  static void dummyData(MaxPermittedAmountFareInfo& obj)
  {
    using namespace tag;
    obj.get<OriginAirport>() = "AGP";
    obj.get<OriginCity>() = "AGP";
    obj.get<OriginNation>() = "ES";
    obj.get<DestAirport>() = "PMI";
    obj.get<DestCity>() = "PMI";
    obj.get<DestNation>() = "ES";
    obj.get<SSequence>() = 1234567;
    LocKey loc1;
    loc1.locType() = 'P';
    loc1.loc() = "AGP";
    obj.get<Loc1>() = loc1;
    LocKey loc2;
    loc2.locType() = 'P';
    loc2.loc() = "PMI";
    obj.get<Loc2>() = loc2;
    obj.get<CreateDate>() = std::time(nullptr);
    obj.get<EffDate>() = std::time(nullptr);
    obj.get<ExpireDate>() = std::time(nullptr);
    obj.get<DiscDate>() = std::time(nullptr);
    obj.get<SCurrencyCode>() = "USD";
    obj.get<MarketPrice>() = 100.0;
    obj.get<MarketPriceNoDec>() = 2;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    using namespace tag;
    FLATTENIZE(archive, get<OriginAirport>());
    FLATTENIZE(archive, get<OriginCity>());
    FLATTENIZE(archive, get<OriginNation>());
    FLATTENIZE(archive, get<DestAirport>());
    FLATTENIZE(archive, get<DestCity>());
    FLATTENIZE(archive, get<DestNation>());
    FLATTENIZE(archive, get<SSequence>());
    FLATTENIZE(archive, get<Loc1>());
    FLATTENIZE(archive, get<Loc2>());
    FLATTENIZE(archive, get<CreateDate>());
    FLATTENIZE(archive, get<EffDate>());
    FLATTENIZE(archive, get<ExpireDate>());
    FLATTENIZE(archive, get<DiscDate>());
    FLATTENIZE(archive, get<LastModDate>());
    FLATTENIZE(archive, get<SCurrencyCode>());
    FLATTENIZE(archive, get<MarketPrice>());
    FLATTENIZE(archive, get<MarketPriceNoDec>());
  }
};
} // tse
