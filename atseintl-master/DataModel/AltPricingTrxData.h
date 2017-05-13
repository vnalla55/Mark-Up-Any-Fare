//-------------------------------------------------------------------
//
//  File:        AltPricingTrxData.h
//
//  Copyright Sabre 2015
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

#include <string>
#include <vector>

namespace tse
{
struct AltPricingTrxData
{
  class AccompRestrictionInfo
  {
  public:
    uint16_t& selectionNumber() { return _selectionNumber; }
    uint16_t selectionNumber() const { return _selectionNumber; }

    std::string& validationStr() { return _validationStr; }
    const std::string& validationStr() const { return _validationStr; }

    bool& guaranteed() { return _guaranteed; }
    bool guaranteed() const { return _guaranteed; }

    std::string& selectionXml() { return _selectionXml; }
    const std::string& selectionXml() const { return _selectionXml; }

    bool& surfaceRestricted() { return _surfaceRestricted; }
    bool surfaceRestricted() const { return _surfaceRestricted; }

  private:
    uint16_t _selectionNumber = 0;
    std::string _validationStr;
    bool _guaranteed = 0;
    std::string _selectionXml;
    bool _surfaceRestricted = 0;
  };

  using AccompRestrictionVec = std::vector<AccompRestrictionInfo>;

  AccompRestrictionVec& accompRestrictionVec() { return _accompRestrictionVec; }
  const AccompRestrictionVec& accompRestrictionVec() const { return _accompRestrictionVec; }

  bool& wtfrFollowingWp() { return _wtfrFollowingWp; }
  bool wtfrFollowingWp() const { return _wtfrFollowingWp; }

  bool isPriceSelectionEntry() { return !_accompRestrictionVec.empty(); }

  std::string& agentXml() { return _agentXml; }
  const std::string& agentXml() const { return _agentXml; }

  std::string& billingXml() { return _billingXml; }
  const std::string& billingXml() const { return _billingXml; }

  bool& noMatchPricing() { return _noMatchWpaWP; }
  bool noMatchPricing() const { return _noMatchWpaWP; }

  AccompRestrictionVec _accompRestrictionVec;
  std::string _agentXml;
  std::string _billingXml;
  bool _wtfrFollowingWp = false;
  bool _noMatchWpaWP = false;
};
} // namespace tse
