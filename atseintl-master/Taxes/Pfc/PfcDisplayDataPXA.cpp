
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayDataPXA.h"
#include <algorithm>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXA::PfcDisplayDataPXA
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXA::PfcDisplayDataPXA(TaxTrx* trx, PfcDisplayDb* db)
  : PfcDisplayData(trx, db),
    _pfcAbsorbV(nullptr),
    _carrier(this->trx()->pfcDisplayRequest()->carrier1())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDataPXC::~PfcDisplayDataPXA
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDataPXA::~PfcDisplayDataPXA() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::getPfcAbsorb
//
// Description:  Get PFC Absorption records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcAbsorb*>&
PfcDisplayDataPXA::getPfcAbsorb() const
{
  if (_pfcAbsorbV != nullptr)
  {
    return *_pfcAbsorbV;
  }

  trx()->dataHandle().get(_pfcAbsorbV);

  const std::vector<PfcAbsorb*>& allPfcAbsorbV = db()->getAllPfcAbsorb();

  ///////////////////
  // PXA* entry
  ///////////////////

  if (trx()->pfcDisplayRequest()->segments().empty() &&
      trx()->pfcDisplayRequest()->absorptionRecordNumber() == 0)
  {
    return allPfcAbsorbV;
  }

  ///////////////////
  // PXA*NBR entry
  ///////////////////

  PfcDisplayRequest::Segments::const_iterator itS = trx()->pfcDisplayRequest()->segments().begin();
  PfcDisplayRequest::Segments::const_iterator itSEnd = trx()->pfcDisplayRequest()->segments().end();

  std::vector<PfcAbsorb*>::const_iterator it;
  std::vector<PfcAbsorb*>::const_iterator itEnd;

  if (trx()->pfcDisplayRequest()->absorptionRecordNumber() != 0)
  {
    LocCode arpt;
    CarrierCode carrier;
    std::string effDate;
    std::string discDate;

    uint32_t nbr = 1;

    it = allPfcAbsorbV.begin();
    itEnd = allPfcAbsorbV.end();

    for (; it < itEnd; it++)
    {
      if (ifNotEqualitySet((*it)->pfcAirport(),
                           (*it)->localCarrier(),
                           (*it)->effDate().dateToString(tse::DDMMMYY, nullptr),
                           (*it)->expireDate().dateToString(tse::DDMMMYY, nullptr),
                           arpt,
                           carrier,
                           effDate,
                           discDate))
      {
        if (nbr == trx()->pfcDisplayRequest()->absorptionRecordNumber())
        {
          it = allPfcAbsorbV.begin();
          itEnd = allPfcAbsorbV.end();

          for (; it < itEnd; it++)
          {
            if ((*it)->pfcAirport() == arpt && (*it)->localCarrier() == carrier &&
                (*it)->effDate().dateToString(tse::DDMMMYY, nullptr) == effDate &&
                (*it)->expireDate().dateToString(tse::DDMMMYY, nullptr) == discDate)
            {
              _pfcAbsorbV->push_back(*it);
            }
          }
          break;
        }

        nbr++;
      }
    }

    std::sort(_pfcAbsorbV->begin(),
              _pfcAbsorbV->end(),
              std::not2(PfcDisplayDb::Greater<PfcAbsorb, int>(&PfcAbsorb::seqNo)));

    return *_pfcAbsorbV;
  }

  ////////////////////////
  // PXA*I entry
  ////////////////////////
  if (isPNR())
  {
    CarrierCode carrier;
    LocCode airport;

    size_t sizeBefore;
    size_t sizeAfter;

    for (; itS < itSEnd; itS++)
    {
      carrier = std::get<PfcDisplayRequest::CARRIER_CODE>(*itS);
      airport = std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(*itS);

      sizeBefore = _pfcAbsorbV->size();
      std::remove_copy_if(allPfcAbsorbV.begin(),
                          allPfcAbsorbV.end(),
                          back_inserter(*_pfcAbsorbV),
                          std::not1(PfcDisplayDb::IsValidLocCarrier<PfcAbsorb>(
                              airport, carrier, &PfcAbsorb::pfcAirport, &PfcAbsorb::localCarrier)));
      sizeAfter = _pfcAbsorbV->size();

      if (sizeBefore == sizeAfter)
      {
        PfcAbsorb* pfcAbsorb;
        trx()->dataHandle().get(pfcAbsorb);

        pfcAbsorb->pfcAirport() = airport;
        pfcAbsorb->localCarrier() = carrier;
        _pfcAbsorbV->push_back(pfcAbsorb);
      }
    }

    /* std::sort(_pfcAbsorbV->begin(),
               _pfcAbsorbV->end(),
               std::not2(PfcDisplayDb::Greater<PfcAbsorb, LocCode,
       CarrierCode>(&PfcAbsorb::pfcAirport,
                                                                                &PfcAbsorb::localCarrier)));
       */

    return *_pfcAbsorbV;
  }

  //////////////////////////////////////////////////////////////////////////////
  // PXA*AAA, PXA*AAA/CC, PXA*AAA/CC/DD, PXA*AAA/CC-.PCT entries
  //////////////////////////////////////////////////////////////////////////////

  CarrierCode wantedCarrier;
  LocCode airport;
  for (; itS < itSEnd; itS++)
  {
    wantedCarrier = std::get<PfcDisplayRequest::CARRIER_CODE>(*itS);
    airport = std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(*itS);

    it = allPfcAbsorbV.begin();
    itEnd = allPfcAbsorbV.end();
    for (; it < itEnd; it++)
    {
      if (airport == (*it)->pfcAirport())
      {
        if (((!wantedCarrier.empty() && wantedCarrier == (*it)->localCarrier()) ||
             (!carrier().empty() && carrier() == (*it)->localCarrier())) ||
            (wantedCarrier.empty() && carrier().empty()))
        {
          _pfcAbsorbV->push_back(*it);
        }
      }
    }
  }

  std::sort(_pfcAbsorbV->begin(),
            _pfcAbsorbV->end(),
            std::not2(PfcDisplayDb::Greater<PfcAbsorb, CarrierCode, int>(&PfcAbsorb::localCarrier,
                                                                         &PfcAbsorb::seqNo)));

  return *_pfcAbsorbV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::getFareAmt
//
// Description:  Get fare amount.
//
// </PRE>
// ----------------------------------------------------------------------------
const MoneyAmount
PfcDisplayDataPXA::getFareAmt(LocCode key) const
{
  const std::vector<TaxCodeReg*>& us1 = db()->getTaxCode("US1");

  if (!us1.size())
  {
    return 0.0;
  }

  MoneyAmount percentage;

  if (trx()->pfcDisplayRequest()->percentageRate() != 0.0)
  {
    percentage = trx()->pfcDisplayRequest()->percentageRate() / 10000;
  }
  else
  {
    percentage = us1.front()->taxAmt();
  }

  std::vector<PfcPFC*> pfcV = db()->getPfcPFC(key);

  if (pfcV.empty())
  {
    return 0.0;
  }

  return pfcV.front()->pfcAmt1() / (1 + percentage);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::getTaxAmt
//
// Description:  Get tax amount.
//
// </PRE>
// ----------------------------------------------------------------------------
const MoneyAmount
PfcDisplayDataPXA::getTaxAmt(LocCode key) const
{
  std::vector<PfcPFC*> pfcV = db()->getPfcPFC(key);

  if (pfcV.empty())
  {
    return 0.0;
  }

  return pfcV.front()->pfcAmt1() - getFareAmt(key);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderPXA::setCarrier
//
// Description:  Set current carrrier.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayDataPXA::setCarrier(CarrierId id)
{
  if (id == FIRST_CARRIER)
  {
    carrier() = trx()->pfcDisplayRequest()->carrier1();
  }
  else // SECOND_CARRIER
  {
    carrier() = trx()->pfcDisplayRequest()->carrier2();
  }
}
