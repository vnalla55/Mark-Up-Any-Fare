//----------------------------------------------------------------------------
//  File:         DCFactory.C
//  Description:  Parameterized Factory Method for creating specific diagnostic
//                object.
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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
#include "Diagnostic/DCFactory.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DiagCollListInc.h"

#include <iostream>

namespace tse
{
// static members
boost::mutex DCFactory::_mutex;
boost::mutex DCFactory::_mutexThreadDiag;
DCFactory::PidDiagMap DCFactory::_pidDiagMap;

// A candidate to be volatile
DCFactory* DCFactory::_instance = nullptr;

DCFactory::DCFactory()
{
  DCFactoryBase::initialize(this);
  initializeDiagCollectorCreators();
}

DiagCollector*
DCFactory::create(Trx& trx)
{
  DiagCollector* dc;

  if (LIKELY(trx.diagnostic().diagnosticType() == DiagnosticNone))
  {
    dc = thrDisabledDC(trx);
  }
  else
  {
    pthread_t pid = pthread_self();
    boost::lock_guard<boost::mutex> g(_mutexThreadDiag);
    // Because we encounted many problem to existing code,
    // we will keep the Diagnostic per thread only

    Diagnostic& diag = threadDiag(trx, pid);

    dc = threadCreate(trx, diag);
    if (dc)
    {
      dc->deActivate();
      dc->useThreadDiag() = true;
    }
  }
  return dc;
}

void DCFactory::initializeDiagCollectorCreators()
{
  _diagCreatorMap[SimilarItinIADiagnostic] =
      createSharedPtr(new DiagCollectorCreator<SimilarItinIADiagCollector>());
  _diagCreatorMap[Diagnostic185] = createSharedPtr(new DiagCollectorCreator<Diag185Collector>());
  _diagCreatorMap[Diagnostic187] = createSharedPtr(new DiagCollectorCreator<Diag187Collector>());
  _diagCreatorMap[Diagnostic188] = createSharedPtr(new DiagCollectorCreator<Diag188Collector>());
  _diagCreatorMap[Diagnostic191] = createSharedPtr(new DiagCollectorCreator<Diag191Collector>());
  _diagCreatorMap[Diagnostic192] = createSharedPtr(new DiagCollectorCreator<Diag192Collector>());
  _diagCreatorMap[Diagnostic194] = createSharedPtr(new DiagCollectorCreator<Diag194Collector>());
  _diagCreatorMap[Diagnostic198] = createSharedPtr(new DiagCollectorCreator<Diag198Collector>());
  _diagCreatorMap[Diagnostic199] = createSharedPtr(new DiagCollectorCreator<Diag199Collector>());
  _diagCreatorMap[AllFareDiagnostic] = createSharedPtr(new DiagCollectorCreator<Diag200Collector>());
  _diagCreatorMap[Diagnostic201] = createSharedPtr(new DiagCollectorCreator<Diag201Collector>());
  _diagCreatorMap[Diagnostic202] = createSharedPtr(new DiagCollectorCreator<Diag202Collector>());
  _diagCreatorMap[Diagnostic607] = createSharedPtr(new DiagCollectorCreator<Diag203Collector>());
  _diagCreatorMap[Diagnostic204] = createSharedPtr(new DiagCollectorCreator<Diag204Collector>());
  _diagCreatorMap[Diagnostic208] = createSharedPtr(new DiagCollectorCreator<Diag208Collector>());
  // FareCalcConfig
  _diagCreatorMap[Diagnostic212] = createSharedPtr(new DiagCollectorCreator<DiagCollector>);
  _diagCreatorMap[Diagnostic220] = createSharedPtr(new DiagCollectorCreator<Diag220Collector>);
  _diagCreatorMap[Diagnostic223] = createSharedPtr(new DiagCollectorCreator<Diag223Collector>);
  _diagCreatorMap[Diagnostic225] = createSharedPtr(new DiagCollectorCreator<Diag225Collector>);
  _diagCreatorMap[Diagnostic231] = createSharedPtr(new DiagCollectorCreator<Diag23XCollector>);
  _diagCreatorMap[Diagnostic233] = createSharedPtr(new DiagCollectorCreator<Diag23XCollector>);
  _diagCreatorMap[Diagnostic240] = createSharedPtr(new DiagCollectorCreator<Diag240Collector>);
  _diagCreatorMap[Diagnostic251] = createSharedPtr(new DiagCollectorCreator<Diag251Collector>);
  _diagCreatorMap[Diagnostic252] = createSharedPtr(new DiagCollectorCreator<Diag252Collector>);
  _diagCreatorMap[Diagnostic253] = createSharedPtr(new DiagCollectorCreator<Diag253Collector>);
  _diagCreatorMap[Diagnostic254] = createSharedPtr(new DiagCollectorCreator<Diag254Collector>);
  _diagCreatorMap[Diagnostic255] = createSharedPtr(new DiagCollectorCreator<Diag255Collector>);
  _diagCreatorMap[Diagnostic256] = createSharedPtr(new DiagCollectorCreator<Diag256Collector>);
  _diagCreatorMap[Diagnostic257] = createSharedPtr(new DiagCollectorCreator<Diag257Collector>);
  _diagCreatorMap[Diagnostic258] = createSharedPtr(new DiagCollectorCreator<Diag258Collector>);
  _diagCreatorMap[Diagnostic259] = createSharedPtr(new DiagCollectorCreator<Diag259Collector>);
  _diagCreatorMap[Diagnostic270] = createSharedPtr(new DiagCollectorCreator<Diag270Collector>);
  _diagCreatorMap[Diagnostic302] = createSharedPtr(new DiagCollectorCreator<Diag302Collector>);
  _diagCreatorMap[Diagnostic304] = createSharedPtr(new DiagCollectorCreator<Diag304Collector>);
  _diagCreatorMap[Diagnostic306] = createSharedPtr(new DiagCollectorCreator<Diag306Collector>);
  _diagCreatorMap[Diagnostic311] = createSharedPtr(new DiagCollectorCreator<Diag311Collector>);
  _diagCreatorMap[Diagnostic312] = createSharedPtr(new DiagCollectorCreator<Diag312Collector>);
  _diagCreatorMap[Diagnostic315] = createSharedPtr(new DiagCollectorCreator<Diag315Collector>);
  _diagCreatorMap[Diagnostic316] = createSharedPtr(new DiagCollectorCreator<Diag316Collector>);
  _diagCreatorMap[Diagnostic323] = createSharedPtr(new DiagCollectorCreator<Diag323Collector>);
  _diagCreatorMap[Diagnostic325] = createSharedPtr(new DiagCollectorCreator<Diag325Collector>);
  _diagCreatorMap[Diagnostic327] = createSharedPtr(new DiagCollectorCreator<Diag327Collector>);
  _diagCreatorMap[Diagnostic335] = createSharedPtr(new DiagCollectorCreator<Diag335Collector>);
  _diagCreatorMap[Diagnostic372] = createSharedPtr(new DiagCollectorCreator<Diag372Collector>);
  _diagCreatorMap[Diagnostic400] = createSharedPtr(new DiagCollectorCreator<Diag400Collector>);

  _diagCreatorMap[Diagnostic404] = createSharedPtr(new DiagCollectorCreator<Diag405Collector>);
  _diagCreatorMap[Diagnostic405] = createSharedPtr(new DiagCollectorCreator<Diag405Collector>);

  _diagCreatorMap[Diagnostic411] = createSharedPtr(new DiagCollectorCreator<Diag411Collector>);
  _diagCreatorMap[Diagnostic413] = createSharedPtr(new DiagCollectorCreator<Diag413Collector>);
  _diagCreatorMap[Diagnostic419] = createSharedPtr(new DiagCollectorCreator<Diag419Collector>);
  _diagCreatorMap[Diagnostic430] = createSharedPtr(new DiagCollectorCreator<Diag430Collector>);
  _diagCreatorMap[Diagnostic450] = createSharedPtr(new DiagCollectorCreator<Diag450Collector>);
  _diagCreatorMap[Diagnostic451] = createSharedPtr(new DiagCollectorCreator<Diag451Collector>);
  _diagCreatorMap[Diagnostic452] = createSharedPtr(new DiagCollectorCreator<Diag452Collector>);
  _diagCreatorMap[Diagnostic455] = createSharedPtr(new DiagCollectorCreator<Diag455Collector>);
  _diagCreatorMap[Diagnostic457] = createSharedPtr(new DiagCollectorCreator<Diag457Collector>);
  _diagCreatorMap[Diagnostic460] = createSharedPtr(new DiagCollectorCreator<Diag460Collector>);
  _diagCreatorMap[Diagnostic500] = createSharedPtr(new DiagCollectorCreator<Diag500Collector>);
  _diagCreatorMap[Diagnostic502] = createSharedPtr(new DiagCollectorCreator<Diag502Collector>);
  _diagCreatorMap[Diagnostic512] = createSharedPtr(new DiagCollectorCreator<Diag512Collector>);
  _diagCreatorMap[Diagnostic527] = createSharedPtr(new DiagCollectorCreator<Diag527Collector>);
  _diagCreatorMap[Diagnostic535] = createSharedPtr(new DiagCollectorCreator<Diag535Collector>);
  _diagCreatorMap[Diagnostic550] = createSharedPtr(new DiagCollectorCreator<Diag550Collector>);
  _diagCreatorMap[Diagnostic600] = createSharedPtr(new DiagCollectorCreator<Diag600Collector>);
  _diagCreatorMap[Diagnostic601] = createSharedPtr(new DiagCollectorCreator<Diag601Collector>);
  _diagCreatorMap[Diagnostic602] = createSharedPtr(new DiagCollectorCreator<Diag602Collector>);
  _diagCreatorMap[Diagnostic603] = createSharedPtr(new DiagCollectorCreator<Diag603Collector>);
  _diagCreatorMap[Diagnostic605] = createSharedPtr(new DiagCollectorCreator<Diag605Collector>);
  _diagCreatorMap[Diagnostic606] = createSharedPtr(new DiagCollectorCreator<Diag606Collector>);

  _diagCreatorMap[Diagnostic610] = createSharedPtr(new DiagCollectorCreator<Diag610Collector>);
  _diagCreatorMap[Diagnostic612] = createSharedPtr(new DiagCollectorCreator<Diag610Collector>);

  _diagCreatorMap[Diagnostic611] = createSharedPtr(new DiagCollectorCreator<Diag611Collector>);
  _diagCreatorMap[Diagnostic614] = createSharedPtr(new DiagCollectorCreator<Diag614Collector>);
  _diagCreatorMap[Diagnostic620] = createSharedPtr(new DiagCollectorCreator<Diag620Collector>);
  _diagCreatorMap[Diagnostic625] = createSharedPtr(new DiagCollectorCreator<Diag625Collector>);
  _diagCreatorMap[Diagnostic660] = createSharedPtr(new DiagCollectorCreator<Diag660Collector>);
  _diagCreatorMap[Diagnostic666] = createSharedPtr(new DiagCollectorCreator<Diag666Collector>);
  _diagCreatorMap[Diagnostic671] = createSharedPtr(new DiagCollectorCreator<Diag671Collector>);
  _diagCreatorMap[Diagnostic676] = createSharedPtr(new DiagCollectorCreator<Diag676Collector>);
  _diagCreatorMap[Diagnostic688] = createSharedPtr(new DiagCollectorCreator<Diag688Collector>());
  _diagCreatorMap[Diagnostic689] = createSharedPtr(new DiagCollectorCreator<Diag689Collector>);

  _diagCreatorMap[Diagnostic690] = createSharedPtr(new DiagCollectorCreator<Diag690Collector>);
  _diagCreatorMap[Diagnostic691] = createSharedPtr(new DiagCollectorCreator<Diag690Collector>);
  _diagCreatorMap[Diagnostic693] = createSharedPtr(new DiagCollectorCreator<Diag690Collector>);

  _diagCreatorMap[FailTaxCodeDiagnostic] = createSharedPtr(new DiagCollectorCreator<Diag800Collector>);
  _diagCreatorMap[AllPassTaxDiagnostic281] = createSharedPtr(new DiagCollectorCreator<Diag801Collector>);
  _diagCreatorMap[TaxRecSummaryDiagnostic] = createSharedPtr(new DiagCollectorCreator<Diag802Collector>);
  _diagCreatorMap[PFCRecSummaryDiagnostic] = createSharedPtr(new DiagCollectorCreator<Diag803Collector>);

  _diagCreatorMap[Diagnostic807] = createSharedPtr(new DiagCollectorCreator<Diag807Collector>);
  _diagCreatorMap[Diagnostic817] = createSharedPtr(new DiagCollectorCreator<Diag817Collector>);
  _diagCreatorMap[Diagnostic825] = createSharedPtr(new DiagCollectorCreator<Diag825Collector>);
  // Fare Calc Diag
  _diagCreatorMap[Diagnostic851] = createSharedPtr(new DiagCollectorCreator<Diag851Collector>);
  // Baggage Allowance Diag
  _diagCreatorMap[Diagnostic852] = createSharedPtr(new DiagCollectorCreator<Diag852Collector>);
  // Fare Calc Diag
  _diagCreatorMap[Diagnostic853] = createSharedPtr(new DiagCollectorCreator<Diag853Collector>);
  // Fare Calc Diag
  _diagCreatorMap[Diagnostic854] = createSharedPtr(new DiagCollectorCreator<Diag854Collector>);
  // WQCC Diag
  _diagCreatorMap[Diagnostic856] = createSharedPtr(new DiagCollectorCreator<Diag856Collector>);
  _diagCreatorMap[Diagnostic858] = createSharedPtr(new DiagCollectorCreator<Diag858Collector>);
  // Fare Calc Diag WPA
  _diagCreatorMap[Diagnostic860] = createSharedPtr(new DiagCollectorCreator<Diag860Collector>);
  // NVB/NVA processing
  _diagCreatorMap[Diagnostic861] = createSharedPtr(new DiagCollectorCreator<Diag861Collector>);
  // Consolidator Plus Up Diag
  _diagCreatorMap[Diagnostic864] = createSharedPtr(new DiagCollectorCreator<Diag864Collector>);
  // Cat35 commissions
  _diagCreatorMap[Diagnostic865] = createSharedPtr(new DiagCollectorCreator<Diag865Collector>);
  // Tkt Commission (commission cap)
  _diagCreatorMap[Diagnostic866] = createSharedPtr(new DiagCollectorCreator<Diag866Collector>);
  // Commission Management
  _diagCreatorMap[Diagnostic867] = createSharedPtr(new DiagCollectorCreator<Diag867Collector>);
  // Service Fee - OB Fee
  _diagCreatorMap[Diagnostic868] = createSharedPtr(new DiagCollectorCreator<Diag868Collector>);
  _diagCreatorMap[Diagnostic870] = createSharedPtr(new DiagCollectorCreator<Diag870Collector>);
  // Service Fee - OC Fee S5
  _diagCreatorMap[Diagnostic875] = createSharedPtr(new DiagCollectorCreator<Diag875Collector>);
  // Service Fee - OC Fee S6
  _diagCreatorMap[Diagnostic876] = createSharedPtr(new DiagCollectorCreator<Diag876Collector>);
  // Service Fee - OC Fee S7
  _diagCreatorMap[Diagnostic877] = createSharedPtr(new DiagCollectorCreator<Diag877Collector>);
  _diagCreatorMap[Diagnostic878] = createSharedPtr(new DiagCollectorCreator<Diag878Collector>);
  _diagCreatorMap[Diagnostic879] = createSharedPtr(new DiagCollectorCreator<Diag879Collector>);
  // Service Fee - OC Slice and Dice
  _diagCreatorMap[Diagnostic880] = createSharedPtr(new DiagCollectorCreator<Diag880Collector>);
  // Branded Fares - S8
  _diagCreatorMap[Diagnostic888] = createSharedPtr(new DiagCollectorCreator<Diag888Collector>);
  // Branded Fares - T189
  _diagCreatorMap[Diagnostic889] = createSharedPtr(new DiagCollectorCreator<Diag889Collector>);
  // Branded Fares - Request XML
  _diagCreatorMap[Diagnostic890] = createSharedPtr(new DiagCollectorCreator<Diag890Collector>);
  // Branded Fares - Response XML
  _diagCreatorMap[Diagnostic891] = createSharedPtr(new DiagCollectorCreator<Diag891Collector>);
  // Branded Fares - Brand Info
  _diagCreatorMap[Diagnostic892] = createSharedPtr(new DiagCollectorCreator<Diag892Collector>);
  _diagCreatorMap[Diagnostic893] = createSharedPtr(new DiagCollectorCreator<Diag893Collector>);
  _diagCreatorMap[Diagnostic894] = createSharedPtr(new DiagCollectorCreator<Diag894Collector>);
  // Branded Fares - S8
  _diagCreatorMap[Diagnostic898] = createSharedPtr(new DiagCollectorCreator<Diag898Collector>);
  _diagCreatorMap[Diagnostic900] = createSharedPtr(new DiagCollectorCreator<Diag900Collector>);
  _diagCreatorMap[Diagnostic901] = createSharedPtr(new DiagCollectorCreator<Diag901Collector>);
  _diagCreatorMap[Diagnostic902] = createSharedPtr(new DiagCollectorCreator<Diag902Collector>);
  _diagCreatorMap[Diagnostic903] = createSharedPtr(new DiagCollectorCreator<Diag903Collector>);
  _diagCreatorMap[Diagnostic904] = createSharedPtr(new DiagCollectorCreator<Diag904Collector>);
  _diagCreatorMap[Diagnostic905] = createSharedPtr(new DiagCollectorCreator<Diag905Collector>);
  _diagCreatorMap[Diagnostic906] = createSharedPtr(new DiagCollectorCreator<Diag906Collector>);
  _diagCreatorMap[Diagnostic907] = createSharedPtr(new DiagCollectorCreator<Diag907Collector>);
  _diagCreatorMap[Diagnostic908] = createSharedPtr(new DiagCollectorCreator<Diag908Collector>);
  _diagCreatorMap[Diagnostic909] = createSharedPtr(new DiagCollectorCreator<Diag909Collector>);
  _diagCreatorMap[Diagnostic910] = createSharedPtr(new DiagCollectorCreator<Diag910Collector>);
  _diagCreatorMap[Diagnostic911] = createSharedPtr(new DiagCollectorCreator<Diag911Collector>);
  _diagCreatorMap[Diagnostic912] = createSharedPtr(new DiagCollectorCreator<Diag912Collector>);
  _diagCreatorMap[Diagnostic922] = createSharedPtr(new DiagCollectorCreator<Diag922Collector>);
  _diagCreatorMap[Diagnostic923] = createSharedPtr(new DiagCollectorCreator<Diag923Collector>);
  _diagCreatorMap[Diagnostic924] = createSharedPtr(new DiagCollectorCreator<Diag924Collector>);
  _diagCreatorMap[Diagnostic930] = createSharedPtr(new DiagCollectorCreator<Diag930Collector>);
  _diagCreatorMap[Diagnostic931] = createSharedPtr(new DiagCollectorCreator<Diag931Collector>);
  _diagCreatorMap[Diagnostic914] = createSharedPtr(new DiagCollectorCreator<Diag914Collector>);
  _diagCreatorMap[Diagnostic941] = createSharedPtr(new DiagCollectorCreator<Diag941Collector>);
  _diagCreatorMap[Diagnostic952] = createSharedPtr(new DiagCollectorCreator<Diag952Collector>);
  _diagCreatorMap[Diagnostic953] = createSharedPtr(new DiagCollectorCreator<Diag953Collector>);
  _diagCreatorMap[Diagnostic954] = createSharedPtr(new DiagCollectorCreator<Diag954Collector>);
  _diagCreatorMap[Diagnostic956] = createSharedPtr(new DiagCollectorCreator<Diag956Collector>);
  _diagCreatorMap[Diagnostic957] = createSharedPtr(new DiagCollectorCreator<Diag957Collector>);
  _diagCreatorMap[Diagnostic958] = createSharedPtr(new DiagCollectorCreator<Diag958Collector>);
  _diagCreatorMap[Diagnostic959] = createSharedPtr(new DiagCollectorCreator<Diag959Collector>);
  _diagCreatorMap[Diagnostic969] = createSharedPtr(new DiagCollectorCreator<Diag969Collector>);
  _diagCreatorMap[Diagnostic970] = createSharedPtr(new DiagCollectorCreator<Diag970Collector>);
  _diagCreatorMap[Diagnostic980] = createSharedPtr(new DiagCollectorCreator<Diag980Collector>);
  _diagCreatorMap[Diagnostic981] = createSharedPtr(new DiagCollectorCreator<Diag981Collector>);
  _diagCreatorMap[Diagnostic982] = createSharedPtr(new DiagCollectorCreator<Diag982Collector>);
  _diagCreatorMap[Diagnostic983] = createSharedPtr(new DiagCollectorCreator<Diag983Collector>);
  _diagCreatorMap[Diagnostic986] = createSharedPtr(new DiagCollectorCreator<Diag986Collector>);
  _diagCreatorMap[Diagnostic988] = createSharedPtr(new DiagCollectorCreator<Diag988Collector>);
  _diagCreatorMap[Diagnostic989] = createSharedPtr(new DiagCollectorCreator<Diag989Collector>);
  _diagCreatorMap[Diagnostic990] = createSharedPtr(new DiagCollectorCreator<Diag990Collector>);
  _diagCreatorMap[Diagnostic993] = createSharedPtr(new DiagCollectorCreator<Diag993Collector>);
  _diagCreatorMap[Diagnostic994] = createSharedPtr(new DiagCollectorCreator<Diag994Collector>);
  _diagCreatorMap[Diagnostic996] = createSharedPtr(new DiagCollectorCreator<Diag996Collector>);
  _diagCreatorMap[Diagnostic997] = createSharedPtr(new DiagCollectorCreator<Diag997Collector>);
  _diagCreatorMap[Diagnostic942] = createSharedPtr(new DiagCollectorCreator<Diag942Collector>);
  _diagCreatorMap[Diagnostic24] = createSharedPtr(new DiagCollectorCreator<Diag804Collector>);
  _diagCreatorMap[LegacyTaxDiagnostic24] = createSharedPtr(new DiagCollectorCreator<Diag804Collector>);
}

DiagCollector*
DCFactory::threadCreate(Trx& trx, Diagnostic& root)
{
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if(diagType == DiagnosticNone)
  {
    return thrDisabledDC(trx);
  }
  DiagnosticCreatorMap::const_iterator it = _diagCreatorMap.find(diagType);
  if(it != _diagCreatorMap.end())
  {
    return it->second->createDiagCollector(trx, root, diagType);
  }
  else
  {
    DiagCollectorCreator<DiagCollector> defaultCollectorCreator;
    return defaultCollectorCreator.createDiagCollector(trx, root, diagType);
  }
}

DCFactory*
DCFactory::instance()
{
  if (!_instance)
  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (!_instance)
    {
      _instance = new DCFactory;
    }
  }
  return _instance;
}

