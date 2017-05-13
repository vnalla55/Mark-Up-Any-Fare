
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

#include "Taxes/Pfc/PfcDisplayDb.h"
#include <algorithm>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::PfcDisplayDb
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDb::PfcDisplayDb(TaxTrx* trx)
  : _trx(trx),
    _date(DateTime::localTime()),
    _localDate(DateTime::localTime()),
    _sellingDate(false),
    _lazyAcquiredPfcPFC(nullptr),
    _lazyAcquiredDateIndependentPfcPFC(nullptr),
    _lazyAcquiredPfcAbsorb(nullptr),
    _lazyAcquiredPfcEssAirSvc(nullptr),
    _lazyAcquiredPfcMultiAirport(nullptr),
    _lazyAcquiredPfcEquipExempt(nullptr),
    _lazyAcquiredPfcCollectMethData(nullptr)
{

  if (this->trx()->pfcDisplayRequest()->overrideDate().isValid())
  {
    setDate(this->trx()->pfcDisplayRequest()->overrideDate());
    this->trx()->dataHandle().setTicketDate(date());
  }
  else
  {
    if (this->trx()->pfcDisplayRequest()->cmdType() == PfcDisplayRequest::PXC)
    {
      _sellingDate = true;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::isValidDate()
//
// Description:  Returns true if date is valid.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
PfcDisplayDb::isValidDate() const
{
  DateTime tempDate = date();
  tempDate = tempDate.addYears(2);

  if (tempDate.date() >= localDate().date())
  {
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::isSellingDate()
//
// Description:  Returns true if is selling date.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
PfcDisplayDb::isSellingDate() const
{
  return _sellingDate;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::oldestAllowedDate()
//
// Description:  Returns oldest historical allowed date.
//
// </PRE>
// ----------------------------------------------------------------------------
DateTime
PfcDisplayDb::oldestAllowedDate() const
{
  uint32_t y = localDate().year() - 2;
  uint32_t m = localDate().month();
  uint32_t d = localDate().day();

  return DateTime(y, m, d);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::~PfcDisplayDb
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDb::~PfcDisplayDb() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getTaxCode
//
// Description:  Get Tax records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<TaxCodeReg*>&
PfcDisplayDb::getTaxCode(const TaxCode& taxCode) const
{
  return trx()->dataHandle().getTaxCode(taxCode, date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcPFC
//
// Description:  Get Customer records.
//
// </PRE>
// ----------------------------------------------------------------------------
const Customer*
PfcDisplayDb::getCustomer(const PseudoCityCode& key) const
{
  const std::vector<Customer*>& customerV = trx()->dataHandle().getCustomer(key);

  if (customerV.empty())
  {
    return nullptr;
  }

  return customerV.front();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getCurrency
//
// Description:  Get nation currency.
//
// </PRE>
// ----------------------------------------------------------------------------
const Nation*
PfcDisplayDb::getNation(const NationCode& nationCode) const
{
  return trx()->dataHandle().getNation(nationCode, date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getLoc
//
// Description:  Get Location record.
//
// </PRE>
// ----------------------------------------------------------------------------
const Loc*
PfcDisplayDb::getLoc(const LocCode& locCode) const
{
  return trx()->dataHandle().getLoc(locCode, date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcMultiAirport
//
// Description:  Get airports for the city record.
//
// </PRE>
// ----------------------------------------------------------------------------
const PfcMultiAirport*
PfcDisplayDb::getPfcMultiAirport(const LocCode& key) const
{
  return trx()->dataHandle().getPfcMultiAirport(key, date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcMultiAirport
//
// Description:  Get PFC MultiAirport records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcMultiAirport*>&
PfcDisplayDb::getAllPfcMultiAirport() const
{
  if (!_lazyAcquiredPfcMultiAirport)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcMultiAirport);

    const std::vector<PfcMultiAirport*>& rawPfcMultiAirportV =
        this->trx()->dataHandle().getAllPfcMultiAirport();

    std::remove_copy_if(rawPfcMultiAirportV.begin(),
                        rawPfcMultiAirportV.end(),
                        back_inserter(*_lazyAcquiredPfcMultiAirport),
                        std::not1(IsValidDate<PfcMultiAirport>(date(), isSellingDate())));

    std::sort(_lazyAcquiredPfcMultiAirport->begin(),
              _lazyAcquiredPfcMultiAirport->end(),
              std::not2(PfcMultiArptGreater()));
  }

  /* if(!_lazyAcquiredPfcMultiAirport)
   {
     trx()->dataHandle().get(_lazyAcquiredPfcMultiAirport);
   }*/

  return *_lazyAcquiredPfcMultiAirport;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcPFC
//
// Description:  Get PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDb::getPfcPFC(const LocCode& loc) const
{
  if (!_lazyAcquiredPfcPFC)
  {
    getAllPfcPFC();
  }

  std::vector<PfcPFC*>* pfcV;
  trx()->dataHandle().get(pfcV);

  std::remove_copy_if(_lazyAcquiredPfcPFC->begin(),
                      _lazyAcquiredPfcPFC->end(),
                      back_inserter(*pfcV),
                      std::not1(IsValidLocCarrier<PfcPFC>(loc, &PfcPFC::pfcAirport)));

  return *pfcV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getAllPfcPFC
//
// Description:  Get all PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDb::getAllPfcPFC() const
{
  if (!_lazyAcquiredPfcPFC)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcPFC);

    const std::vector<PfcPFC*>& rawPfcV = this->trx()->dataHandle().getAllPfcPFC();

    std::remove_copy_if(rawPfcV.begin(),
                        rawPfcV.end(),
                        back_inserter(*_lazyAcquiredPfcPFC),
                        std::not1(IsValidDate<PfcPFC>(date(), isSellingDate())));
  }

  std::sort(_lazyAcquiredPfcPFC->begin(),
            _lazyAcquiredPfcPFC->end(),
            std::not2(Greater<PfcPFC, LocCode>(&PfcPFC::pfcAirport)));

  return *_lazyAcquiredPfcPFC;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getDateIndependentPfcPFC
//
// Description:  Get PFC date independent records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDb::getDateIndependentPfcPFC(const LocCode& loc) const
{
  std::vector<PfcPFC*>* pfcV;
  trx()->dataHandle().get(pfcV);

  const std::vector<PfcPFC*>& rawPfcV = this->trx()->dataHandle().getAllPfcPFC();

  std::remove_copy_if(rawPfcV.begin(),
                      rawPfcV.end(),
                      back_inserter(*pfcV),
                      std::not1(IsValidLocCarrier<PfcPFC>(loc, &PfcPFC::pfcAirport)));

  std::sort(pfcV->begin(),
            pfcV->end(),
            Greater<PfcPFC, DateTime, DateTime, DateTime>(
                &PfcPFC::effDate, &PfcPFC::createDate, &PfcPFC::expireDate));

  return *pfcV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcAbsorb
//
// Description:  Get PFC Absorption records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcAbsorb*>&
PfcDisplayDb::getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier) const
{
  if (!_lazyAcquiredPfcAbsorb)
  {
    getAllPfcAbsorb();
  }

  std::vector<PfcAbsorb*>* pfcAbsorbV;
  trx()->dataHandle().get(pfcAbsorbV);

  std::remove_copy_if(
      _lazyAcquiredPfcAbsorb->begin(),
      _lazyAcquiredPfcAbsorb->end(),
      back_inserter(*pfcAbsorbV),
      std::not1(IsValidLocCarrier<PfcAbsorb>(
          pfcAirport, localCarrier, &PfcAbsorb::pfcAirport, &PfcAbsorb::localCarrier)));

  return *pfcAbsorbV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getAllPfcAbsorb
//
// Description:  Get PFC Absorption records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcAbsorb*>&
PfcDisplayDb::getAllPfcAbsorb() const
{

  if (!_lazyAcquiredPfcAbsorb)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcAbsorb);

    const std::vector<PfcAbsorb*>& rawAbsorbV = this->trx()->dataHandle().getAllPfcAbsorb();

    std::remove_copy_if(rawAbsorbV.begin(),
                        rawAbsorbV.end(),
                        back_inserter(*_lazyAcquiredPfcAbsorb),
                        std::not1(IsValidDate<PfcAbsorb>(date(), isSellingDate())));

    std::sort(_lazyAcquiredPfcAbsorb->begin(),
              _lazyAcquiredPfcAbsorb->end(),
              std::not2(Greater<PfcAbsorb, LocCode, CarrierCode, int>(
                  &PfcAbsorb::pfcAirport, &PfcAbsorb::localCarrier, &PfcAbsorb::seqNo)));
  }

  return *_lazyAcquiredPfcAbsorb;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getPfcEssAirSvc
//
// Description:  Get PFC Essential Air Service records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcEssAirSvc*>&
PfcDisplayDb::getPfcEssAirSvc(const LocCode& easHubArpt) const
{
  if (!_lazyAcquiredPfcEssAirSvc)
  {
    getAllPfcEssAirSvc();
  }

  std::vector<PfcEssAirSvc*>* pfcEssAirSvcV;
  trx()->dataHandle().get(pfcEssAirSvcV);

  std::remove_copy_if(
      _lazyAcquiredPfcEssAirSvc->begin(),
      _lazyAcquiredPfcEssAirSvc->end(),
      back_inserter(*pfcEssAirSvcV),
      std::not1(IsValidLocCarrier<PfcEssAirSvc>(easHubArpt, &PfcEssAirSvc::easHubArpt)));

  return *pfcEssAirSvcV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getAllPfcEssAirSvc
//
// Description:  Get Essential Air Service records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcEssAirSvc*>&
PfcDisplayDb::getAllPfcEssAirSvc() const
{
  if (!_lazyAcquiredPfcEssAirSvc)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcEssAirSvc);
    const std::vector<PfcEssAirSvc*>& rawPfcEssAirSvcV =
        this->trx()->dataHandle().getAllPfcEssAirSvc();

    std::remove_copy_if(rawPfcEssAirSvcV.begin(),
                        rawPfcEssAirSvcV.end(),
                        back_inserter(*_lazyAcquiredPfcEssAirSvc),
                        std::not1(IsValidDate<PfcEssAirSvc>(date(), isSellingDate())));

    std::sort(_lazyAcquiredPfcEssAirSvc->begin(),
              _lazyAcquiredPfcEssAirSvc->end(),
              std::not2(Greater<PfcEssAirSvc, LocCode, LocCode>(&PfcEssAirSvc::easHubArpt,
                                                                &PfcEssAirSvc::easArpt)));
  }

  return *_lazyAcquiredPfcEssAirSvc;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDb::getCxrPfcAbsorb
//
// Description:  Get PFC Absorption records for airport.
//
// </PRE>
// ----------------------------------------------------------------------------

const std::vector<PfcEquipTypeExempt*>&
PfcDisplayDb::getAllPfcEquipTypeExempt() const
{
  if (!_lazyAcquiredPfcEquipExempt)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcEquipExempt);
    const std::vector<PfcEquipTypeExempt*>& rawPfcEquipTypeExempt =
        this->trx()->dataHandle().getAllPfcEquipTypeExemptData();

    std::remove_copy_if(rawPfcEquipTypeExempt.begin(),
                        rawPfcEquipTypeExempt.end(),
                        back_inserter(*_lazyAcquiredPfcEquipExempt),
                        std::not1(IsValidDate<PfcEquipTypeExempt>(date(), isSellingDate())));
  }
  return *_lazyAcquiredPfcEquipExempt;
}

// ---------------------------------------------------------------------------
// <PRE>
//
// @function
//
// Description:  Get PFC .
//
// </PRE>
// ----------------------------------------------------------------------------

const std::vector<PfcCollectMeth*>&
PfcDisplayDb::getAllPfcCollectMethData() const
{
  // return trx()->dataHandle().getAllPfcCollectMeth(date());

  if (!_lazyAcquiredPfcCollectMethData)
  {
    trx()->dataHandle().get(_lazyAcquiredPfcCollectMethData);
    const std::vector<PfcCollectMeth*>& rawPfcCollectMethData =
        this->trx()->dataHandle().getAllPfcCollectMeth(date());

    std::remove_copy_if(rawPfcCollectMethData.begin(),
                        rawPfcCollectMethData.end(),
                        back_inserter(*_lazyAcquiredPfcCollectMethData),
                        std::not1(IsValidDate<PfcCollectMeth>(date(), isSellingDate())));
  }
  return *_lazyAcquiredPfcCollectMethData;
}

//---

const std::vector<PfcCollectMeth*>&
PfcDisplayDb::getPfcCollectMethData(const CarrierCode& carrier) const
{

  return trx()->dataHandle().getPfcCollectMeth(carrier, date());
}

//---
