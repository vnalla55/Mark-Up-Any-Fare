#include "Xform/STLTicketingCxrDisplayRequestHandler.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/STLTicketingCxrSchemaNames.h"

namespace tse
{
using namespace stlticketingcxr;

namespace {
  Logger _logger("atseintl.Xform.STLTicketingCxrDisplayRequestHandler");

  ILookupMap _elemLookupMapTktCxr, _attrLookupMapTktCxr;
  bool
    init(IXMLUtils::initLookupMaps(_STLTicketingCxrElementNames,
          NUMBER_OF_ELEMENT_NAMES_TKT_CXR,
          _elemLookupMapTktCxr,
          _STLTicketingCxrAttributeNames,
          NUMBER_OF_ATTRIBUTE_NAMES_TKT_CXR,
          _attrLookupMapTktCxr));
}

STLTicketingCxrDisplayRequestHandler::STLTicketingCxrDisplayRequestHandler(Trx*& trx)
  : CommonRequestHandler(trx)
{
}

void
STLTicketingCxrDisplayRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
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
STLTicketingCxrDisplayRequestHandler::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = dataHandle.create<TicketingCxrDisplayTrx>();
  _trx->dataHandle().get(_tcdReq);
  static_cast<TicketingCxrDisplayTrx*>(_trx)->getRequest() = _tcdReq;

  Billing* billing;
  _trx->dataHandle().get(billing);
  static_cast<TicketingCxrDisplayTrx*>(_trx)->billing() = billing;
  _trx->dataHandle().setTicketDate(DateTime::localTime());
}

bool
STLTicketingCxrDisplayRequestHandler::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case DISPLAY_VALIDATINGCXR:
    onStartDisplayValidatingCxr(attrs);
    break;
  case DISPLAY_INTERLINEAGREEMENT:
    onStartDisplayInterlineAgreement(attrs);
    break;
  case POS:
    onStartPOS(attrs);
    break;
  case ACTUAL:
    onStartActual(attrs);
    break;
  case REQUESTED_DIAGNOSTIC:
    onStartRequestedDiagnostic(attrs);
    break;
  case BILLINGINFORMATION:
    {
      Billing* billing = static_cast<TicketingCxrDisplayTrx*>(_trx)->billing();
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
STLTicketingCxrDisplayRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case DISPLAY_VALIDATINGCXR:
    onEndDisplayValidatingCxr();
    break;
  case DISPLAY_INTERLINEAGREEMENT:
    onEndDisplayInterlineAgreement();
    break;
  case POS:
    onEndPOS();
    break;
  case ACTUAL:
    onEndActual();
    break;
  case PCC:
    onEndPcc();
    break;
  case REQUESTED_DIAGNOSTIC:
    onEndRequestedDiagnostic();
    break;
  case BILLINGINFORMATION:
    {
      Billing* billing = static_cast<TicketingCxrDisplayTrx*>(_trx)->billing();
      if (billing)
      {
        onEndValCxrBIL(*billing,
            Billing::SVC_TICKETINGCXR_DISP,
            _trx->transactionId());
      }
      break;
    }
  }
  return true;
}

void
STLTicketingCxrDisplayRequestHandler::onStartDisplayValidatingCxr(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DisplayValidatingCxr");
  _tcdReq->setRequestType(vcx::DISPLAY_VCXR);

  NationCode cntry;
  getAttr(attrs, COUNTRY, cntry);
  _tcdReq->specifiedCountry() = cntry;

  CrsCode ph;
  getAttr(attrs, PRIMEHOST, ph);
  _tcdReq->specifiedPrimeHost() = ph;

  SettlementPlanType sp;
  getAttr(attrs, SETTLEMENTPLAN, sp);
  _tcdReq->settlementPlan() = sp;
}

void
STLTicketingCxrDisplayRequestHandler::onStartDisplayInterlineAgreement(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DisplayInterlineAgreement");
  _tcdReq->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);

  NationCode cntry;
  getAttr(attrs, COUNTRY, cntry);
  _tcdReq->specifiedCountry() = cntry;

  CarrierCode cxr;
  getAttr(attrs, CARRIER_CODE, cxr);
  _tcdReq->validatingCxr() = cxr;

  CrsCode ph;
  getAttr(attrs, PRIMEHOST, ph);
  _tcdReq->specifiedPrimeHost() = ph;
}

void
STLTicketingCxrDisplayRequestHandler::onStartPOS(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter POS");
  CrsCode mh;
  getAttr(attrs, MULTIHOST, mh);
  _tcdReq->pointOfSale().primeHost()=mh;
}

void
STLTicketingCxrDisplayRequestHandler::onStartActual(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Actual");
  NationCode c;
  getAttr(attrs, COUNTRY, c);
  _tcdReq->pointOfSale().country()=c;
}

void
STLTicketingCxrDisplayRequestHandler::onEndActual()
{
  LOG4CXX_DEBUG(_logger, "Leave Actual");
}

void
STLTicketingCxrDisplayRequestHandler::onEndPOS()
{
  LOG4CXX_DEBUG(_logger, "Leave POS");
}

void
STLTicketingCxrDisplayRequestHandler::onEndPcc()
{
  PseudoCityCode pcc;
  getValue(pcc);
  _tcdReq->pointOfSale().pcc()=pcc;
  LOG4CXX_DEBUG(_logger, "Leave Pcc");
}

void
STLTicketingCxrDisplayRequestHandler::onStartRequestedDiagnostic(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter RequestedDiagnostic");

  // number
  const int16_t diagnosticNumber = attrs.get<int16_t>(NUMBER, 0);
  _tcdReq->diagnosticNumber() = diagnosticNumber;
  _trx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
  _trx->diagnostic().activate();
}

void
STLTicketingCxrDisplayRequestHandler::onEndRequestedDiagnostic()
{
  LOG4CXX_DEBUG(_logger, "Leave RequestedDiagnostic");
}

void
STLTicketingCxrDisplayRequestHandler::onEndDisplayValidatingCxr()
{
  LOG4CXX_DEBUG(_logger, "Leave DisplayValidatingCxr");
}

void
STLTicketingCxrDisplayRequestHandler::onEndDisplayInterlineAgreement()
{
  LOG4CXX_DEBUG(_logger, "Leave DisplayInterlineAgreement");
}

}