bool
DCFactory::createThreadDiag(Trx& trx, const pthread_t pid)
{
  if (trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    pthread_t pid = pthread_self();
    boost::lock_guard<boost::mutex> g(trx.mutexDisabledDC());
    Trx::TidDisabledDCMap::iterator it = trx.tidDisabledDCMap().find(pid);
    if (it == trx.tidDisabledDCMap().end())
    {
      DiagCollector* dc = nullptr;
      trx.dataHandle().get(dc);
      dc->rootDiag() = &trx.diagnostic();
      dc->trx() = &trx;
      trx.tidDisabledDCMap().insert(Trx::TidDisabledDCMap::value_type(pid, dc));
    }
  }
  else
  {
    boost::lock_guard<boost::mutex> g(_mutexThreadDiag);

    //  this is the root process, save trx.diagnostic into PidThreadMap
    ThreadDiagInfo* tdInfo = nullptr;
    trx.dataHandle().get(tdInfo);
    tdInfo->diag = &trx.diagnostic();

    // save into PidThreadMap
    _pidDiagMap.insert(PidDiagMap::value_type(pid, tdInfo));
  }
  return true;
}

bool
DCFactory::createThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid)
{
  if (LIKELY(trx.diagnostic().diagnosticType() == DiagnosticNone))
  {
    boost::lock_guard<boost::mutex> g(trx.mutexDisabledDC());
    Trx::TidDisabledDCMap::iterator it = trx.tidDisabledDCMap().find(pid);
    if (it == trx.tidDisabledDCMap().end())
    {
      DiagCollector* dc = nullptr;
      trx.dataHandle().get(dc);
      dc->rootDiag() = &trx.diagnostic();
      dc->trx() = &trx;
      trx.tidDisabledDCMap().insert(Trx::TidDisabledDCMap::value_type(pid, dc));
    }
  }
  else
  {
    // Mutax lock because accessing the static data member
    boost::lock_guard<boost::mutex> g(_mutexThreadDiag);

    ThreadDiagInfo* tdInfo = nullptr;
    trx.dataHandle().get(tdInfo);

    Diagnostic* myDiag = nullptr;
    trx.dataHandle().get(myDiag);

    Diagnostic& parentDiag = threadDiag(trx, ppid);

    // inherent some data member from parent
    if (parentDiag.isActive())
    {
      myDiag->activate();
    }
    else
    {
      myDiag->deActivate();
    }
    myDiag->diagnosticType() = parentDiag.diagnosticType();
    myDiag->diagParamMap() = parentDiag.diagParamMap();

    tdInfo->diag = myDiag;

    _pidDiagMap.insert(PidDiagMap::value_type(pid, tdInfo));
  }
  return true;
}

