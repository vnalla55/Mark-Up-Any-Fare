//----------------------------------------------------------------------------
//  File:           PfcDisplayDbMock.h
//  Authors:        Piotr Lach
//  Created:        4/17/2008
//  Description:    PfcDisplayDbMock header file for ATSE V2 PFC Display Project.
//                  DB mock for PFC Display functionality.
//
//  Copyright Sabre 2008
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
#ifndef PFC_DISPLAY_DB_MOCK_H
#define PFC_DISPLAY_DB_MOCK_H

#include "Taxes/Pfc/PfcDisplayDb.h"
#include "Taxes/Pfc/PfcDisplayCurrencyFacade.h"

namespace tse
{

class PfcDisplayCurrencyFacadeMock : public PfcDisplayCurrencyFacade
{
public:
  PfcDisplayCurrencyFacadeMock(const TaxTrx* trx) : PfcDisplayCurrencyFacade(trx) {}
  virtual ~PfcDisplayCurrencyFacadeMock() {}

  virtual std::string getEquivalentAmount(const MoneyAmount amt,
                                          const std::string& targetCurrency,
                                          const DateTime& dateTime) const
  {
    return "9.69";
  }
};

class PfcDisplayDbMock : public PfcDisplayDb
{
public:
  PfcDisplayDbMock(TaxTrx* trx);
  virtual ~PfcDisplayDbMock();

  virtual const Customer* getCustomer(const PseudoCityCode& key) const;
  virtual const Loc* getLoc(const LocCode& locCode) const;
  virtual const std::vector<PfcPFC*>& getPfcPFC(const LocCode& key) const;

  virtual const PfcMultiAirport*
  getPfcMultiAirport(const LocCode& key, bool isCurrentDate = false) const;

  virtual const std::vector<PfcEssAirSvc*>& getPfcEssAirSvc(const LocCode& easHubArpt) const;

  virtual const std::vector<PfcAbsorb*>&
  getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier) const;

