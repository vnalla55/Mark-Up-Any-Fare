//------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/AborterTasks.h"

#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"

namespace tse
{
template <>
void
AborterTask<AborterTaskMode::ABORT>::run()
{
  _aborter->forEachTrx([](Trx* trx) { trx->setTimeout(); });
}

template <>
void
AborterTask<AborterTaskMode::HURRY>::run()
{
  _aborter->forEachTrx([](Trx* trx) { trx->setHurry(); });
}

template <>
void
AborterTask<AborterTaskMode::HURRY_COND>::run()
{
  _aborter->forEachTrx([](Trx* trx) { trx->setHurryWithCond(); });
}

template <>
void
AborterTask<AborterTaskMode::ABORT>::resetFlag()
{
  _aborter->forEachTrx([](Trx* trx) { trx->resetTimeoutFlag(); });
}

template <>
void
AborterTask<AborterTaskMode::HURRY>::resetFlag()
{
  _aborter->forEachTrx([](Trx* trx) { trx->resetHurryFlag(); });
}

template <>
void
AborterTask<AborterTaskMode::HURRY_COND>::resetFlag()
{
  _aborter->forEachTrx([](Trx* trx) { trx->resetHurryWithCondFlag(); });
}
}
