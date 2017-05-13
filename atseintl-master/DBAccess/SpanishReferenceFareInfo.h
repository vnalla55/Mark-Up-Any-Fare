#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Util/TaggedTuple.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
namespace tag
{
struct TktCarrier : Tag<CarrierCode> {};
struct FareCarrier : Tag<CarrierCode> {};
struct OriginAirport : Tag<LocCode> {};
struct DestinationAirport : Tag<LocCode> {};
struct ViaPointOneAirport : Tag<LocCode> {};
struct ViaPointTwoAirport : Tag<LocCode> {};
struct CreateDate : Tag<DateTime> {};
struct EffDate : Tag<DateTime> {};
struct ExpireDate : Tag<DateTime> {};
struct DiscDate : Tag<DateTime> {};
struct LastModDate : Tag<DateTime> {};
struct SCurrencyCode : Tag<CurrencyCode> {};
struct FareAmount : Tag<MoneyAmount> {};
struct FareAmountNoDec : Tag<int> {};
}

class SpanishReferenceFareInfo : public TaggedTuple<tag::TktCarrier,
                                                    tag::FareCarrier,
                                                    tag::OriginAirport,
                                                    tag::DestinationAirport,
                                                    tag::ViaPointOneAirport,
                                                    tag::ViaPointTwoAirport,
                                                    tag::CreateDate,
                                                    tag::EffDate,
                                                    tag::ExpireDate,
                                                    tag::DiscDate,
                                                    tag::LastModDate,
                                                    tag::SCurrencyCode,
                                                    tag::FareAmount,
                                                    tag::FareAmountNoDec>
{
public:

  const DateTime& createDate() const { return get<tag::CreateDate>(); }

  const DateTime& effDate() const { return get<tag::EffDate>(); }

  const DateTime& discDate() const { return get<tag::DiscDate>(); }

  const DateTime& expireDate() const { return get<tag::ExpireDate>(); }

  static void dummyData(SpanishReferenceFareInfo& obj)
  {
    using namespace tag;
    obj.get<TktCarrier>() = "IB";
    obj.get<FareCarrier>() = "AU";
    obj.get<OriginAirport>() = "AGP";
    obj.get<DestinationAirport>() = "PMI";
    obj.get<ViaPointOneAirport>() = "MAD";
    obj.get<ViaPointTwoAirport>() = "MAD";
    obj.get<CreateDate>() = std::time(nullptr);
    obj.get<EffDate>() = std::time(nullptr);
    obj.get<ExpireDate>() = std::time(nullptr);
    obj.get<DiscDate>() = std::time(nullptr);
    obj.get<LastModDate>() = std::time(nullptr);
    obj.get<SCurrencyCode>() = "USD";
    obj.get<FareAmount>() = 100.0;
    obj.get<FareAmountNoDec>() = 2;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    using namespace tag;
    FLATTENIZE(archive, get<TktCarrier>());
    FLATTENIZE(archive, get<FareCarrier>());
    FLATTENIZE(archive, get<OriginAirport>());
    FLATTENIZE(archive, get<DestinationAirport>());
    FLATTENIZE(archive, get<ViaPointOneAirport>());
    FLATTENIZE(archive, get<ViaPointTwoAirport>());
    FLATTENIZE(archive, get<CreateDate>());
    FLATTENIZE(archive, get<EffDate>());
    FLATTENIZE(archive, get<ExpireDate>());
    FLATTENIZE(archive, get<DiscDate>());
    FLATTENIZE(archive, get<LastModDate>());
    FLATTENIZE(archive, get<SCurrencyCode>());
    FLATTENIZE(archive, get<FareAmount>());
    FLATTENIZE(archive, get<FareAmountNoDec>());
  }
};
}// tse
