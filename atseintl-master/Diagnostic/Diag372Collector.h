#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FarePath;
class SurfaceSectorExemptionInfo;

class Diag372Collector : public DiagCollector
{
public:
  class DiagStream : public std::ostringstream
  {
  public:
    void addSectorExemptionInfo(int count, const SurfaceSectorExemptionInfo& info);

  private:
    std::string addDate(const DateTime& date)
    {
      return (date.isValid()) ? date.dateToString(DDMMMYYYY, "") : empty;
    }

    template <class T>
    void addSet(const std::set<T>& inputSet, bool except, size_t margin);

    void addDates(const DateTime& first, const DateTime& last)
    {
      (*this) << addDate(first) + "-" + addDate(last);
    }

    static const std::string empty;
    static const size_t lineLength;
  };

  using DiagCollector::operator<<;
  virtual Diag372Collector& operator<<(const FarePath& farePath) override;

  Diag372Collector& operator<<(const DiagStream& diagStream)
  {
    (*this) << diagStream.str();
    return *this;
  }
};
}

