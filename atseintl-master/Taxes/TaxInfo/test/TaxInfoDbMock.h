//----------------------------------------------------------------------------
//  File:           TaxInfoDbMock.h
//  Authors:        Piotr Lach
//  Created:        2/10/2009
//  Description:    DB mock for Tax Info functionality.
//
//  Copyright Sabre 2009
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
#ifndef TAX_INFO_DB_MOCK_H
#define TAX_INFO_DB_MOCK_H

#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "Taxes/TaxInfo/TaxInfoBuilderMisc.h"

namespace tse
{

class TaxInfoBuilderPFCMock : public TaxInfoBuilderPFC
{
protected:
  const std::vector<PfcPFC*>& getPfc(TaxTrx& trx, const LocCode& airport)
  {

    std::vector<PfcPFC*>* pfcAirport;
    trx.dataHandle().get(pfcAirport);

    if (airport == "DFW")
    {
      PfcPFC* item = new PfcPFC;

      item->pfcAmt1() = 4.50;
      item->effDate() = boost::posix_time::time_from_string("2002-07-01 21:59:59.000");
      item->discDate() = boost::posix_time::time_from_string("2017-03-01 21:59:59.000");
      item->pfcCur1() = "USD";

      pfcAirport->push_back(item);
    }
    else if (airport == "JFK")
    {
      PfcPFC* item = new PfcPFC;

      item->pfcAmt1() = 3.50;
      item->effDate() = boost::posix_time::time_from_string("2003-07-01 21:59:59.000");
      item->discDate() = boost::posix_time::time_from_string("2010-03-01 21:59:59.000");
      item->pfcCur1() = "USD";

      pfcAirport->push_back(item);
    }

    return *pfcAirport;
  }

  const Loc* getLoc(TaxTrx& trx, const LocCode& locCode)
  {
    Loc* loc = new Loc;

    if (locCode == "DFW" || locCode == "KRK" || locCode == "JFK")
      loc->transtype() = 'P';
    else
      loc->transtype() = ' ';

    return loc;
  }
};

class TaxInfoBuilderZPMock : public TaxInfoBuilderZP
{

protected:
  const Loc* getLoc(TaxTrx& trx, const LocCode& locCode)
  {
    Loc* loc = new Loc;

    if (locCode == "DFW" || locCode == "KRK" || locCode == "JFK")
      loc->transtype() = 'P';
    else
      loc->transtype() = ' ';

    return loc;
  }

  const std::vector<TaxCodeReg*>& getTaxCodeReg(TaxTrx& trx)
  {
    std::vector<TaxCodeReg*>* taxCodeReg;
    trx.dataHandle().get(taxCodeReg);

    TaxCodeReg* item = new TaxCodeReg;

    item->taxCode() = "ZP";
    item->effDate() = boost::posix_time::time_from_string("2002-07-01 21:59:59.000");
    item->discDate() = boost::posix_time::time_from_string("2017-03-01 21:59:59.000");
    item->taxType() = 'F';
    item->nation() = "US";
    item->taxAmt() = 3.60;
    item->taxCur() = "USD";

    TaxCodeGenText* taxCodeGenText = new TaxCodeGenText;
    taxCodeGenText->txtMsgs().push_back("SEGMENT TAX");

    item->taxCodeGenTexts().push_back(taxCodeGenText);

    taxCodeReg->push_back(item);

    return *taxCodeReg;
  }
};

class TaxInfoBuilderMiscMock : public TaxInfoBuilderMisc
{
public:
  TaxCode& reqTaxCode() { return _reqTaxCode; }

protected:
  const Loc* getLoc(TaxTrx& trx, const LocCode& locCode)
  {
    Loc* loc = new Loc;

    if (locCode == "DFW" || locCode == "KRK" || locCode == "JFK" || locCode == "YYC")
    {
      loc->transtype() = 'P';
      loc->loc() = locCode;
    }
    else
      loc->transtype() = ' ';

    return loc;
  }

  const std::vector<TaxCodeReg*>& getTaxCodeReg(TaxTrx& trx)
  {

    std::vector<TaxCodeReg*>* taxCodeReg;
    trx.dataHandle().get(taxCodeReg);

    if (_reqTaxCode != "")
    {
      TaxCodeReg* item = new TaxCodeReg;
      TaxCodeGenText* taxCodeGenText = new TaxCodeGenText;

      if (_reqTaxCode == "US1")
      {
        item->taxCode() = "US1";
        item->effDate() = boost::posix_time::time_from_string("2002-07-01 21:59:59.000");
        item->discDate() = boost::posix_time::time_from_string("2017-03-01 21:59:59.000");
        item->taxType() = 'P';
        item->nation() = "US";
        item->taxAmt() = 0.075;
        item->taxCur() = "USD";
        item->loc1ExclInd() = 'N';
        item->loc1Type() = ' ';
        taxCodeGenText->txtMsgs().push_back("US DOMESTIC TRANSPORTATION TAX");
      }
      else if (_reqTaxCode == "CA1")
      {
        item->taxCode() = "CA1";
        item->effDate() = boost::posix_time::time_from_string("2002-07-01 21:59:59.000");
        item->discDate() = boost::posix_time::time_from_string("2017-03-01 21:59:59.000");
        item->taxType() = 'F';
        item->nation() = "CA";
        item->taxAmt() = 4.67;
        item->taxCur() = "CAD";
        item->loc1ExclInd() = 'N';
        item->loc1Type() = ' ';
        taxCodeGenText->txtMsgs().push_back("CANADA AIR SECURITY CHARGE - SUBJECT TO GST");
      }
      else
      {
        return *taxCodeReg;
      }

      item->taxCodeGenTexts().push_back(taxCodeGenText);

      taxCodeReg->push_back(item);
    }

    return *taxCodeReg;
  }

  const std::vector<Customer*>& getCustomer(TaxTrx& trx, const PseudoCityCode& pcc)
  {

    std::vector<Customer*>* customer;
    trx.dataHandle().get(customer);

    Customer* item = new Customer;

    if (pcc == "03KA")
      item->aaCity() = "YYC";
    if (pcc == "01J7")
      item->aaCity() = "DFW";

    customer->push_back(item);

    return *customer;
  }

  TaxCode _reqTaxCode;
};

} // namespace tse
#endif