  virtual const std::vector<PfcPFC*>& getAllPfcPFC() const;
  // virtual const std::vector<PfcEssAirSvc*>& getAllPfcEssAirSvc() const;
  // virtual const std::vector<PfcAbsorb*>&    getAllPfcAbsorb() const;
};

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::PfcDisplayDbMock
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDbMock::PfcDisplayDbMock(TaxTrx* trx) : PfcDisplayDb(trx) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::~PfcDisplayDbMock
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayDbMock::~PfcDisplayDbMock() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getCustomer
//
// Description:  Get Customer records.
//
// </PRE>
// ----------------------------------------------------------------------------
const Customer*
PfcDisplayDbMock::getCustomer(const PseudoCityCode& key) const
{

  Customer* customer = new Customer;

  customer->pseudoCity() = "17R7";
  customer->homePseudoCity() = "17R7";
  customer->defaultCur() = "PLN";
  customer->crsCarrier() = "1S";
  customer->agencyName() = "MVD TRNG";
  customer->aaCity() = "KRK";

  return customer;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getCustomer
//
// Description:  Get Location record.
//
// </PRE>
// ----------------------------------------------------------------------------
const Loc*
PfcDisplayDbMock::getLoc(const LocCode& locCode) const
{

  Loc* loc = new Loc;

  loc->loc() = "FFW";

  return loc;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getPfcPFC
//
// Description:  Get PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDbMock::getPfcPFC(const LocCode& key) const
{
  PfcPFC* pfc = new PfcPFC;
  std::vector<PfcPFC*>* pfcV = new std::vector<PfcPFC*>;

  pfc->pfcAirport() = "ABI";
  pfc->effDate() = boost::posix_time::time_from_string("2008-01-20 23:59:59.000");
  DateTime now = DateTime::localTime();
  pfc->expireDate() = now.addDays(400);
  pfc->pfcAmt1() = 4.5;
  pfc->pfcAirTaxExcp() = YES;
  pfc->pfcCharterExcp() = YES;
  pfc->freqFlyerInd() = YES;

  pfcV->push_back(pfc);

  return *pfcV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getAllPfcPFC
//
// Description:  Get all PFC records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcPFC*>&
PfcDisplayDbMock::getAllPfcPFC() const
{
  std::vector<PfcPFC*>* pfcV = new std::vector<PfcPFC*>;

  DateTime now = DateTime::localTime();

  PfcPFC* pfc1 = new PfcPFC;
  PfcPFC* pfc2 = new PfcPFC;
  PfcPFC* pfc3 = new PfcPFC;

  pfc1->pfcAirport() = "ABI";
  pfc1->effDate() = boost::posix_time::time_from_string("2006-01-20 23:59:59.000");
  pfc1->discDate() = now.addDays(400);
  pfc1->pfcAmt1() = 4.5;
  pfc1->pfcAirTaxExcp() = YES;
  pfc1->pfcCharterExcp() = YES;
  pfc1->freqFlyerInd() = YES;

  pfc2->pfcAirport() = "ABQ";
  pfc2->effDate() = boost::posix_time::time_from_string("2006-07-20 23:59:59.000");
  pfc2->discDate() = now.addDays(1);
  pfc2->pfcAmt1() = 40.25;
  pfc2->pfcAirTaxExcp() = NO;
  pfc2->pfcCharterExcp() = NO;
  pfc2->freqFlyerInd() = NO;

  pfc3->pfcAirport() = "JFK";
  pfc3->effDate() = boost::posix_time::time_from_string("2007-01-20 23:59:59.000");
  pfc3->discDate() = now.addDays(10);
  pfc3->pfcAmt1() = 4.05;
  pfc3->pfcAirTaxExcp() = NO;
  pfc3->pfcCharterExcp() = YES;
  pfc3->freqFlyerInd() = YES;

  PfcCxrExcpt* cxrExcpt1 = new PfcCxrExcpt;
  PfcCxrExcpt* cxrExcpt2 = new PfcCxrExcpt;
  PfcCxrExcpt* cxrExcpt3 = new PfcCxrExcpt;
  PfcCxrExcpt* cxrExcpt4 = new PfcCxrExcpt;

  cxrExcpt1->flt1() = 500;
  cxrExcpt1->flt2() = 3300;
  cxrExcpt1->excpCarrier() = "CO";

  cxrExcpt2->flt1() = 50;
  cxrExcpt2->flt2() = 5300;
  cxrExcpt2->excpCarrier() = "EX";

  cxrExcpt3->flt1() = 1100;
  cxrExcpt3->flt2() = 2000;
  cxrExcpt3->excpCarrier() = "AA";

  cxrExcpt4->flt1() = 0;
  cxrExcpt4->flt2() = 0;
  cxrExcpt4->excpCarrier() = "AX";

  pfc1->cxrExcpts().push_back(cxrExcpt1);
  pfc3->cxrExcpts().push_back(cxrExcpt2);
  pfc3->cxrExcpts().push_back(cxrExcpt3);
  pfc3->cxrExcpts().push_back(cxrExcpt4);

  pfcV->push_back(pfc1);
  pfcV->push_back(pfc2);
  pfcV->push_back(pfc3);

  return *pfcV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getPfcMultiAirport
//
// Description:  Get airports for the city record.
//
// </PRE>
// ----------------------------------------------------------------------------
const PfcMultiAirport*
PfcDisplayDbMock::getPfcMultiAirport(const LocCode& key, bool isCurrentDate) const
{
  return trx()->dataHandle().getPfcMultiAirport(key, date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getPfcEssAirSvc
//
// Description:  Get PFC Essential Air Service records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcEssAirSvc*>&
PfcDisplayDbMock::getPfcEssAirSvc(const LocCode& easHubArpt) const
{
  return trx()->dataHandle().getPfcEssAirSvc(easHubArpt, "DFW", date());
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getPfcEssAirSvc
//
// Description:  Get PFC Absorption records.
//
// </PRE>
// ----------------------------------------------------------------------------
const std::vector<PfcAbsorb*>&
PfcDisplayDbMock::getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier) const
{
  std::vector<PfcAbsorb*>* absorbV = new std::vector<PfcAbsorb*>;

  DateTime now = DateTime::localTime();

  PfcAbsorb* absorb1 = new PfcAbsorb;

  absorb1->seqNo() = 0;
  absorb1->effDate() = boost::posix_time::time_from_string("2006-01-20 23:59:59.000");
  absorb1->discDate() = now.addDays(400);
  absorb1->geoAppl() = '1';
  absorb1->absorbType() = '1';
  absorb1->absorbCity1() = "DFW";
  absorb1->absorbCity2() = "NYC";

  absorbV->push_back(absorb1);

  return *absorbV;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayDbMock::getAllPfcAbsorb
//
// Description:  Get all PFC Absorb records.
//
// </PRE>
// ----------------------------------------------------------------------------
/*const std::vector<PfcAbsorb*>&
PfcDisplayDbMock::getAllPfcAbsorb() const
{
  std::vector<PfcAbsorb*>* absorbV = new std::vector<PfcAbsorb*>;

    DateTime now = DateTime::localTime();

    PfcAbsorb* absorb1  = new PfcAbsorb;
    PfcAbsorb* absorb2  = new PfcAbsorb;


    absorb1->seqNo() = 0;
    absorb1->effDate() = boost::posix_time::time_from_string("2006-01-20 23:59:59.000");
    absorb1->discDate() = now.addDays(400);
    absorb1->geoAppl() = '1';
    absorb1->absorbType() = '1';
    absorb1->absorbCity1()= "DFW";
    absorb1->absorbCity2() = "NYC";

    absorb2->seqNo() = 1;
    absorb2->effDate() = boost::posix_time::time_from_string("2006-07-20 23:59:59.000");
    absorb2->discDate() = now.addDays(1);
  absorb2->geoAppl() = '2';
  absorb2->absorbType() = '3';
    absorb2->absorbCity1() = "BOS";





    absorbV->push_back(absorb1);
    absorbV->push_back(absorb2);

    return *absorbV;
}*/

} // namespace tse
#endif
