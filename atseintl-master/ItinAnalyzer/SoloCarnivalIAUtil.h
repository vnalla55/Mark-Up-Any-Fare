//-----------------------------------------------------------------------------
//
//  File:        SoloCarnivalIAUtil.h
//
//  Created:     Sep 27, 2011
//
//  Authors:     Artur de Sousa Rocha
//               Bartosz Kolpanowicz
//
//  Description: Sub-itinerary groups generator for the SOL Carnival project.
//
//  Copyright Sabre 2011
//
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "DataModel/SOLItinGroups.h"
#include "DataModel/TravelSeg.h"

#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{
class Itin;

class SOLSegInfo
{
public:
  SOLSegInfo(TravelSeg* ptr, size_t pos);

  static LocCode segBegin(const TravelSeg* ptr);
  static LocCode segEnd(const TravelSeg* ptr);
  static int16_t segLegId(const TravelSeg* ptr);

  bool gapOnTheLeft(const SOLSegInfo& left) const;

  LocCode segBegin() const;
  LocCode segEnd() const;
  int16_t segLegId() const;

  TravelSeg* segPtr() const;
  size_t segSegId() const;

private:
  TravelSeg* _segPtr;
  size_t _segSegId;
};

//---

class SOLItinInfo
{
public:
  SOLItinInfo();

  void fill(Itin* itin, size_t limit);

  bool isOneSegLeg(int16_t legId) const;
  bool isSegFirstOnLeg(size_t segId) const;
  bool isSegLastOnLeg(int16_t legId, size_t segId) const;
  size_t segType(int16_t legId, size_t segId) const;

  Itin*& itinPtr();
  size_t& itinType();
  const size_t& itinType() const;
  std::set<CarrierCode>& carriers();
  std::set<CarrierCode> const& carriers() const;
  std::map<int16_t, size_t>& legToNumSegs();
  std::map<int16_t, size_t> const& legToNumSegs() const;

private:
  Itin* _itinPtr;
  size_t _itinType;
  std::set<CarrierCode> _carriers;
  std::map<int16_t, size_t> _legToNumSegs;
};

//---

class SoloCarnivalIAUtil
{
  friend class SoloCarnivalIAUtilTest;

public:
  static void generateSubitins(PricingTrx& trx);
  static std::size_t genOptNumFields();
  static std::vector<int16_t> getItinHash(std::vector<TravelSeg*>& tSegs);
  static void removeExternalArunkSegs(std::vector<TravelSeg*>& tSegs);

private:
  static const std::size_t _genOptNumFields = 4; // bit 0    - generate (or not) the whole group,
  // bit 1..3 - generate (or not) if OW, RT, MD.
  SoloCarnivalIAUtil(PricingTrx& trx);
  void addNewItin(SOLItinGroups::ItinGroup& itinGroup,
                  std::list<SOLSegInfo>& tSegs,
                  bool removeExternalArunks = false);
  void createInterlineByCxrAndLeg();
  void createInterlineByCxrGrouping();
  void createOnlineByDomIntGrouping();
  void createOnlineByLeg();
  Itin* createSubitin(const std::vector<TravelSeg*>& tSegs);
  void createSubitinGroups();
  void sortSubitinGroups();
  static void sortGroupInTravelOrder(SOLItinGroups::ItinGroup& group, const Itin* original);
  void fillGapsInPath(std::list<SOLSegInfo>& pathInfo);
  void fillItinHashes();
  IntIndex findMaxItinNum() const; // to find out max itin num thru trx.itin()
  const std::vector<std::bitset<_genOptNumFields> >& generationOptions() const;
  IntIndex incItinNum(); // to generate unique itin num
  SOLItinGroups::ItinGroupVec& itinGroupVec();
  std::map<std::vector<int16_t>, Itin*>& itinHashes();
  SOLItinInfo& itinInfo();
  const SOLItinInfo& itinInfo() const;
  std::vector<std::bitset<_genOptNumFields> > parseGenerationOptions() const;
  void processInputItin(Itin* itin);
  bool shouldItinBeProcessed(SOLItinGroups::GroupType groupType) const;
  SOLItinGroups*& subitinGroups();
  std::vector<Itin*>& subitins();
  PricingTrx& trx();
  const PricingTrx& trx() const;

  // propagate booking codes from parent to sub itin
  class PropagateOrigBooking
  {
  public:
    PropagateOrigBooking(const Itin& fromParent, DataHandle& dh);
    void to(Itin& subItin) const;

  private:
    ClassOfService* lookupParentBKK(const TravelSeg* subItinSeg) const;

    const Itin& _parent;
    DataHandle& _dh;
  };

  PricingTrx& _trx;
  SOLItinGroups* _subitinGroups;
  std::vector<Itin*> _subitins;
  std::map<std::vector<int16_t>, Itin*> _itinHashes;
  SOLItinInfo _itinInfo; // Information on current itinerary.
  const std::vector<std::bitset<_genOptNumFields> > _generationOptions; // If and when to generate
                                                                        // groups and subitins.
  int _prevMaxItinNum;
};

