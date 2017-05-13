#!/bin/bash

for (( i = 0 ; i <= 1000; i++ ))
do
  echo "##     iteration $i"
  echo "./appconsole.pl ltxl0247 5432 RCENABLE ltxl0247/30443"
  date;./appconsole.pl ltxl0247 5432 RCENABLE ltxl0247/30443;date
  echo "./appconsole.pl ltxl0246 5432 RCENABLE ltxl0247/30443"
  ./appconsole.pl ltxl0246 5432 RCENABLE ltxl0247/30443;date
  echo sleep 40
  sleep 40
  echo "./appconsole.pl ltxl0247 5432 RCDISABLE"
  date;./appconsole.pl ltxl0247 5432 RCDISABLE;date
  echo "./appconsole.pl ltxl0246 5432 RCDISABLE"
  ./appconsole.pl ltxl0246 5432 RCDISABLE;date
  echo sleep 5
  sleep 5
  echo "./appconsole.pl ltxl0247 5432 RCENABLE ltxl0246/30443"
  date;./appconsole.pl ltxl0247 5432 RCENABLE ltxl0246/30443;date
  echo "./appconsole.pl ltxl0246 5432 RCENABLE ltxl0246/30443"
  ./appconsole.pl ltxl0246 5432 RCENABLE ltxl0246/30443;date
  echo sleep 40
  sleep 40
  echo "./appconsole.pl ltxl0247 5432 RCDISABLE"
  date;./appconsole.pl ltxl0247 5432 RCDISABLE;date
  echo "./appconsole.pl ltxl0246 5432 RCDISABLE"
  ./appconsole.pl ltxl0246 5432 RCDISABLE;date
done
