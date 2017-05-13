#ifndef test_ScreamVector_h
#define test_ScreamVector_h

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <boost/thread/thread.hpp>
#include <boost/tokenizer.hpp>
#include <cxxabi.h>

#include "Util/StackUtil.h"

namespace tse
{

template <typename T>
struct ScreamPolicy
{
  static const int StackTraceDepth = 0;
  static const bool StackTraceDemangle = false;
  static const bool ScreamThread = false;
  static const bool ScreamOnPushBack = false;
  static const bool ScreamOnPopBack = false;
  static const bool ScreamOnAssignment = false;

  void scream(const std::string& output)
  {
    std::cerr << output;
  }

  void print(std::ostream& os, const T& t)
  {
    // You may need to override this default implementation.
    os << t;
  }
};

template <typename T>
struct VerboseScreamPolicy : ScreamPolicy<T>
{
  static const int StackTraceDepth = 10;
  static const bool StackTraceDemangle = true;
  static const bool ScreamThread = true;
  static const bool ScreamOnPushBack = true;
  static const bool ScreamOnPopBack = true;
  static const bool ScreamOnAssignment = true;
};

template <typename T, typename Policy = ScreamPolicy<T>,
          typename Allocator = std::allocator<T> >
class ScreamVector : public std::vector<T, Allocator>
{
  typedef std::vector<T, Allocator> Vector;

public:
  void push_back(const T& t)
  {
    if (policy.ScreamOnPushBack)
      scream_push_back(t);

    Vector::push_back(t);
  }

  void pop_back()
  {
    assert(!this->empty());

    if (policy.ScreamOnPopBack)
      scream_pop_back(this->back());

    Vector::pop_back();
  }

  ScreamVector& operator=(const Vector& V)
  {
    if (policy.ScreamOnAssignment)
      scream_assign(V);

    Vector::operator=(V);
    return *this;
  }

  //TODO more methods should scream.

private:
  Policy policy;

  __attribute__((always_inline))
  void scream_thread(std::ostream& os)
  {
    os << "[Thread " << boost::this_thread::get_id() << "]\n";
  }

  __attribute__((always_inline))
  void scream_demangle(std::string& trace_line)
  {
    const size_t left = trace_line.find('(');
    if (left == std::string::npos)
      return;

    size_t right = trace_line.find('+', left);
    if (right == std::string::npos)
      right = trace_line.find(')', left);
    if (right == std::string::npos)
      return;

    const char old_right = trace_line[right];
    trace_line[right] = '\0';

    int status = 0;
    char* output = __cxxabiv1::__cxa_demangle(&trace_line[left + 1], 0, 0, &status);
    trace_line[right] = old_right;

    if (!output || status)
      return;

    trace_line.replace(left + 1, right - left - 1, output);

    std::free(output);
  }

  __attribute__((always_inline))
  void scream_stack_trace(std::ostream& os, int frames)
  {
    const std::string trace = StackUtil::getStackTrace(frames + 5);

    boost::char_separator<char> newLine("\n");
    boost::tokenizer<boost::char_separator<char> > lines(trace, newLine);

    for (std::string line : lines)
    {
      if (line.find("ScreamVector") != std::string::npos ||
          line.find("StackUtil") != std::string::npos)
        continue;

      if (policy.StackTraceDemangle)
        scream_demangle(line);

      os << line << '\n';

      if (--frames == 0)
        return;
    }
  }

  __attribute__((always_inline))
  void scream_self(std::ostream& os)
  {
    os << "(*" << (void*)this << ")";
  }

  __attribute__((always_inline))
  void scream_output(std::ostringstream& oss)
  {
    if (policy.ScreamThread)
      scream_thread(oss);
    if (policy.StackTraceDepth >= 0)
      scream_stack_trace(oss, policy.StackTraceDepth);
    policy.scream(oss.str());
  }

  __attribute__((always_inline))
  void scream_push_back(const T& t)
  {
    std::ostringstream oss;

    scream_self(oss);
    oss << ".push_back(";
    policy.print(oss, t);
    oss << ");\n";

    scream_output(oss);
  }

  __attribute__((always_inline))
  void scream_pop_back(const T& t)
  {
    std::ostringstream oss;

    scream_self(oss);
    oss << ".pop_back() of ";
    policy.print(oss, t);
    oss << ";\n";

    scream_output(oss);
  }

  __attribute__((always_inline))
  void scream_assign(const Vector& V)
  {
    std::ostringstream oss;

    scream_self(oss);
    oss << " = {";
    for (const T& t : V)
    {
      policy.print(oss, t);
      oss << ", ";
    }
    oss << "};\n";

    scream_output(oss);
  }
};

}

#endif
