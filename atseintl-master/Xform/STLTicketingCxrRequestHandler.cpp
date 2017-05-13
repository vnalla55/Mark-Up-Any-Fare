//-------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//-------------------------------------------------------------------
#include "Xform/STLTicketingCxrRequestHandler.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/Agent.h"
#include "DataModel/TicketingCxrRequest.h"
#include "DataModel/TicketingCxrTrx.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/STLTicketingCxrSchemaNames.h"

namespace tse
{
using namespace stlticketingcxr;

namespace
{
Logger
_logger("atseintl.Xform.STLTicketingCxrRequestHandler");

ILookupMap _elemLookupMapTktCxr, _attrLookupMapTktCxr;
bool
init(IXMLUtils::initLookupMaps(_STLTicketingCxrElementNames,
                               NUMBER_OF_ELEMENT_NAMES_TKT_CXR,
                               _elemLookupMapTktCxr,
                               _STLTicketingCxrAttributeNames,
                               NUMBER_OF_ATTRIBUTE_NAMES_TKT_CXR,
                               _attrLookupMapTktCxr));
}

STLTicketingCxrRequestHandler::STLTicketingCxrRequestHandler(Trx*& trx) : CommonRequestHandler(trx)
{
}

bool
STLTicketingCxrRequestHandler::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case VALIDATINGCXR_CHECK:
    onStartValidatingCxrCheck(attrs);
    break;
  case SETTLEMENTPLAN_CHECK:
    onStartSettlementPlanCheck(attrs);
    break;
  case POS:
    onStartPOS(attrs);
    break;
  case ACTUAL:
    onStartActual(attrs);
    break;
  case HOME:
    onStartHome(attrs);
    break;
  case SETTLEMENT_PLAN:
    onStartSettlementPlan(attrs);
    break;
  case VALIDATING_CXR:
    onStartValidatingCxr(attrs);
    break;
  case PARTICIPATING_CXR:
    onStartParticipatingCxr(attrs);
    break;
  case TICKET_TYPE:
    onStartTicketType(attrs);
    break;
  case TICKET_DATE:
    onStartTicketDate(attrs);
    break;
  case REQUESTED_DIAGNOSTIC:
    onStartRequestedDiagnostic(attrs);
    break;
  case BILLINGINFORMATION:
    {
      Billing* billing = static_cast<TicketingCxrTrx*>(_trx)->billing();
      if (billing)
        onStartValCxrBIL(attrs, *billing);
      else
        LOG4CXX_ERROR(_logger, "ERROR: billing is null");
      break;
    }
  default:
    LOG4CXX_DEBUG(_logger, "unprocessed element ID");
    break;
  }
  return true;
}

bool
STLTicketingCxrRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case VALIDATINGCXR_CHECK:
    onEndValidatingCxrCheck();
    break;
  case SETTLEMENTPLAN_CHECK:
    onEndSettlementPlanCheck();
    break;
  case POS:
    onEndPOS();
    break;
  case ACTUAL:
    onEndActual();
    break;
  case HOME:
    onEndHome();
    break;
  case PCC:
    onEndPcc();
    break;
  case SETTLEMENT_PLAN:
    onEndSettlementPlan();
    break;
  case VALIDATING_CXR:
    onEndValidatingCxr();
    break;
  case PARTICIPATING_CXR:
    onEndParticipatingCxr();
    break;
  case TICKET_TYPE:
    onEndTicketType();
    break;
  case TICKET_DATE:
    onEndTicketDate();
    break;
  case REQUESTED_DIAGNOSTIC:
    onEndRequestedDiagnostic();
    break;
  case BILLINGINFORMATION:
    {
      Billing* billing = static_cast<TicketingCxrTrx*>(_trx)->billing();
      if (billing)
      {
        onEndValCxrBIL(*billing,
            Billing::SVC_TICKETINGCXR,
            _trx->transactionId());
      }
      break;
    }
  default:
    LOG4CXX_DEBUG(_logger, "unprocessed element ID");
    break;
  }
  return true;
}

void
STLTicketingCxrRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
{
  createTransaction(dataHandle, content);
  IValueString attrValueArray[NUMBER_OF_ATTRIBUTE_NAMES_TKT_CXR];
  int attrRefArray[NUMBER_OF_ATTRIBUTE_NAMES_TKT_CXR];
  IXMLSchema schema(_elemLookupMapTktCxr,
                    _attrLookupMapTktCxr,
                    NUMBER_OF_ATTRIBUTE_NAMES_TKT_CXR,
                    attrValueArray,
                    attrRefArray,
                    true);

  const char* pChar(content.c_str());
  size_t length(content.length());
  size_t pos(content.find_first_of('<'));
  if (pos != std::string::npos)
  {
    pChar += pos;
    length -= pos;
  }

  IParser parser(pChar, length, *this, schema);
  parser.parse();
}

void
STLTicketingCxrRequestHandler::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = dataHandle.create<TicketingCxrTrx>();
  _trx->dataHandle().get(_tcsReq);
  static_cast<TicketingCxrTrx*>(_trx)->setRequest(_tcsReq);

  Billing* billing;
  _trx->dataHandle().get(billing);
  static_cast<TicketingCxrTrx*>(_trx)->billing() = billing;

  _trx->dataHandle().setTicketDate(DateTime::localTime());
}

