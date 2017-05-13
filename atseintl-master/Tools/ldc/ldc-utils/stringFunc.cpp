#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <string>
#include <vector>

void
stringTok(const std::string& input, std::vector<std::string>& output, const std::string& delimiter)
{
  output.clear();

  size_t len = input.length();
  size_t pos = 0;
  size_t endPos;

  char delim = delimiter[0];

  for (;;)
  {
    endPos = input.find(delim, pos);

    // no more delimiters - add remainder of string
    if (endPos == std::string::npos)
    {
      output.push_back(input.substr(pos));
      break;
    }

    // found delimiter - add up to it
    else
    {
      output.push_back(input.substr(pos, endPos - pos));
      pos = endPos + 1;
      if (pos == len)
      {
        output.push_back("");
        break;
      }
    }
  }
  return;
}

void
stringUpper(std::string& input)
{
  size_t len = input.length();

  for (size_t idx = 0; idx < len; ++idx)
  {
    input[idx] = toupper(input[idx]);
  }
}

void
dumpHexData(const void* data, int size, bool zero, FILE* fp)
{
  unsigned char* ptr = (unsigned char*)data;
  unsigned char* addr = zero ? nullptr : ptr;

  int index = 0;

  for (index = 0; index < size; index += 16)
  {
    int i;

    fprintf(fp, "  %10p  ", addr + index);

    int mid = 0;
    for (i = index; i < index + 16; ++i)
    {
      if (i < size)
      {
        fprintf(fp, "%2.2X ", ptr[i]);
      }
      else
      {
        fprintf(fp, "   ");
      }

      if (++mid == 8)
        fprintf(fp, "  ");
    }
    for (i = index; i < index + 16; ++i)
    {
      if (i < size)
      {
        char chr = ptr[i];

        if (chr >= 0x20)
        {
          fprintf(fp, "%c", chr);
        }
        else
        {
          fprintf(fp, ".");
        }
      }
    }
    fprintf(fp, "\n");
  }
  fflush(fp);
}
