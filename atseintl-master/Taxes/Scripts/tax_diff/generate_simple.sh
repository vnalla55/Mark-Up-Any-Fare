#!/bin/bash
if [[ "$0" != "igawk" ]] ; then exec igawk --file <(sed '2s/./#/' "$0") "$@"; fi; exit
BEGIN {
  FS=","

@include comments.awk

}

/Trx No/ {
  trx_no++;
}

/^ / {
  gsub(/ /, "", $0)
  sum[$1] += $4
  count[$1]++
  if ($4 == 0)
    count_0[$1]++
  else if ($4 < 10)
    count_10[$1]++
  else if ($4 < 100)
    count_99[$1]++
  else if ($4 == 100)
    count_100[$1]++
}

END {
  for (i in sum)  {
    sum_all += sum[i]/count[i]
    count_all++
    weight_sum_all += sum[i]
    weight_count_all += count[i]
    if (sum[i]/count[i] == 0)
    {
      taxes_0++
    }
    else if (sum[i]/count[i] < 10)
    {
      taxes_10++
    }
    else if (sum[i]/count[i] < 100)
    {
      taxes_99++
    }
    else
    {
      taxes_100++
    }
  }

  print "Number of requests: "trx_no
  print "Number of taxes: "count_all
  printf("Taxes calculated correctly: %d/%d = %f%\n", taxes_0, count_all, taxes_0/count_all*100)
  
  print "Average difference,"sum_all/count_all"%"
  print "Weighted average difference,"weight_sum_all/weight_count_all"%"
  print ""
  print "\"0% difference\","taxes_0/count_all*100"%"
  print "\"<0,10)% difference\","taxes_10/count_all*100"%"
  print "\"<10,100)% difference\","taxes_99/count_all*100"%"
  print "\"100% difference\","taxes_100/count_all*100"%"
  print ""

  sort = "sort -n -k2"
  printf("%5s %11s   %9s\n", "Tax", "Correctly", "Comment")
  for (i in sum)  {
    printf ("%4s:%11f%%   %9s\n", i, count_0[i]/count[i]*100, comment[i]) | sort
  }
  close(sort)
}
