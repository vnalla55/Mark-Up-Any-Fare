//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/Shopping/IBF/IbfDiag910Collector.h"

#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <iomanip>

using namespace std;

namespace tse
{

using namespace utils;

SopCombinationListFormatter::SopCombinationListFormatter(ShoppingTrx& trx)
  : _trx(trx), _solutionNbrWidth(8), _numericFieldWidth(5), _locationWidth(25)
{
}

void
SopCombinationListFormatter::addSopCombination(const utils::SopCombination& combination,
                                               const std::string& comment)
{
  _list.push_back(std::make_pair(combination, comment));
}

void
SopCombinationListFormatter::flush(std::ostream& out)
{
  if (_list.size() > 0)
  {
    printHeader(out);
    for (size_t i = 0; i < _list.size(); ++i)
    {
      dumpSopCombination(out, _list[i], i);
    }
    out << "Total " << getNbrOfCombinations() << " combinations." << std::endl;
  }
  else
  {
    out << "NO SOP COMBINATIONS" << endl;
  }
}

void
SopCombinationListFormatter::printHeader(std::ostream& out) const
{
  out << left << setw(_solutionNbrWidth) << "Solution";
  out << setw(_numericFieldWidth) << " Leg";
  out << setw(_numericFieldWidth) << " Sop";
  out << setw(_numericFieldWidth) << " Cxr";
  out << setw(_locationWidth) << " From";
  out << setw(_locationWidth) << " To";
  if (_commentMetatext.size() > 0)
  {
    out << left << " " << _commentMetatext;
  }
  out << endl;

  for (size_t i = 0; i < getWidth(); ++i)
  {
    out << "-";
  }
  out << endl;
}

void
SopCombinationListFormatter::dumpSopCombination(std::ostream& out,
                                                const CommentedCombination& c,
                                                int index) const
{
  out << right;
  for (unsigned int legId = 0; legId < c.first.size(); ++legId)
  {
    out << setw(_solutionNbrWidth) << (index + 1);

    const int sopId = c.first[legId];
    out << setw(_numericFieldWidth - 1) << legId << " ";
    out << setw(_numericFieldWidth - 1) << sopId << " ";

    const ShoppingTrx::SchedulingOption& sop = findSopInTrx(legId, sopId, _trx);
    out << setw(_numericFieldWidth - 1) << sop.governingCarrier() << " ";

    const Itin* itin = sop.itin();
    TSE_ASSERT(itin != nullptr);
    for (const auto ts : itin->travelSeg())
    {
      out << formatTravelSegment(*ts);
    }

    if ((0 == legId) && (c.second.size() > 0))
    {
      out << " " << c.second;
    }

    out << std::endl;
  }
  out << std::endl;
}

std::string
SopCombinationListFormatter::formatTravelSegment(const TravelSeg& segment) const
{
  std::ostringstream tmp;
  tmp << " " << segment.origin()->loc() << " " << segment.departureDT();

  std::ostringstream out;
  out << left << setw(_locationWidth) << tmp.str();

  tmp.str("");
  tmp << " " << segment.destination()->loc() << " " << segment.arrivalDT();
  out << left << setw(_locationWidth) << tmp.str();
  return out.str();
}

IbfDiag910Collector::IbfDiag910Collector(ShoppingTrx& trx, const std::string& customHeaderMessage)
  : _trx(trx),
    _diag(trx, Diagnostic910),
    _fos(trx),
    _failedSopsPresent(false),
    _failedCombinations(trx),
    _customHeaderMessage(customHeaderMessage),
    _shouldPrintFailedSops(true)
{
  _areDetailsEnabled =
      (_diag.isActive() &&
       (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "DETAILS"));
  _failedCombinations.setCommentMetatext("Reason");
}

void
IbfDiag910Collector::elementInvalid(const utils::SopCandidate& candid,
                                    const INamedPredicate<SopCandidate>& failedPredicate)
{
  if (!_areDetailsEnabled)
  {
    return;
  }
  _failedSopsPresent = true;
  const ShoppingTrx::SchedulingOption& sop = utils::findSopInTrx(candid.legId, candid.sopId, _trx);
  _failedSopStream << sop;
  _failedSopStream << " " << failedPredicate.getName() << endl;
}

void
IbfDiag910Collector::elementInvalid(const SopCombination& combination,
                                    const INamedPredicate<SopCombination>& failedPredicate)
{
  if (!_areDetailsEnabled)
  {
    return;
  }

  _failedCombinations.addSopCombination(combination, failedPredicate.getName());
}

void
IbfDiag910Collector::collect(const utils::SopCombination& c)
{
  if (!_diag.isActive())
  {
    return;
  }
  _fos.addSopCombination(c);
}

void
IbfDiag910Collector::flush()
{
  if (!_diag.isActive())
  {
    return;
  }

  std::ostringstream out;
  if (_customHeaderMessage.size() != 0)
  {
    out << _customHeaderMessage << "\n";
  }

  if (_areDetailsEnabled)
  {
    if (_shouldPrintFailedSops)
    {
      out << "List of SOPs failed for IBF FOS generation\n";
      if (_failedSopsPresent)
      {
        out << setw(54) << left << "SOP";
        out << " Reason" << endl;
        out << "-------------------------------------------------------------\n";
        out << _failedSopStream.str();
      }
      else
      {
        out << "NO SUCH SOPS" << endl;
      }
      out << endl;

      printSopPerLegInfo(out);
      out << endl;
    }

    out << "List of failed SOP combinations" << endl;
    _failedCombinations.flush(out);
    out << endl;
  }

  out << "The list of Flight-Only Solutions for IBF: START\n";
  out << "------------------------------------------------\n";
  _fos.flush(out);
  out << endl;
  out << "The list of Flight-Only Solutions for IBF: END\n";
  out << "------------------------------------------------\n";
  _diag << out.str();
}

void
IbfDiag910Collector::printSopPerLegInfo(std::ostream& out) const
{
  out << "Number of SOPs we use to generate FOS solutions:" << endl;

  ostringstream upper;
  ostringstream lower;

  upper << "Leg id      ";
  lower << "Nbr of SOPs ";

  for (const auto& elem : _sopsPerLeg)
  {
    upper << setw(5) << elem.first;
    lower << setw(5) << elem.second;
  }
  out << upper.str() << endl;
  out << lower.str() << endl;
}

} // namespace tse
