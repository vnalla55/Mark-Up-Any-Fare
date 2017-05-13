//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "DataModel/PricingTrx.h"
#include "DataModel/SimilarItinData.h"

#include <vector>
#include <map>
namespace tse
{
class RexExchangeTrx;
class Itin;
class FamilyLogicSplitter
{
  friend class ItinAnalyzerService_CheckItinFamilyTest;
  friend class ItinAnalyzerService_CheckItinTest;
  friend class ItinAnalyzerService_FindCOSKeysTest;

public:
  using AvailScoreMap = std::map<Itin*, double>;
  using KeyScore = std::pair<PricingTrx::ClassOfServiceKey, double>;
  using KeyScoreVector = std::vector<KeyScore>;
  using ItinAvailKeys = std::map<Itin*, KeyScoreVector>;

  FamilyLogicSplitter(PricingTrx& trx, uint16_t reqNumberOfSeats)
    : _trx(trx), _requestedNumberOfSeats(reqNumberOfSeats)
  {
  }

  void splitItinFamilies(bool splitItinsForDomesticOnly);

private:
  void checkItinFamily(Itin*& itin,
                       std::vector<Itin*>& splitItins,
                       bool splitItinsForDomesticOnly,
                       bool checkChildren);
  Itin* createFamily(std::vector<SimilarItinData>& itinVec);

  bool processBookingCodeForSegments(const PricingTrx::ClassOfServiceKey& key,
                                     Itin* itin,
                                     uint16_t* score,
                                     bool calculateLocalJourneyScore = false,
                                     bool useLeastAvail = false);

  bool processBookingCodeForSegments(const PricingTrx::ClassOfServiceKey& key,
                                     Itin* itin,
                                     const std::vector<ClassOfService*>& origBooking);

  void getBestItinKeys(Itin* itin, KeyScoreVector& keyScoreVector);
  bool processFlowJourneyCarriers(Itin* itin,
                                  const std::vector<TravelSegJourneyType>& legJourneyOrder,
                                  const PricingTrx::ClassOfServiceKey& key,
                                  KeyScoreVector& keyScoreVector);
  bool processBookingCodes(Itin* itin,
                           const KeyScoreVector& keys,
                           const std::vector<ClassOfService*>& origBooking);

  void splitFamiliesBasedOnTravel();
  void splitFamiliesBasedOnBookedAvailibility(ItinAvailKeys& itinAvailKeys,
                                              bool useMotherAvail,
                                              uint16_t numSplits);
  void splitFamiliesBasedOnCabinAvailibility(AvailScoreMap& availScoreMap);
  void addNewMothers(std::vector<Itin*>& itinsToSplit,
                     std::vector<Itin*>& newMothers,
                     uint16_t splitSize,
                     AvailScoreMap* availScoreMap,
                     std::stringstream& diag);
  void printUpdatedMother(Itin* updatedItin, AvailScoreMap& availScoreMap, std::stringstream& diag);
  void eraseItins(const std::vector<Itin*>& erasedItins);
  bool processLocalJourneyCarriers(Itin* itin,
                                   const std::vector<TravelSegJourneyType>& legJourneyOrder,
                                   const PricingTrx::ClassOfServiceKey& key,
                                   KeyScoreVector& keyScoreVector);
  void displayKeySegments(const PricingTrx::ClassOfServiceKey& key);
  void processNonJourneyCarriers(Itin* itin,
                                 const PricingTrx::ClassOfServiceKey& key,
                                 KeyScoreVector& keyScoreVector);

  void diag982(const std::vector<Itin*>& splittedItins,
               const Itin* excItin,
               const std::vector<Itin*>& excRemovedItins,
               const std::vector<Itin*>& excSplittedItins);
  bool checkExcItinFamily(RexExchangeTrx* excTrx, Itin* itin, std::vector<Itin*>& splitItins);
  void getItinCOSKeys(ItinAvailKeys& itinAvailKeys, AvailScoreMap& cabinAvailScores);
  bool checkItin(Itin* itin, const std::vector<ClassOfService*>& origBooking);
  std::vector<PricingTrx::ClassOfServiceKey> findAllCOSKeys(Itin* itin);
  void cleanInvalidatedItins(Itin* mother, std::vector<Itin*>& invalidatedItinVec);
  void cleanInvalidatedItins(Itin* mother, std::vector<SimilarItinData>& invalidatedItinVec);

  PricingTrx& _trx;
  const uint16_t _requestedNumberOfSeats;
};
}
