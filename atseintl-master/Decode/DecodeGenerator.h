//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/ZoneInfo.h"

#include <tuple>

namespace tse
{
class DecodeTrx;

class DecodeGenerator
{
  friend class DecodeGeneratorTest;

  using NationStatelist = std::tuple<std::string, std::string, std::string, std::string>;
  static constexpr int INC_NATION = 0;
  static constexpr int EXC_NATION = 1;
  static constexpr int INC_STATE = 2;
  static constexpr int EXC_STATE = 3;

public:
  DecodeGenerator(DecodeTrx& trx, const LocCode& locCode) : _trx(trx), _locCode(locCode) {}

  void generate() const;

protected:
  void generateAlianceCarrierList() const;

  void generateGenericCityList() const;

  void generateZoneState(std::string& nationsList,
                         std::string& statesList,
                         const ZoneInfo::ZoneSeg& seg) const;
  void generateZoneList() const;

  void
  prepareZoneMessage(const std::string& zoneDesc, const NationStatelist& nationStatelist) const;

  DecodeTrx& _trx;
  const LocCode _locCode;

private:

  std::string getErrMsg() const;
};
} // end namespace tse
