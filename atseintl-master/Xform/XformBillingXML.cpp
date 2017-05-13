//----------------------------------------------------------------------------
//  File:        XformBillingXML.cpp
//  Created:     February 8, 2006
//  Authors:     Valentin Perov
//  Description:
//
//  Copyright Sabre 2006
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#include "Xform/XformBillingXML.h"

#include "Common/DataCollectorUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TseSrvStats.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AltPricingDetailTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "Server/TseServer.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
// static init
static Logger
logger("atseintl.Xform.XformBillingXML");

FALLBACK_DECL(xformBillingRefactoring);
namespace
{
const std::type_info& _pricingType = typeid(PricingTrx);
const std::type_info& _shoppingType = typeid(ShoppingTrx);
const std::type_info& _fareDisplayType = typeid(FareDisplayTrx);
const std::type_info& _taxType = typeid(TaxTrx);
const std::type_info& _altPricingType = typeid(AltPricingTrx);
const std::type_info& _altPricingDetailType = typeid(AltPricingDetailTrx);
const std::type_info& _noPNRPricingType = typeid(NoPNRPricingTrx);
const std::type_info& _currencyType = typeid(CurrencyTrx);
const std::type_info& _mileageType = typeid(MileageTrx);
const std::type_info& _ticketingCxrType = typeid(TicketingCxrTrx);
const std::type_info& _ticketingCxrDisplayType = typeid(TicketingCxrDisplayTrx);
const std::type_info& _rexPricingType = typeid(RexPricingTrx);
const std::type_info& _refundPricingType = typeid(RefundPricingTrx);
const std::type_info& _exchangePricingType = typeid(ExchangePricingTrx);
const std::type_info& _FlightFinderType = typeid(FlightFinderTrx);
const std::type_info& _multiExchangeType = typeid(MultiExchangeTrx);
const std::type_info& _pricingDetailType = typeid(PricingDetailTrx);
const std::type_info& _rexShoppingTrx = typeid(RexShoppingTrx);
const std::type_info& _ancPricingTrx = typeid(AncillaryPricingTrx);
const std::type_info& _baggageTrx = typeid(BaggageTrx);
const std::type_info& _tktFeesPricingTrx = typeid(TktFeesPricingTrx);

std::string _hostname;
}

static LoadableModuleRegister<Xform, XformBillingXML>
_("libXformBillingXML.so");

//-----------------------------------------------------------------
// @method: XformBillingXML::XformBillingXML
// @params: const std::string&
//          tse::ConfigMan&
// @remark: ctor
//-----------------------------------------------------------------
XformBillingXML::XformBillingXML(const std::string& name, tse::ConfigMan& config)
  : Xform(name, config)
{
}

XformBillingXML::XformBillingXML(const std::string& name, tse::TseServer& srv)
  : Xform(name, srv.config())
{
}

//-----------------------------------------------------------------
// @method: XformBillingXML::~XformBillingXML()
// @remark: ctor
//-----------------------------------------------------------------
XformBillingXML::~XformBillingXML() {}

//-----------------------------------------------------------------
// @method: XformBillingXML::initialize
// @return: bool
// @params: int
//          char*[]
// @remark: initialize Xform
//-----------------------------------------------------------------
bool
XformBillingXML::initialize(int argc, char* argv[])
{
  // get host name
  char host[1024];

  if (::gethostname(host, 1023) < 0) // lint !e505 !e506
  {
    return false;
  }

  _hostname = host; // lint !e603

  return true;
}

