//-------------------------------------------------------------------
//
//  File:        InterlineTicketCarrierData.h
//  Created:    October 19, 2010
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{

class PricingTrx;
class TravelSeg;
class InterlineTicketCarrierInfo;
class InterlineTicketCarrierStatus;

class InterlineTicketCarrier
{
  typedef std::set<CarrierCode> CarrierCodeSet;
  typedef std::map<std::pair<CarrierCode, CrsCode>, const InterlineTicketCarrierStatus*>
  CarrierStatusMap;

public:
  typedef std::vector<const InterlineTicketCarrierInfo*> Agreements;

  struct ValidateStatus
  {
    enum Status
    {
      OK,
      NO_AGREEMENT,
      NO_RELATIONSHIP_AGREEMENT,
      NO_VALID_PROFILE,
      EMPTY_SEGMENTS
    };

    enum ProfileStatus
    {
      PROFILE_OK,
      PROFILE_NOT_AVAILABLE,
      PROFILE_DEACTIVATED,
      PROFILE_NOT_INITIALIZED,
      PROFILE_BETA
    };

    explicit ValidateStatus(bool collectAgreements = true);

    Status status() const { return _status; }
    bool isOk() const { return _status == OK; }

    void setNoAgreement(const CarrierCode& carrier);
    void setNoRelationshipAgreement();
    void setEmptySegments();

    const CarrierCode& noAgreementCarrier() const;

    void addAgreement(const InterlineTicketCarrierInfo& interlineCarrier);
    const Agreements& agreements() const { return _agreements; }

    bool validatingCarrierParticipate() const { return _validatingCarrierParticipate; }
    void setValidatingCarrierParticipate() { _validatingCarrierParticipate = true; }

    bool normalRelationship() const { return _normalRelationship; }
    void setNormalRelationship() { _normalRelationship = true; }

    bool superPseudoInterlineRelationship() const { return _superPseudoInterlineRelationship; }
    void setSuperPseudoInterlineRelationship() { _superPseudoInterlineRelationship = true; }

    ProfileStatus profileStatus() const { return _profileStatus; }
    void setProfileStatus(const CrsCode& agentCode, ProfileStatus profileStatus);
    const CrsCode& agentCode() const { return _agentCode; }

    std::string toString() const;
    std::string profileStatusToString() const;

  private:
    bool _collectAgreements;
    Status _status;
    CarrierCode _noAgreementCarrier;
    Agreements _agreements;
    bool _validatingCarrierParticipate;
    bool _normalRelationship;
    bool _superPseudoInterlineRelationship;
    CrsCode _agentCode;
    ProfileStatus _profileStatus;
  };

  // interline ticketing carrier agreement data storage
  typedef std::map<CarrierCode, const InterlineTicketCarrierInfo*> CarrierInfoMap;
  typedef std::map<CarrierCode, CarrierInfoMap> InterlineTicketCarrierMap;

  const CarrierInfoMap& getInterlineCarriers(const PricingTrx& trx, const CarrierCode& carrier);

  void validateInterlineTicketCarrierAgreement(const PricingTrx& trx,
                                               const CarrierCode& validatingCarrier,
                                               const std::vector<TravelSeg*>& travelSegments,
                                               ValidateStatus& validateStatus);

  bool validateInterlineTicketCarrierAgreement(const PricingTrx& trx,
                                               const CarrierCode& validatingCarrier,
                                               const std::vector<TravelSeg*>& travelSegments,
                                               std::string* validationMessage = nullptr);

  std::string
  getInterlineCarriersForValidatingCarrier(const PricingTrx& trx, const CarrierCode& carrier);

  bool validateAgreementBetweenValidatingAndInterlineCarrier(const PricingTrx& trx,
                                                             const CarrierCode& validatingCarrier,
                                                             const CarrierCode& interlineCarrier);

  void validateInterlineTicketCarrierStatus(const PricingTrx& trx,
                                            const CarrierCode& validatingCarrier,
                                            const CrsCode& agentCode,
                                            ValidateStatus& validateStatus);

  static bool isPriceInterlineActivated(PricingTrx& trx);

private:
  bool isCarrierAgreementDataPrinted(const CarrierCode& carrier) const;
  const CarrierInfoMap&
  loadInterlineTicketCarrierDataFromDB(const PricingTrx& trx, const CarrierCode& carrier);

  void processAgreement(const CarrierInfoMap& interlineCarriers,
                        const CarrierCode& carrier,
                        ValidateStatus& validateStatus);

  const InterlineTicketCarrierStatus*
  getInterlineTicketCarrierStatus(const PricingTrx& trx,
                                  const CarrierCode& validatingCarrier,
                                  const CrsCode& agentCode);

private:
  InterlineTicketCarrierMap _interlineTicketCarrierMap;
  CarrierStatusMap _carrierStatusMap;
  CarrierCodeSet _interlineTicketCarrierPrinted;
};

} // namespace tse

