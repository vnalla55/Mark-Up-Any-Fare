export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/atseintl/3rdParty/lib/libevent/lib
/opt/atseintl/3rdParty/lib/memcached/bin/memcached -vv -m 20000 -l atseitst1 -p 11211
