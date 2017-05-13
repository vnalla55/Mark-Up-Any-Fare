#!/usr/bin/perl

sub dashifyList
{
   my(@list) = @_;
   my $result;
 
   my $size=@list;
   return $result if ($size == 0);

   my $consecutive=0;
   my $result = $list[0];

   for (my $i=1; $i < $size; $i++)
   {
       my $prefix1=substr($list[$i-1], 0, 5);
       my $prefix2=substr($list[$i], 0, 5);
       my $prefix3=substr($list[$i+1], 0, 5);
       my $num1=substr($list[$i-1], 5, 3);
       my $num2=substr($list[$i], 5, 3);
       my $num3=substr($list[$i+1], 5, 3);

       if ( ($prefix1 eq $prefix2) && ($num1 == $num2-1) && 
            (($i != $size-1) && $prefix2 eq $prefix3) && ($num2 == $num3-1) )
       {
           $result .= "-" if ($consecutive == 0);
           $consecutive = 1;
       }
       else
       {
           $result .= " " if ($consecutive == 0);
           $result .= $list[$i];
           $consecutive = 0;
       }
    }
    
    return $result;
}

1
