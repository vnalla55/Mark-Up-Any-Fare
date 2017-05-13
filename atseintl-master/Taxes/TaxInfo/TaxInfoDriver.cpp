
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

#include "Taxes/TaxInfo/TaxInfoDriver.h"
#include "Taxes/TaxInfo/TaxInfoBuilderMisc.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"
#include "DataModel/TaxInfoResponse.h"

#include <boost/bind.hpp>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderFactory::getInstance
//
// Description:  Get TaxInfo instance
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilder*
TaxInfoBuilderFactory::getInstance(TaxTrx& trx, const TaxCode& taxCode)
{
  TaxInfoBuilder* ans;

  if (taxCode == TaxInfoBuilderPFC::TAX_CODE)
  {
    ans = trx.dataHandle().create<TaxInfoBuilderPFC>();
  }
  else if (taxCode == TaxInfoBuilderZP::TAX_CODE)
  {
    ans = trx.dataHandle().create<TaxInfoBuilderZP>();
  }
  else
  {
    ans = trx.dataHandle().create<TaxInfoBuilderMisc>();
    ans->taxCode() = taxCode;
  }

  return ans;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoDriver::TaxInfoDriver
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoDriver::TaxInfoDriver(TaxTrx* trx) : _trx(trx)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function: TaxInfoDriver::~TaxInfoDriver
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoDriver::~TaxInfoDriver()
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoDriver::TaxInfoDriver
//
// Description: Build TaxInfo response for each TaxInfoItems.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoDriver::buildTaxInfoResponse()
{
  TaxInfoRequest* req = trx()->taxInfoRequest();

  if (req->taxItems().empty())
  {
    TaxInfoItem dummyItem;
    build(dummyItem);
  }
  else
  {
    std::for_each(req->taxItems().begin(),
                  req->taxItems().end(),
                  boost::bind(&TaxInfoDriver::build, this, _1));
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoDriver::build
//
// Description: Build TaxInfo response for TaxInfoItem.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoDriver::build(const TaxInfoItem& item)
{
  TaxInfoBuilder* builder;

  builder = TaxInfoBuilderFactory::getInstance(*trx(), item.taxCode());
  builder->build(*trx());

  trx()->taxInfoResponseItems().push_back(builder->response());
}
