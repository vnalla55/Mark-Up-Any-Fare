// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "TaxDisplay/TaxDisplay.h"

#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Entry.h"
#include "TaxDisplay/EntryHelp.h"
#include "TaxDisplay/EntryNationReportingRecord.h"
#include "TaxDisplay/EntryNationRulesRecord.h"
#include "TaxDisplay/EntryTaxReportingRecord.h"
#include "TaxDisplay/EntryTaxRulesRecord.h"

namespace tax
{
namespace display
{

TaxDisplay::TaxDisplay(const TaxDisplayRequest& request,
                       Services& services) :
                           _entry(nullptr),
                           _request(request),
                           _services(services)
{
}

TaxDisplay::~TaxDisplay()
{
  if(_entry)
    delete _entry;
}


bool TaxDisplay::buildResponse(std::ostringstream& response)
{
  setEntry();
  buildEntry();

  _formatter.format(response);
  return true;
}

void TaxDisplay::buildEntry()
{
  if(_entry)
  {
    _entry->buildHeader() && _entry->buildBody() && _entry->buildFooter();
  }
  else
  {
    _formatter.addLine("BAD REQUEST");
  }
}

void TaxDisplay::setEntry()
{
  switch(_request.entryType)
  {
  case TaxDisplayRequest::EntryType::ENTRY_HELP:
  case TaxDisplayRequest::EntryType::ENTRY_HELP_CALCULATION_BREAKDOWN:
    _entry = new EntryHelp(_request, _formatter);
    break;
  case TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION:
    _entry = new EntryNationReportingRecord(_formatter, _services, _request);
    break;
  case TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_NATION:
    _entry = new EntryNationRulesRecord(_formatter, _services, _request);
    break;
  case TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_TAX:
    _entry = new EntryTaxReportingRecord(_formatter, _services, _request);
    break;
  case TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_TAX:
    _entry = new EntryTaxRulesRecord(_formatter, _services, _request);
    break;
  default:
    _entry = nullptr;
  }
}

} /* namespace display */
} /* namespace tax */