Diagnostic&
DCFactory::threadDiag(Trx& trx, const pthread_t pid)
{
  if (trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    return trx.diagnostic();
  }
  else
  {
    PidDiagMapCI pdMapCI = _pidDiagMap.find(pid);
    if (pdMapCI == _pidDiagMap.end())
    {
      // not found, this is the root process, using diagnostic for the trx
      return trx.diagnostic();
    }
    return (*(pdMapCI->second->diag));
  }
}

DiagCollector*
DCFactory::thrDisabledDC(Trx& trx)
{
  pthread_t pid = pthread_self();
  {
    boost::lock_guard<boost::mutex> g(trx.mutexDisabledDC());
    Trx::TidDisabledDCMap::iterator it = trx.tidDisabledDCMap().find(pid);
    if (it != trx.tidDisabledDCMap().end())
    {
      return it->second;
    }
  }

  const bool res = createThreadDiag(trx, pid);

  // there is no known way createThreadDiag can fail for a disabled
  // diagnostic. If there is later a way it can fail, this is probably
  // a critical error for the transaction, and so an exception might
  // be thrown.
  TSE_ASSERT(res);

  boost::lock_guard<boost::mutex> g(trx.mutexDisabledDC());
  TSE_ASSERT(trx.tidDisabledDCMap().count(pid));
  return trx.tidDisabledDCMap().find(pid)->second;
}

