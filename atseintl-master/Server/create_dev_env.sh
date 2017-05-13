#!/bin/bash

#Purpose of this script is to create all elements necessary to start server.
#It basically means to create links to cfg/ini/sh files

[ $# -eq 0 ] && exit 0

GLOBAL_DIR=$1

ln -sf ../../deploy/apps/tseshared/appmon_hook.sh -t ${GLOBAL_DIR}
ln -sf ../../deploy/apps/tseshared/checkhung.sh   -t ${GLOBAL_DIR}
ln -sf ../../deploy/apps/tseshared/initovars.sh   -t ${GLOBAL_DIR}
ln -sf ../../deploy/apps/tseshared/cfgshell_functions.sh -t ${GLOBAL_DIR}

ln -sf ../../Server/faredisplay_server.sh         -t ${GLOBAL_DIR}
ln -sf ../../Server/historical_server.sh          -t ${GLOBAL_DIR}
ln -sf ../../Server/pricing_server.sh             -t ${GLOBAL_DIR}
ln -sf ../../Server/prof-server.sh                -t ${GLOBAL_DIR}
ln -sf ../../Server/pserver.sh                    -t ${GLOBAL_DIR}
ln -sf ../../Server/pserver_shopping.sh           -t ${GLOBAL_DIR}
ln -sf ../../Server/server.sh                     -t ${GLOBAL_DIR}
ln -sf ../../Server/serverje.sh                   -t ${GLOBAL_DIR}
ln -sf ../../Server/servertbb.sh                  -t ${GLOBAL_DIR}
ln -sf ../../Server/set.atse.common.sh            -t ${GLOBAL_DIR}
ln -sf ../../Server/set_ld_path.sh                -t ${GLOBAL_DIR}
ln -sf ../../Server/setpath.sh                    -t ${GLOBAL_DIR}
ln -sf ../../Server/shopping_server.sh            -t ${GLOBAL_DIR}
ln -sf ../../Server/shoppingesv_server.sh         -t ${GLOBAL_DIR}
ln -sf ../../Server/shoppinghist_server.sh        -t ${GLOBAL_DIR}
ln -sf ../../Server/shoppingis_server.sh          -t ${GLOBAL_DIR}
ln -sf ../../Server/acms_server.sh                -t ${GLOBAL_DIR}
ln -sf ../../Server/tax_server.sh                 -t ${GLOBAL_DIR}
ln -sf ../../Server/totalview.sh                  -t ${GLOBAL_DIR}
ln -sf ../../Server/vserver.sh                    -t ${GLOBAL_DIR}
ln -sf ../../Server/tv.sh                         -t ${GLOBAL_DIR}

ln -sf ../../Server/appmon_log                    -t ${GLOBAL_DIR}
ln -sf ../../Server/dbaccess.esv.ini              -t ${GLOBAL_DIR}
ln -sf ../../Server/asap.ini                      -t ${GLOBAL_DIR}
ln -sf ../../Server/dbaccess.ini                  -t ${GLOBAL_DIR}
ln -sf ../../Server/dbaccess.ini                  ${GLOBAL_DIR}/dbaccess-ora.ini
ln -sf ../../Server/tsi.ini                       -t ${GLOBAL_DIR}
ln -sf ../../Server/log4cxx.xml                   -t ${GLOBAL_DIR}
ln -sf ../../Server/cacheNotify.xml               -t ${GLOBAL_DIR}
ln -sf ../../Server/log4j.dtd                     -t ${GLOBAL_DIR}
ln -sf ../../Server/sqlnet.ora                    -t ${GLOBAL_DIR}
ln -sf ../../Server/tnsnames.ora                  -t ${GLOBAL_DIR}
ln -sf ../../Server/tseserverje                   -t ${GLOBAL_DIR}
ln -sf ../../Server/tseservertbb                  -t ${GLOBAL_DIR}
ln -sf ../../Server/VIS_Beta.txt                  -t ${GLOBAL_DIR}

ln -sf ../../Xform/xmlConfig.cfg                  -t ${GLOBAL_DIR}
ln -sf ../../Xform/currencyRequest.cfg            -t ${GLOBAL_DIR}
ln -sf ../../Xform/detailXmlConfig.cfg            -t ${GLOBAL_DIR}
ln -sf ../../Xform/fareDisplayRequest.cfg         -t ${GLOBAL_DIR}
ln -sf ../../Xform/mileageRequest.cfg             -t ${GLOBAL_DIR}
ln -sf ../../Xform/pfcDisplayRequest.cfg          -t ${GLOBAL_DIR}
ln -sf ../../Xform/pricingDetailRequest.cfg       -t ${GLOBAL_DIR}
ln -sf ../../Xform/pricingDisplayRequest.cfg      -t ${GLOBAL_DIR}
ln -sf ../../Xform/pricingRequest.cfg             -t ${GLOBAL_DIR}
ln -sf ../../Xform/taxDisplayRequest.cfg          -t ${GLOBAL_DIR}
ln -sf ../../Xform/taxInfoRequest.cfg             -t ${GLOBAL_DIR}
ln -sf ../../Xform/taxOTARequest.cfg              -t ${GLOBAL_DIR}
ln -sf ../../Xform/taxRequest.cfg                 -t ${GLOBAL_DIR}
