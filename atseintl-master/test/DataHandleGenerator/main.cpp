#include "test/DataHandleGenerator/Config.h"
#include "test/DataHandleGenerator/DHGenerator.h"

int
main(int argc, char* argv[])
{
  Config cfg;
  if (!cfg.read(argc, argv))
    return -1;

  DHGenerator generator(cfg);
  if (!generator.generateMock())
    return -1;

  return 0;
};