// --- SoloCarnivalIAUtil class inline functions ---

inline const std::vector<std::bitset<SoloCarnivalIAUtil::_genOptNumFields> >&
SoloCarnivalIAUtil::generationOptions() const
{
  return _generationOptions;
}

inline std::size_t
SoloCarnivalIAUtil::genOptNumFields()
{
  return _genOptNumFields;
}

inline SOLItinGroups::ItinGroupVec&
SoloCarnivalIAUtil::itinGroupVec()
{
  return _subitinGroups->itinGroups();
}

inline std::map<std::vector<int16_t>, Itin*>&
SoloCarnivalIAUtil::itinHashes()
{
  return _itinHashes;
}

inline SOLItinInfo&
SoloCarnivalIAUtil::itinInfo()
{
  return _itinInfo;
}

inline const SOLItinInfo&
SoloCarnivalIAUtil::itinInfo() const
{
  return _itinInfo;
}

inline SOLItinGroups*&
SoloCarnivalIAUtil::subitinGroups()
{
  return _subitinGroups;
}

inline std::vector<Itin*>&
SoloCarnivalIAUtil::subitins()
{
  return _subitins;
}

inline PricingTrx&
SoloCarnivalIAUtil::trx()
{
  return _trx;
}

inline const PricingTrx&
SoloCarnivalIAUtil::trx() const
{
  return _trx;
}

// --- SoloCarnivalIAUtil::PropagateOrigBooking class inline functions ---

inline SoloCarnivalIAUtil::PropagateOrigBooking::PropagateOrigBooking(const Itin& fromParent,
                                                                      DataHandle& dh)
  : _parent(fromParent), _dh(dh)
{
}

// --- SOLSegInfo class inline functions ---

inline SOLSegInfo::SOLSegInfo(TravelSeg* ptr, size_t pos) : _segPtr(ptr), _segSegId(pos) {}

inline bool
SOLSegInfo::gapOnTheLeft(const SOLSegInfo& left) const
{
  if (left.segPtr())
    return left.segPtr()->offMultiCity() != segPtr()->boardMultiCity();
  else
    return true;
}

inline LocCode
SOLSegInfo::segBegin() const
{
  if (segPtr())
    return segPtr()->boardMultiCity();
  else
    return LocCode("");
}

inline LocCode
SOLSegInfo::segBegin(const TravelSeg* ptr)
{
  if (ptr)
    return ptr->boardMultiCity();
  else
    return LocCode("");
}

inline LocCode
SOLSegInfo::segEnd() const
{
  if (segPtr())
    return segPtr()->offMultiCity();
  else
    return LocCode("");
}

inline LocCode
SOLSegInfo::segEnd(const TravelSeg* ptr)
{
  if (ptr)
    return ptr->offMultiCity();
  else
    return LocCode("");
}

inline int16_t
SOLSegInfo::segLegId() const
{
  if (segPtr())
    return segPtr()->legId();
  else
    return -1;
}

inline int16_t
SOLSegInfo::segLegId(const TravelSeg* ptr)
{
  if (ptr)
    return ptr->legId();
  else
    return -1;
}

inline TravelSeg*
SOLSegInfo::segPtr() const
{
  return _segPtr;
}

inline size_t
SOLSegInfo::segSegId() const
{
  return _segSegId;
}

// --- SOLItinInfo class inline functions ---

inline SOLItinInfo::SOLItinInfo() : _itinPtr(nullptr), _itinType(0) {}

inline std::set<CarrierCode>&
SOLItinInfo::carriers()
{
  return _carriers;
}

inline const std::set<CarrierCode>&
SOLItinInfo::carriers() const
{
  return _carriers;
}

inline bool
SOLItinInfo::isOneSegLeg(int16_t legId) const
{
  std::map<int16_t, size_t>::const_iterator it = legToNumSegs().find(legId);

  if (legToNumSegs().end() == it)
    return false;

  return 1 == it->second;
}

inline bool
SOLItinInfo::isSegFirstOnLeg(size_t segId) const
{
  return 0 == segId;
}

inline bool
SOLItinInfo::isSegLastOnLeg(int16_t legId, size_t segId) const
{
  std::map<int16_t, size_t>::const_iterator it = legToNumSegs().find(legId);

  if (legToNumSegs().end() == it)
    return false;

  return segId + 1 == it->second;
}

inline Itin*&
SOLItinInfo::itinPtr()
{
  return _itinPtr;
}

/* Function: Returns the type of the current itinerary (OW/RT/MD, values 1-3), which corresponds
 * with
   *           the number of its legs.
   */
inline size_t&
SOLItinInfo::itinType()
{
  // OW = 1 leg, RT = 2 legs, MD = 3 and more legs.
  return _itinType;
}

inline const size_t&
SOLItinInfo::itinType() const
{
  return _itinType;
}

inline std::map<int16_t, size_t>&
SOLItinInfo::legToNumSegs()
{
  return _legToNumSegs;
}

inline const std::map<int16_t, size_t>&
SOLItinInfo::legToNumSegs() const
{
  return _legToNumSegs;
}

} // namespace tse

