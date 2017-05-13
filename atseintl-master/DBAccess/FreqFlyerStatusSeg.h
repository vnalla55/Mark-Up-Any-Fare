//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
static const uint8_t FF_LEVEL_NOT_DETERMINED = 10;

struct FreqFlyerTierStatusRow
{
  CarrierCode _carrier;
  CarrierCode _partnerCarrier;
  FreqFlyerTierLevel _partnerLevel = 0;
  FreqFlyerTierLevel _level = 0;
  DateTime _createDate;
  DateTime _effectiveDate;
  DateTime _expireDate;
  DateTime _discDate;
};

class FreqFlyerStatusSeg
{
public:
  struct PartnerStatusMapping
  {
    CarrierCode _partnerCarrier;
    FreqFlyerTierLevel _partnerLevel = FF_LEVEL_NOT_DETERMINED;
    FreqFlyerTierLevel _level = FF_LEVEL_NOT_DETERMINED;

    PartnerStatusMapping() = default;
    PartnerStatusMapping(const CarrierCode partnerCarrier,
                         const FreqFlyerTierLevel partnerLevel,
                         const FreqFlyerTierLevel level = FF_LEVEL_NOT_DETERMINED)
      : _partnerCarrier(partnerCarrier), _partnerLevel(partnerLevel), _level(level)
    {
    }

    bool operator<(const PartnerStatusMapping& other) const
    {
      if (_partnerLevel != other._partnerLevel)
        return _partnerLevel < other._partnerLevel;

      return _partnerCarrier < other._partnerCarrier;
    }

    bool operator==(const PartnerStatusMapping& other) const
    {
      return _partnerLevel == other._partnerLevel && _partnerCarrier == other._partnerCarrier;
    }

    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _partnerCarrier);
      FLATTENIZE(archive, _partnerLevel);
      FLATTENIZE(archive, _level);
    }
  };

  using PartnerStatusMap = std::vector<PartnerStatusMapping>;

private:
  CarrierCode _carrier;
  PartnerStatusMap _partnerStatusMap;
  DateTime _createDate;
  DateTime _effectiveDate;
  DateTime _expireDate;
  DateTime _discDate;

public:
  bool operator==(const FreqFlyerStatusSeg& rhs) const
  {
    return _carrier == rhs._carrier && _createDate == rhs._createDate &&
           _partnerStatusMap == rhs._partnerStatusMap && _effectiveDate == rhs._effectiveDate &&
           _expireDate == rhs._expireDate && _discDate == rhs._discDate;
  }

  void setCarrier(const CarrierCode carrier) { _carrier = carrier; }
  CarrierCode carrier() const { return _carrier; }

  void addPartnerLevel(const CarrierCode partnerCarrier,
                       const FreqFlyerTierLevel partnerLevel,
                       const FreqFlyerTierLevel level)
  {
    _partnerStatusMap.emplace_back(partnerCarrier, partnerLevel, level);
  }
  void sortPartnerStatusMap() { std::sort(_partnerStatusMap.begin(), _partnerStatusMap.end()); }
  const PartnerStatusMap& partnerStatusMap() const { return _partnerStatusMap; }

  void setCreateDate(const DateTime& createDate) { _createDate = createDate; }
  const DateTime& createDate() const { return _createDate; }

  void setEffectiveDate(const DateTime& effectiveDate) { _effectiveDate = effectiveDate; }
  const DateTime& effDate() const { return _effectiveDate; }

  void setExpireDate(const DateTime& expireDate) { _expireDate = expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  void setDiscDate(const DateTime& discDate) { _discDate = discDate; }
  const DateTime& discDate() const { return _discDate; }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _partnerStatusMap);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effectiveDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
  }
};
}
