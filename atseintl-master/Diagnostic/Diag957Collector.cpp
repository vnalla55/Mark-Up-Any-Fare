//----------------------------------------------------------------------------
//  File:        Diag956Collector.C
//
//  Copyright Sabre 2004
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
#include "Diagnostic/Diag957Collector.h"

#include "Pricing/ESVPQ.h"
#include "Pricing/ESVPQItem.h"

namespace tse
{
Diag957Collector&
Diag957Collector::displayHeader()
{
  if (false == _active)
  {
    return (*this);
  }

  *this << "\n";
  *this << "**********************************************************\n";
  *this << "957: ESV FINAL DIVERSITY RESULTS";
  *this << "**********************************************************\n";
  *this << "\n";

  const ShoppingTrx& sTrx = dynamic_cast<const ShoppingTrx&>(*_trx);
  if (sTrx.legs().empty())
  {
    *this << "ERROR: "
          << "LEGS VECTOR IS EMPTY FOR CURRENT SHOPPING TRANSACTION\n";
  }
  return (*this);
}

Diag957Collector&
Diag957Collector::displaySolutions(const std::vector<ESVPQItem*>& pqItemVec)
{
  std::vector<ESVPQItem*>::const_iterator pqIter;

  if (false == _active)
  {
    return (*this);
  }

  std::map<CarrierCode, std::vector<ESVPQItem*> > cxrItemMap;
  std::vector<ESVPQItem*> nonCxrItemVec;

  for (pqIter = pqItemVec.begin(); pqIter != pqItemVec.end(); pqIter++)
  {
    if ((*pqIter) == nullptr)
    {
      continue;
    }
    if ((*pqIter)->outSopWrapper()->sop() == nullptr)
    {
      continue;
    }

    CarrierCode cxr = (*pqIter)->outSopWrapper()->sop()->governingCarrier();
    QueueType qType = (*pqIter)->queueType();

    if ((qType == MPNonstopOnline) || (qType == MPOutNonstopOnline) ||
        (qType == MPNonstopInterline) || (qType == MPOutNonstopInterline) ||
        (qType == MPSingleStopOnline))
    {
      cxrItemMap[cxr].push_back(*pqIter);
    }
    else
    {
      nonCxrItemVec.push_back(*pqIter);
    }
  }

  std::map<CarrierCode, std::vector<ESVPQItem*> >::iterator it;

  for (it = cxrItemMap.begin(); it != cxrItemMap.end(); it++)
  {
    *this << "\n";
    *this << "**********************************************************\n";
    *this << "SOLUTIONS FOR OUTBOUND CARRIER " << it->first << "\n";
    *this << "**********************************************************\n";

    clasifyAndDisplaySolutions(it->second, it->first);
  }

  // non carrier related part

  clasifyAndDisplaySolutions(nonCxrItemVec, "");

  return (*this);
}

Diag957Collector&
Diag957Collector::clasifyAndDisplaySolutions(const std::vector<ESVPQItem*>& pqItemVec,
                                             CarrierCode carrier)
{
  std::map<QueueType, std::vector<ESVPQItem*> > itemsPerqTypeMap;
  std::vector<ESVPQItem*>::const_iterator pqIter;

  for (pqIter = pqItemVec.begin(); pqIter != pqItemVec.end(); pqIter++)
  {
    if (*pqIter == nullptr)
    {
      continue;
    }
    itemsPerqTypeMap[(*pqIter)->queueType()].push_back(*pqIter);
  }

  std::map<QueueType, std::vector<ESVPQItem*> >::iterator perqTIt;

  for (perqTIt = itemsPerqTypeMap.begin(); perqTIt != itemsPerqTypeMap.end(); perqTIt++)
  {
    displayClasifiedSolutions(ESVPQ::getQueueTitle(perqTIt->first), perqTIt->second);
  }

  *this << "\n";
  *this << "**********************************************************\n";

  if (carrier != "")
  {
    *this << "SUMMARY FOR OUTBOUND CARRIER " << carrier << "\n";
  }
  else
  {
    *this << "SUMMARY FOR NON CARRIER QUEUES\n";
  }

  *this << "**********************************************************\n";

  for (perqTIt = itemsPerqTypeMap.begin(); perqTIt != itemsPerqTypeMap.end(); perqTIt++)
  {
    *this << "NUMBER OF " << ESVPQ::getQueueTitle(perqTIt->first) << " = " << perqTIt->second.size()
          << "\n";
  }

  return (*this);
}

Diag957Collector&
Diag957Collector::displayClasifiedSolutions(std::string description,
                                            const std::vector<ESVPQItem*>& pqItemVec)
{
  if (false == _active)
  {
    return (*this);
  }

  *this << "\n";
  *this << "..........................................................\n";
  *this << description << "\n";
  *this << "..........................................................\n";

  std::vector<ESVPQItem*>::const_iterator pqIter;

  for (pqIter = pqItemVec.begin(); pqIter != pqItemVec.end(); pqIter++)
  {
    if (*pqIter == nullptr)
    {
      continue;
    }

    displayESVPQItem(*pqIter, "", false, true);
  }
  return (*this);
}
}
