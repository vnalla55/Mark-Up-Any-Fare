//----------------------------------------------------------------------------
//  File:        Diag877Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 880 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2010
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

#pragma once

#include "Common/TseUtil.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class TravelSeg;
class Money;

class Diag880Collector : public DiagCollector
{
  friend class Diag880CollectorTest;

  typedef std::pair<std::vector<TravelSeg*>::const_iterator,
                    std::vector<TravelSeg*>::const_iterator> TvlSegPair;

public:
  explicit Diag880Collector(Diagnostic& root)
    : DiagCollector(root), _filteredPass(true), _filteredFMPass(true), _itinMultiTktNbr(0)
  {
  }
  Diag880Collector() : _filteredPass(true), _filteredFMPass(true), _itinMultiTktNbr(0) {}

  void printHeader(const ServiceGroup groupCode,
                   ServiceSubTypeCode subCode,
                   PaxTypeCode paxTypeCode,
                   std::vector<TravelSeg*>::const_iterator first,
                   const std::vector<TravelSeg*>::const_iterator end,
                   uint16_t _itinMultiTktNbr);
  void printSliceAndDiceMatrix(TseUtil::SolutionSet& solutions, uint32_t maxDiagSize);
  void printSolution(const std::vector<TvlSegPair*>& solution, uint16_t counter);
  void printPiece(const TvlSegPair& route,
                  Money feeMoney,
                  const CarrierCode& carrierCode,
                  const bool isMarketing,
                  const DateTime& tktDate);
  void printTravelAndMessage(const TvlSegPair& route, const std::string& message);
  void printPieceNotFound(const TvlSegPair& route);
  void printHardMatchFound(const TvlSegPair& route);
  void printSoftMatchFound(const TvlSegPair& route);
  void printResult(bool isWinning);
  void printResult(Money feeMoney, bool isWinning, const DateTime& tktDate);
  void printResultFail(Money feeMoney, const DateTime& tktDate);

private:
  void printSectorsHeader(uint16_t skippedSegs);
  void printTravelRoute(std::vector<TravelSeg*>::const_iterator first,
                        const std::vector<TravelSeg*>::const_iterator end);

  bool _filteredPass;
  bool _filteredFMPass;
  uint16_t _itinMultiTktNbr;
};
} // namespace tse

