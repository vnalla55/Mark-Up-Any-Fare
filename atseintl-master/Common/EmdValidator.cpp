//-------------------------------------------------------------------
//
//  File:        EmdValidator.cpp
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

#include "Common/EmdValidator.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "Diagnostic/Diag875Collector.h"
#include "ServiceFees/OCEmdDataProvider.h"

#include <boost/bind.hpp>

namespace tse
{
static Logger
logger("atseintl.ServiceFees.EmdValidator");
const NationCode EmdValidator::ALL_COUNTRIES = "ZZ";

EmdValidator::EmdValidator( PricingTrx& trx,
                            OCEmdDataProvider& ocEmdDataProvider,
                            Diag875Collector* diag)
  : _trx(trx),
    _ocEmdDataProvider(ocEmdDataProvider),
    _diag875(diag)
{
}

bool
EmdValidator::validate(std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >& carrierEmdInfoMap,
                       boost::mutex& mutex,
                       NationCode& nation) const
{

  if(nation.empty())
  {
    const Loc* pointOfSaleLocation = TrxUtil::saleLoc(_trx);
    if(pointOfSaleLocation)
    {
      const boost::lock_guard<boost::mutex> guard(mutex);
      nation = pointOfSaleLocation->nation();
    }
  }
  _ocEmdDataProvider.nation() = nation;

  if (_trx.getRequest() && _trx.getRequest()->ticketingAgent())
    _ocEmdDataProvider.gds() = _trx.getRequest()->ticketingAgent()->cxrCode();

  if(_ocEmdDataProvider.nation().empty() || _ocEmdDataProvider.gds().empty())
  {
    LOG4CXX_DEBUG(logger,
                "Leaving EmdValidator::validate() - Missing nation or gds");
    return false;
  }



  int rc = checkPreValidation();

  if(rc != -1 )
    return (rc ? true : false);

  std::vector<EmdInterlineAgreementInfo*> eiaList;
  if(!getRecordsFromDatabase(eiaList, carrierEmdInfoMap, mutex))
  {
    if (isDdInfo())
      _diag875->printNoInterlineDataFoundInfo();
    return false;
  }
  else if (isAvEMDIA())
    _diag875->printDetailInterlineEmdAgreementInfo(eiaList);

  return checkRegularEmdAgreement(eiaList);
}

bool
EmdValidator::validate(const std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*>>& carrierEmdInfoMap) const
{
  if(isDdInfo())
    _diag875->printDetailInterlineEmdProcessingS5Info(_ocEmdDataProvider.nation(),
                                                      _ocEmdDataProvider.gds(),
                                                      _ocEmdDataProvider.emdValidatingCarrier(),
                                                      _ocEmdDataProvider.marketingCarriers(),
                                                      _ocEmdDataProvider.operatingCarriers());

  if(isAnyCarrierEmpty())
  {
    if(isDdInfo())
      _diag875->printDetailInterlineEmdProcessingStatusS5Info(false);
    return false;
  }

  if(!isValidationNeeded())
  {
    if(isDdInfo())
      _diag875->printDetailInterlineEmdProcessingStatusS5Info(true);
    return true;
  }

  auto carrierEmdInfoMapIterator = carrierEmdInfoMap.find(_ocEmdDataProvider.emdValidatingCarrier());
  if(carrierEmdInfoMapIterator == carrierEmdInfoMap.end())
  {
    if (isDdInfo())
      _diag875->printNoInterlineDataFoundInfo();
    return false;
  }

  const std::vector<EmdInterlineAgreementInfo*>& eiaList = carrierEmdInfoMapIterator->second;
  if (isAvEMDIA())
    _diag875->printDetailInterlineEmdAgreementInfo(eiaList);

  return checkRegularEmdAgreement(eiaList) && checkEmdInterlineAgrrementMsc(eiaList);
}

bool EmdValidator::isAnyCarrierEmpty() const
{
  return _ocEmdDataProvider.operatingCarriers().empty() ||
        _ocEmdDataProvider.marketingCarriers().empty() ||
        _ocEmdDataProvider.emdValidatingCarrier().empty();
}

bool EmdValidator::isValidationNeeded() const
{
  if( _ocEmdDataProvider.operatingCarriers().size() == 1 && _ocEmdDataProvider.marketingCarriers().size() == 1
    && ( *_ocEmdDataProvider.operatingCarriers().begin() == *_ocEmdDataProvider.marketingCarriers().begin()) )
  {
    if( *_ocEmdDataProvider.operatingCarriers().begin() == _ocEmdDataProvider.emdValidatingCarrier() )
    {
      return false;
    }
  }
  return true;
}


short
EmdValidator::checkPreValidation() const
{

  if(isDdInfo())
    _diag875->printDetailInterlineEmdProcessingS5Info(_ocEmdDataProvider.nation(),
                                                      _ocEmdDataProvider.gds(),
                                                      _ocEmdDataProvider.emdValidatingCarrier(),
                                                      _ocEmdDataProvider.marketingCarriers(),
                                                      _ocEmdDataProvider.operatingCarriers());

  if( _ocEmdDataProvider.operatingCarriers().empty() ||
      _ocEmdDataProvider.marketingCarriers().empty() ||
      _ocEmdDataProvider.emdValidatingCarrier().empty() )
  {
    if(isDdInfo())
      _diag875->printDetailInterlineEmdProcessingStatusS5Info(false);
    return 0;
  }

  if( _ocEmdDataProvider.operatingCarriers().size() == 1 && _ocEmdDataProvider.marketingCarriers().size() == 1
    && ( *_ocEmdDataProvider.operatingCarriers().begin() == *_ocEmdDataProvider.marketingCarriers().begin()) )
  {
    if( *_ocEmdDataProvider.operatingCarriers().begin() == _ocEmdDataProvider.emdValidatingCarrier() )
    {
      if(isDdInfo())
        _diag875->printDetailInterlineEmdProcessingStatusS5Info(true);
      return 1;
    }
  }

  return -1;
}

bool
EmdValidator::getRecordsFromDatabase(std::vector<EmdInterlineAgreementInfo*>& eiaList,
                                     std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >& carrierEmdInfoMap,
                                     boost::mutex& mutex) const
{
  std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >::const_iterator carrierEmdInfoMapI =
    carrierEmdInfoMap.find(_ocEmdDataProvider.emdValidatingCarrier());
  if (carrierEmdInfoMapI != carrierEmdInfoMap.end())
  {
    eiaList = (*carrierEmdInfoMapI).second;
    return !eiaList.empty();
  }

  eiaList.reserve(500);
  const std::vector<EmdInterlineAgreementInfo*>& allCountriesEiaList =
    _trx.dataHandle().getEmdInterlineAgreements(EmdValidator::ALL_COUNTRIES, _ocEmdDataProvider.gds(), _ocEmdDataProvider.emdValidatingCarrier());
  if(!allCountriesEiaList.empty())
    eiaList.insert(eiaList.end(), allCountriesEiaList.begin(), allCountriesEiaList.end());
  const std::vector<EmdInterlineAgreementInfo*>& specificCountryEiaList =
    _trx.dataHandle().getEmdInterlineAgreements(_ocEmdDataProvider.nation(), _ocEmdDataProvider.gds(), _ocEmdDataProvider.emdValidatingCarrier());

  if(!specificCountryEiaList.empty())
    eiaList.insert(eiaList.end(), specificCountryEiaList.begin(), specificCountryEiaList.end());

  const boost::lock_guard<boost::mutex> guard(mutex);
  carrierEmdInfoMap.insert(std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> > ::value_type(
    _ocEmdDataProvider.emdValidatingCarrier(), eiaList));

  if(eiaList.empty())
    return false;

  return true;

}


bool
EmdValidator::checkRegularEmdAgreement(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const
{
  std::set<CarrierCode> emdInfoParticipatingCarriers;
  if(!_ocEmdDataProvider.emdValidatingCarrier().empty())
    emdInfoParticipatingCarriers.insert(_ocEmdDataProvider.emdValidatingCarrier());
  std::transform(eiaList.begin(), eiaList.end(), std::inserter(emdInfoParticipatingCarriers, emdInfoParticipatingCarriers.begin()),
                 boost::bind(&EmdInterlineAgreementInfo::getParticipatingCarrier, _1));

  return checkEmdAgreement(_ocEmdDataProvider.marketingCarriers(), _ocEmdDataProvider.operatingCarriers(), emdInfoParticipatingCarriers);
}

bool
EmdValidator::checkEmdInterlineAgrrementMsc(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const
{
  return (_ocEmdDataProvider.getEmdMostSignificantCarrier() == _ocEmdDataProvider.emdValidatingCarrier()) ||
          std::any_of(eiaList.begin(), eiaList.end(),
                     [&](const EmdInterlineAgreementInfo* eia)
                     {
                       return eia->getParticipatingCarrier() == _ocEmdDataProvider.getEmdMostSignificantCarrier();
                     });
}

bool
EmdValidator::isDdInfo() const
{
  return _diag875 && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

bool
EmdValidator::isAvEMDIA() const
{
  return isDdInfo() &&  _trx.diagnostic().diagParamMapItem(Diagnostic::ALL_VALID) == "EMDIA";
}

bool
EmdValidator::checkEmdAgreement(const std::set<CarrierCode>& marketingCarriers,
                                const std::set<CarrierCode>& operatingCarriers,
                                const std::set<CarrierCode>& emdInfoParticipatingCarriers) const
{
  std::set<CarrierCode> unMatchedParticipatingCarriers;
  std::set_difference(marketingCarriers.begin(), marketingCarriers.end(),
                      emdInfoParticipatingCarriers.begin(), emdInfoParticipatingCarriers.end(),
                      std::inserter(unMatchedParticipatingCarriers, unMatchedParticipatingCarriers.begin()));

  if(unMatchedParticipatingCarriers.empty())
  {
    std::set_difference(operatingCarriers.begin(), operatingCarriers.end(),
                        emdInfoParticipatingCarriers.begin(), emdInfoParticipatingCarriers.end(),
                        std::inserter(unMatchedParticipatingCarriers, unMatchedParticipatingCarriers.begin()));
  }
  if(isDdInfo())
    _diag875->printDetailInterlineEmdProcessingStatusS5Info(unMatchedParticipatingCarriers.empty(), unMatchedParticipatingCarriers);

  return unMatchedParticipatingCarriers.empty();
}

} // tse namespace
