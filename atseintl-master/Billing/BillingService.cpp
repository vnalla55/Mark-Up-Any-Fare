/************************************************************************
 * @file     BillingService.h
 * @date     04/25/2005
 * @author   Konstantin Sidorin, Valentin Perov
 *
 * @brief    Implementation file Billing Service.
 *
 *  Updates:
 *
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 *************************************************************************/

#include "Billing/BillingService.h"

#include "ClientSocket/ClientSocket.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/StopWatch.h"
#include "Common/TseSrvStats.h"
#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/Xform.h"

namespace
{
// Client socket didnt expose the sizeof(TRX_HEADER) so had to add this
// TODO: move it to ClientSocket
const size_t SOCKET_HEADER_SIZE = 16;

static const char* REQ_CMD = "REQ";
static const char* SCHEMA_VERSION = "0001";
static const char* SCHEMA_REVISION = "0000";
static const char* XMD_START_TAG = "<XMD>";
static const char* XMD_END_TAG = "</XMD>";

static const char* SUCCESS_RC = "OK";
}

namespace tse
{
namespace
{
ConfigurableValue<std::string>
xformName("BILLING_SVC", "XFORM_NAME");

ClientSocketConfigurableValues cscv("BILLING_SVC");
}
static Logger
logger("atseintl.Service.BillingService");

static LoadableModuleRegister<Service, BillingService>
_("libBilling.so");

bool
BillingService::initialize(int argc, char* argv[])
{
  _xformName = xformName.getValue();
  return !_xformName.empty();
}

bool
BillingService::processTrx(Trx& trx)
{
  // skip billing for the upSell transactions
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
  if (pricingTrx && pricingTrx->getRequest() && pricingTrx->getRequest()->upSellEntry())
  {
    LOG4CXX_INFO(logger, "UpSell Trx - skip billing.");
    return true;
  }

  // get Xform
  Xform* xform = nullptr;
  xform = _server.xform(_xformName);
  if (xform == nullptr)
  {
    LOG4CXX_FATAL(logger, "Xform is not valid.");
    return false;
  }

  // convert Billing to XML
  std::string bill;

  if (!xform->convert(trx, bill))
  {
    LOG4CXX_ERROR(logger, "Can't convert Trx to Billing XML.");
    return false;
  }

  // send XML to Socket
  if (!send(bill, trx))
  {
    LOG4CXX_ERROR(logger, "Can't send Billing Info.");
    return false;
  }

  return true;
}

bool
BillingService::send(const std::string& msg, Trx& trx)
{
  // Construct XML-TCP/IP header
  std::string command(REQ_CMD);
  std::string version(SCHEMA_VERSION);
  std::string revision(SCHEMA_REVISION);
  std::string message = XMD_START_TAG + msg + XMD_END_TAG;

  uint64_t reqSize = message.length() + SOCKET_HEADER_SIZE;
  uint64_t rspSize = 0;

  tse::StopWatch sw;
  sw.start();

  ClientSocket socket(cscv);
  const bool init = socket.initialize();
  if (!init)
  {
    LOG4CXX_ERROR(logger, "Could not initialize billing socket");
    return false;
  }

  // send to socket
  if (!socket.send(command, version, revision, message))
  {
    LOG4CXX_FATAL(logger, "Can't send Billing Info");
    sw.stop();
    TseSrvStats::recordBillingCall(false, reqSize, rspSize, sw.elapsedTime(), trx);
    return false;
  }

  LOG4CXX_INFO(logger, "Sent Billing Info: " << message);

  // receive reply (if any errors)
  if (!socket.receive(command, version, revision, message))
  {
    LOG4CXX_FATAL(logger, "Can't receive Billing Info");
    sw.stop();
    TseSrvStats::recordBillingCall(false, reqSize, rspSize, sw.elapsedTime(), trx);
    return false;
  }

  sw.stop();

  LOG4CXX_INFO(logger,
               "Received from DataCollector: "
                   << "\nCommand  [" << command << "]; "
                   << "\nVersion  [" << version << "]; "
                   << "\nRevision [" << revision << "]; "
                   << "\nMessage  [" << message << "]; ");

  bool success = (message == SUCCESS_RC);

  rspSize = message.length() + SOCKET_HEADER_SIZE;
  TseSrvStats::recordBillingCall(success, reqSize, rspSize, sw.elapsedTime(), trx);

  return true; // Billing Service shouldn't block trx
}
} // tse namespace
