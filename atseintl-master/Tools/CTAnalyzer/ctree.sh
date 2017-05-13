#!/bin/bash

RELEASE=debug
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/lib/$RELEASE
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/Tools/Diagnostic/lib
LD_LIBRARY_PATH=/login/sw919740/FFC/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=/opt/log4cxx/lib:$LD_LIBRARY_PATH

#/usr/local/bin/valgrind --skin=addrcheck --leak-check=yes ./tseserver -D TO_MAN.IOR_FILE=$IOR $*
#/usr/local/bin/valgrind --skin=memcheck --leak-check=yes ./tseserver -D TO_MAN.IOR_FILE=$IOR $*
#/usr/local/bin/valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./tseserver -D TO_MAN.IOR_FILE=$IOR $*
#/usr/local/bin/valgrind --skin=calltree --zero-before=TransactionOrchestrator::process --dump-after=TransactionOrchestrator::process ./tseserver -D TO_MAN.IOR_FILE=$IOR $*
./CallTreeAnalyzer $*
