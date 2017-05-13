//-------------------------------------------------------------------
//
//  File:        EmdValidator.h
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
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
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace tse
{
class PricingTrx;
class OCEmdDataProvider;
class Diag875Collector;
class EmdInterlineAgreementInfo;

class EmdValidator : boost::noncopyable
{

public:

  static const NationCode ALL_COUNTRIES;

  EmdValidator( PricingTrx& trx,
                OCEmdDataProvider& ocEmdDataProvider,
                Diag875Collector* diag);

  bool validate(std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >& carrierEmdInfoMap,
                boost::mutex& mutex,
                NationCode& nation) const;
  bool validate(const std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*>>& carrierEmdInfoMap) const;

  Diag875Collector* diag875() const { return _diag875; }

private:
  short checkPreValidation() const;
  bool getRecordsFromDatabase(std::vector<EmdInterlineAgreementInfo*>& eiaList,
                              std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >& carrierEmdInfoMap,
                              boost::mutex& mutex) const;
  bool checkRegularEmdAgreement(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const;
  bool checkEmdAgreement(const std::set<CarrierCode>& marketingCarriers,
                         const std::set<CarrierCode>& operatingCarriers,
                         const std::set<CarrierCode>& emdInfoParticipatingCarriers) const;

  bool checkEmdInterlineAgrrementMsc(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const;
  bool isDdInfo() const;
  bool isAvEMDIA() const;

  bool isAnyCarrierEmpty() const;
  bool isValidationNeeded() const;


  PricingTrx& _trx;
  OCEmdDataProvider& _ocEmdDataProvider;
  Diag875Collector* _diag875;

  friend class EmdValidatorTest;
};

} // tse namespace