void
STLTicketingCxrRequestHandler::onStartValidatingCxrCheck(const IAttributes& attrs)
{
  _tcsReq->setRequestType(vcx::PLAUSIBILITY_CHECK);
  NationCode c;
  getAttr(attrs, COUNTRYCODE, c);
  _tcsReq->setSpecifiedCountry(c);
}

void
STLTicketingCxrRequestHandler::onStartSettlementPlanCheck(const IAttributes& attrs)
{
  _tcsReq->setRequestType(vcx::SETTLEMENTPLAN_CHECK);
}

void
STLTicketingCxrRequestHandler::onStartPOS(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter POS");
  CrsCode mh;
  getAttr(attrs, MULTIHOST, mh);
  _tcsReq->setMultiHost(mh);
}

void
STLTicketingCxrRequestHandler::onStartActual(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Actual");
  NationCode c;
  getAttr(attrs, COUNTRY, c);
  _tcsReq->setPosCountry(c);
}

void
STLTicketingCxrRequestHandler::onStartHome(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Home");
}

void
STLTicketingCxrRequestHandler::onStartSettlementPlan(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter SettlementPlan");
}

void
STLTicketingCxrRequestHandler::onStartValidatingCxr(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ValidatingCxr");
}

void
STLTicketingCxrRequestHandler::onStartParticipatingCxr(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ParticipatingCxr");
}

void
STLTicketingCxrRequestHandler::onStartTicketDate(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TicketDate");
  // requestDate
  std::string ticketingDate;
  getAttr(attrs, REQUEST_DATE, ticketingDate);

  // requestTimeOfDay
  std::string requestTimeOfDay;
  getAttr(attrs, REQUEST_TIME_OF_DAY, requestTimeOfDay);

  _tcsReq->setTicketDate(DateTime(ticketingDate, atoi(requestTimeOfDay.c_str())));
  _tcsReq->isTicketDateOverride() = true;
  _trx->dataHandle().setTicketDate(DateTime(ticketingDate, atoi(requestTimeOfDay.c_str())));
}

void
STLTicketingCxrRequestHandler::onStartRequestedDiagnostic(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter RequestedDiagnostic");

  // number
  const int16_t diagnosticNumber = attrs.get<int16_t>(NUMBER, 0);
  _tcsReq->diagnosticNumber() = diagnosticNumber;
  _trx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
  _trx->diagnostic().activate();
}

void
STLTicketingCxrRequestHandler::onStartTicketType(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TicketDate");
}

void
STLTicketingCxrRequestHandler::onEndValidatingCxrCheck()
{
  LOG4CXX_DEBUG(_logger, "Leave ValidatingCxrCheck");
}

void
STLTicketingCxrRequestHandler::onEndSettlementPlanCheck()
{
  LOG4CXX_DEBUG(_logger, "Leave SettlementPlanCheck");
}

void
STLTicketingCxrRequestHandler::onEndPOS()
{
  LOG4CXX_DEBUG(_logger, "Leave POS");
}

void
STLTicketingCxrRequestHandler::onEndActual()
{
  LOG4CXX_DEBUG(_logger, "Leave Actual");
}

void
STLTicketingCxrRequestHandler::onEndHome()
{
  LOG4CXX_DEBUG(_logger, "Leave Home");
}

void
STLTicketingCxrRequestHandler::onEndPcc()
{
  PseudoCityCode pcc;
  getValue(pcc);
  _tcsReq->setPcc(pcc);
  LOG4CXX_DEBUG(_logger, "Leave Pcc");
}

void
STLTicketingCxrRequestHandler::onEndSettlementPlan()
{
  SettlementPlanType sp;
  getValue(sp);
  _tcsReq->setSettlementPlan(sp);
  LOG4CXX_DEBUG(_logger, "Leave SettlementPlan");
}

void
STLTicketingCxrRequestHandler::onEndValidatingCxr()
{
  CarrierCode cxr;
  getValue(cxr);
  _tcsReq->setValidatingCxr(cxr);
  LOG4CXX_DEBUG(_logger, "Leave ValidatingCxr: " << cxr);
}

void
STLTicketingCxrRequestHandler::onEndParticipatingCxr()
{
  CarrierCode cxr;
  getValue(cxr);
  vcx::ParticipatingCxr p(cxr, vcx::NO_AGMT);
  _tcsReq->participatingCxrs().push_back(p);
  LOG4CXX_DEBUG(_logger, "Leave ValidatingCxr");
}

//@todo Optimize it
void
STLTicketingCxrRequestHandler::onEndTicketType()
{
  std::string str;
  getValue(str);
  _tcsReq->setTicketType(vcx::getTicketType(str));
  LOG4CXX_DEBUG(_logger, "Leave TicketType");
}

void
STLTicketingCxrRequestHandler::onEndTicketDate()
{
  LOG4CXX_DEBUG(_logger, "Leave TicketDate");
}

void
STLTicketingCxrRequestHandler::onEndRequestedDiagnostic()
{
  LOG4CXX_DEBUG(_logger, "Leave RequestedDiagnostic");
}
} // tse
