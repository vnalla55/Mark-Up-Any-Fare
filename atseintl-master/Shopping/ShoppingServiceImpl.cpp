//----------------------------------------------------------------------------
//
//        File: ShoppingServiceImpl.cpp
// Description: Shopping service class
//     Created: 03/09/2004
//     Authors: Adrienne Stipe, David White
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Shopping/ShoppingServiceImpl.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag194Collector.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag900Collector.h"
#include "Diagnostic/Diag901Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/Diag903Collector.h"
#include "Diagnostic/Diag904Collector.h"
#include "Diagnostic/Diag905Collector.h"
#include "Diagnostic/Diag906Collector.h"
#include "Diagnostic/Diag907Collector.h"
#include "Diagnostic/Diag908Collector.h"
#include "Diagnostic/Diag909Collector.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/Diag911Collector.h"
#include "Diagnostic/Diag912Collector.h"
#include "Diagnostic/Diag914Collector.h"
#include "Diagnostic/Diag930Collector.h"
#include "Diagnostic/Diag952Collector.h"
#include "Diagnostic/Diag953Collector.h"
#include "Diagnostic/Diag954Collector.h"
#include "Diagnostic/Diag956Collector.h"
#include "Diagnostic/Diag957Collector.h"
#include "Diagnostic/Diag958Collector.h"
#include "Diagnostic/Diag959Collector.h"
#include "Diagnostic/Diag969Collector.h"
#include "Diagnostic/Diag986Collector.h"
#include "Diagnostic/Diag989Collector.h"
#include "Server/TseServer.h"

namespace tse
{

static LoadableModuleRegister<Service, ShoppingServiceImpl>
_("libShopping.so");

ShoppingServiceImpl::ShoppingServiceImpl(const std::string& name, tse::TseServer& server)
  : Service(name, server), _config(server.config()), _orchestrator(server)
{
}

bool
ShoppingServiceImpl::process(MetricsTrx& trx)
{
  return _orchestrator.process(trx);
}

bool
ShoppingServiceImpl::process(FlightFinderTrx& trx)
{

  ShoppingTrx& shoppingTrx = trx;
  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(trx);
  Diagnostic& trxDiag = pricingTrx.diagnostic();

  /* Exit when diagnostic deactivated */
  if (!trxDiag.isActive() || trx.ignoreDiagReq())
  {
    return true;
  }

  if (trxDiag.diagnosticType() == Diagnostic969) // show FF respond
  {
    DCFactory* factory = DCFactory::instance();
    Diag969Collector* diagPtr = dynamic_cast<Diag969Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic969);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }

  return process(shoppingTrx);
}

bool
ShoppingServiceImpl::process(RexShoppingTrx& trx)
{

  bool status = _orchestrator.process(trx);

  // Prepare the diagnostic object/trx stuff
  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(trx);
  Diagnostic& trxDiag = pricingTrx.diagnostic();

  DCFactory* factory = DCFactory::instance();

  if (trxDiag.diagnosticType() == Diagnostic905)
  {
    Diag905Collector* diagPtr = dynamic_cast<Diag905Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic905);
    diagPtr->parseQualifiers(trx);

    (*diagPtr) << trx;

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic194)
  {

    Diag194Collector* diagCol = dynamic_cast<Diag194Collector*>(factory->create(trx));
    diagCol->enable(Diagnostic194);

    (*diagCol) << *(dynamic_cast<const RexPricingTrx*>(&trx));

    diagCol->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic986)
  {

    Diag986Collector* diagCol = dynamic_cast<Diag986Collector*>(factory->create(trx));
    diagCol->enable(Diagnostic986);

    (*diagCol) << trx;

    diagCol->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic989)
  {

    Diag989Collector* diagCol = dynamic_cast<Diag989Collector*>(factory->create(trx));
    diagCol->enable(Diagnostic989);

    (*diagCol) << trx;

    diagCol->flushMsg();
  }

  return status;
}

