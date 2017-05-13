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

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Routing/MileageExclusion.h"

namespace tse
{
class MileageRoute;
class MileageRouteItem;
class Loc;

class MileageEqualization : public MileageExclusion
{
public:
  /**
   * Implements the apply interface for MileageEqualization.
   * @param MileageRoute A reference to the MileageRoute.
   * @returns true if MileageEqualization is applicable. False otherwise.
   */
  friend class MileageEqualizationTest;
  virtual bool apply(MileageRoute& route) const override;

private:
  MileageEqualization();
  MileageEqualization(const MileageExclusion& tpd) : _tpd(tpd) {}
  MileageEqualization(const MileageEqualization&);
  MileageEqualization& operator=(const MileageEqualization&);
  friend class tse::Singleton<MileageEqualization>;

  struct Target
  {
    size_t index;
    LocCode replacedCity;
    bool isReplacingCityInDest;
  };

  bool prepareTarget(const MileageRoute& route, Target& target) const;

  bool matchCriteria(const MileageRoute& route,
                     const Loc& oppositeLoc,
                     const LocCode& replacedCity) const;

  bool validateTicketedPoints(const MileageRoute& route, const LocCode& replacedCity) const;

  bool applyEqualization(MileageRoute& route, const Target& target) const;

  void fillSubstitutedRoute(MileageRoute& substitutedRoute,
                            const MileageRoute& route,
                            const Target& target) const;

  bool updateTPM(MileageRoute& route, MileageRouteItem& item) const;

  bool updateMPM(MileageRoute& route) const;

  void applyTPD(MileageRoute& route) const;

  LocCode replaceCity(const LocCode& city) const;

  const MileageExclusion& _tpd;
};

} // namespace tse

