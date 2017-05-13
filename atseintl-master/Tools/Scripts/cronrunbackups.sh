#!/bin/bash --login

#touch /root/backups/last_run_sh
cd /root/backups
#date >> /root/backups/last_run_sh
#env >> /root/backups/last_run_sh
bin/runbackups.pl  >> /root/backups/last_run_sh