bool
ShoppingServiceImpl::process(ShoppingTrx& trx)
{
  // Prepare the diagnostic object/trx stuff
  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(trx);
  Diagnostic& trxDiag = pricingTrx.diagnostic();

  if (trxDiag.diagnosticType() == Diagnostic900)
  {
    trxDiag.activate();

    DCFactory* factory = DCFactory::instance();
    Diag900Collector* diagPtr = dynamic_cast<Diag900Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic900);
    trxDiag.activate();

    *diagPtr << trx;

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic901)
  {
    trxDiag.activate();

    DCFactory* factory = DCFactory::instance();
    Diag901Collector* diagPtr = dynamic_cast<Diag901Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic901);
    diagPtr->parseQualifiers(trx);

    trxDiag.activate();

    // Check for no legs
    if (trx.legs().empty())
    {
      (*diagPtr) << "NO FARES TO SHOW" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }
    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic902)
  {
    trxDiag.activate();

    DCFactory* factory = DCFactory::instance();
    Diag902Collector* diagPtr = dynamic_cast<Diag902Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic902);
    diagPtr->parseQualifiers(trx);

    // Check for no legs
    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO GROUP" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if ((trxDiag.diagnosticType() == Diagnostic903) &&
           (trxDiag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "VCTRINFO"))
  {
    DCFactory* factory = DCFactory::instance();
    Diag903Collector* diagPtr = dynamic_cast<Diag903Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic903);
    diagPtr->parseQualifiers(trx);

    // Check for no legs
    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic904)
  {
    DCFactory* const factory = DCFactory::instance();
    Diag904Collector* const diagPtr = dynamic_cast<Diag904Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic904);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic905)
  {
    DCFactory* factory = DCFactory::instance();
    Diag905Collector* diagPtr = dynamic_cast<Diag905Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic905);
    diagPtr->parseQualifiers(trx);

    (*diagPtr) << trx;

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic906)
  {
    DCFactory* factory = DCFactory::instance();
    Diag906Collector* diagPtr = dynamic_cast<Diag906Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic906);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic907)
  {
    DCFactory* factory = DCFactory::instance();
    Diag907Collector* diagPtr = dynamic_cast<Diag907Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic907);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic908)
  {
    DCFactory* factory = DCFactory::instance();
    Diag908Collector* diagPtr = dynamic_cast<Diag908Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic908);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic909)
  {
    DCFactory* factory = DCFactory::instance();
    Diag909Collector* diagPtr = dynamic_cast<Diag909Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic909);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic910)
  {
    DCFactory* factory = DCFactory::instance();
    Diag910Collector* diagPtr = dynamic_cast<Diag910Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic910);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic911)
  {
    DCFactory* factory = DCFactory::instance();
    Diag911Collector* diagPtr = dynamic_cast<Diag911Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic911);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic912) // save qualify cat 4
  {
    DCFactory* factory = DCFactory::instance();
    Diag912Collector* diagPtr = dynamic_cast<Diag912Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic912);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic930) // save qualify cat 4
  {
    DCFactory* factory = DCFactory::instance();
    Diag930Collector* diagPtr = dynamic_cast<Diag930Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic930);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic914) // save qualify cat 4
  {
    DCFactory* factory = DCFactory::instance();
    Diag914Collector* diagPtr = dynamic_cast<Diag914Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic914);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic952)
  {
    DCFactory* const factory = DCFactory::instance();
    Diag952Collector* const diagPtr = dynamic_cast<Diag952Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic952);
    (*diagPtr) << trx;

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic953)
  {
    DCFactory* const factory = DCFactory::instance();
    Diag953Collector* const diagPtr = dynamic_cast<Diag953Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic953);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic954)
  {
    DCFactory* const factory = DCFactory::instance();
    Diag954Collector* const diagPtr = dynamic_cast<Diag954Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic954);
    diagPtr->parseQualifiers(trx);

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic956)
  {
    DCFactory* factory = DCFactory::instance();
    Diag956Collector* diagPtr = dynamic_cast<Diag956Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic956);
    diagPtr->activate();

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic957)
  {
    DCFactory* factory = DCFactory::instance();
    Diag957Collector* diagPtr = dynamic_cast<Diag957Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic957);
    diagPtr->activate();

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic958)
  {
    DCFactory* factory = DCFactory::instance();
    Diag958Collector* diagPtr = dynamic_cast<Diag958Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic958);
    diagPtr->activate();

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }
  else if (trxDiag.diagnosticType() == Diagnostic959)
  {
    DCFactory* factory = DCFactory::instance();
    Diag959Collector* diagPtr = dynamic_cast<Diag959Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic959);
    diagPtr->activate();

    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO CALCULATE FARES" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }

  return _orchestrator.process(trx);
}
}