DCFactory::ThreadDiagInfo*
DCFactory::threadDiagInfo(const pthread_t pid)
{
  PidDiagMapCI pdMapCI = _pidDiagMap.find(pid);
  if (pdMapCI != _pidDiagMap.end())
  {
    return (pdMapCI->second);
  }
  return nullptr;
}

bool
DCFactory::endThreadDiag(Trx& trx, const pthread_t pid)
{
  if (trx.diagnostic().diagnosticType() != DiagnosticNone)
  {
    boost::lock_guard<boost::mutex> g(_mutexThreadDiag);
    _pidDiagMap.erase(pid);
  }
  return true;
}

bool
DCFactory::endThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid)
{
  if (LIKELY(trx.diagnostic().diagnosticType() == DiagnosticNone))
  {
    return true;
  }

  boost::lock_guard<boost::mutex> g(_mutexThreadDiag);

  ThreadDiagInfo* tDiagInfo = threadDiagInfo(pid);
  if (!tDiagInfo || (!tDiagInfo->diag))
  {
    return false;
  }

  Diagnostic& myDiag = *tDiagInfo->diag;
  Diagnostic& parentDiag = threadDiag(trx, ppid);

  std::string myMsg = myDiag.toString();
  parentDiag.insertDiagMsg(myMsg);

  _pidDiagMap.erase(pid);

  return true;
}
}