//-----------------------------------------------------------------
// @method: XformBillingXML::convert
// @return: bool
// @params: Trx&
//          std::string&
// @remark: converts Billing info to XML
//-----------------------------------------------------------------
bool
XformBillingXML::convert(Trx& trx, std::string& response)
{
  const Billing* billing = nullptr;

  if (fallback::xformBillingRefactoring(&trx))
  {
    const std::type_info& trxType = typeid(trx);

    if (trxType == _pricingType || trxType == _rexPricingType || trxType == _refundPricingType ||
        trxType == _exchangePricingType)
    {
      billing = dynamic_cast<PricingTrx&>(trx).billing();
    }
    else if (trxType == _taxType)
    {
      billing = dynamic_cast<TaxTrx&>(trx).billing();
    }
    else if (trxType == _fareDisplayType)
    {
      billing = dynamic_cast<FareDisplayTrx&>(trx).billing();
    }
    else if (trxType == _shoppingType)
    {
      billing = dynamic_cast<ShoppingTrx&>(trx).billing();
    }
    else if (trxType == _altPricingType)
    {
      billing = dynamic_cast<AltPricingTrx&>(trx).billing();
    }
    else if (trxType == _altPricingDetailType)
    {
      billing = dynamic_cast<AltPricingDetailTrx&>(trx).billing();
    }
    else if (trxType == _noPNRPricingType)
    {
      billing = dynamic_cast<NoPNRPricingTrx&>(trx).billing();
    }
    else if (trxType == _currencyType)
    {
      billing = dynamic_cast<CurrencyTrx&>(trx).billing();
    }
    else if (trxType == _mileageType)
    {
      billing = dynamic_cast<MileageTrx&>(trx).billing();
    }
    else if (trxType == _ticketingCxrType)
    {
      billing = dynamic_cast<TicketingCxrTrx&>(trx).billing();
    }
    else if (trxType == _ticketingCxrDisplayType)
    {
      billing = dynamic_cast<TicketingCxrDisplayTrx&>(trx).billing();
    }
    else if (trxType == _FlightFinderType)
    {
      billing = dynamic_cast<FlightFinderTrx&>(trx).billing();
    }
    else if (trxType == _multiExchangeType)
    {
      billing = static_cast<MultiExchangeTrx&>(trx).newPricingTrx()->billing();
    }
    else if (trxType == _pricingDetailType)
    {
      billing = dynamic_cast<PricingDetailTrx&>(trx).billing();
    }
    else if (trxType == _rexShoppingTrx)
    {
      billing = dynamic_cast<RexShoppingTrx&>(trx).billing();
    }
    else if (trxType == _ancPricingTrx)
    {
      billing = dynamic_cast<AncillaryPricingTrx&>(trx).billing();
    }
    else if (trxType == _baggageTrx)
    {
      billing = dynamic_cast<BaggageTrx&>(trx).billing();
    }
    else if (trxType == _tktFeesPricingTrx)
    {
      billing = dynamic_cast<TktFeesPricingTrx&>(trx).billing();
    }
    else if (trxType == typeid(DecodeTrx))
    {
      billing = static_cast<DecodeTrx&>(trx).billing();
    }
    else if (trxType == typeid(StructuredRuleTrx))
    {
      billing = static_cast<PricingTrx&>(trx).billing();
    }
    else
    {
      LOG4CXX_ERROR(logger, "Transaction type is not supported.");
      return false;
    }
  }
  else
  {
    billing = trx.billing();
  }
  if (billing == nullptr)
    return false; // Nothing to do
  buildXML(*billing, trx, response);

  return true;
}

//-------------------------------------------------------------------------
// @method: XformBillingXML::buildXML
// @return: void
// @params: const Billing&
//          const Trx&
//          std::string&
// @remark: build XML string from Billing
//--------------------------------------------------------------------------
void
XformBillingXML::buildXML(const Billing& billing, const Trx& trx, std::string& xmlString)
{
  XMLConstruct construct;

  construct.openElement(xml2::BillingInformation);

  construct.addAttribute(xml2::UserPseudoCityCode, billing.userPseudoCityCode());
  construct.addAttribute(xml2::UserStation, billing.userStation());
  construct.addAttribute(xml2::UserBranch, billing.userBranch());
  construct.addAttribute(xml2::PartitionID, billing.partitionID());
  construct.addAttributeULong(xml2::TrxID, billing.transactionID());
  construct.addAttributeULong(xml2::ClientTrxID, billing.clientTransactionID());
  construct.addAttributeULong(xml2::ParentTrxID, billing.parentTransactionID());
  construct.addAttribute(xml2::UserSetAddress, billing.userSetAddress());
  construct.addAttribute(xml2::ServiceName, billing.serviceName());
  construct.addAttribute(xml2::ClientSvcName, billing.clientServiceName());
  construct.addAttribute(xml2::ParentSvcName, billing.parentServiceName());
  construct.addAttribute(xml2::AaaCity, billing.aaaCity());
  construct.addAttribute(xml2::AaaSine, billing.aaaSine());
  construct.addAttribute(xml2::ActionCode, billing.actionCode());
  construct.addAttributeUInteger(xml2::SolutionsRequested, TseSrvStats::getSolutionsRequested(trx));
  construct.addAttributeUInteger(xml2::SolutionsProduced, TseSrvStats::getSolutionsFound(trx));

  if (!_hostname.empty())
    construct.addAttribute(xml2::NodeId, _hostname);

  construct.addAttribute(xml2::TrxStartTime, DataCollectorUtil::format(trx.transactionStartTime()));
  construct.addAttribute(xml2::TrxEndTime, DataCollectorUtil::format(trx.transactionEndTime()));

  {
    // Convert Seconds to Microseconds
    uint64_t cpuMicros =
        static_cast<uint64_t>(TseSrvStats::getTrxCPUTime(const_cast<Trx&>(trx)) * 1000000);
    std::ostringstream oss;
    oss << cpuMicros;
    construct.addAttribute(xml2::TrxCPU, oss.str());
  }

  {
    std::ostringstream oss;
    oss << trx.transactionRC();
    construct.addAttribute(xml2::TrxRC, oss.str());
  }

  construct.closeElement();

  xmlString = construct.getXMLData();
}
} // tse
