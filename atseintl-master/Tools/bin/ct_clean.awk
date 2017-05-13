
BEGIN {
	month_name [  1 ] = "Jan"
	month_name [  2 ] = "Feb"
	month_name [  3 ] = "Mar"
	month_name [  4 ] = "Apr"
	month_name [  5 ] = "May"
	month_name [  6 ] = "Jun"
	month_name [  7 ] = "Jul"
	month_name [  8 ] = "Aug"
	month_name [  9 ] = "Sep"
	month_name [ 10 ] = "Oct"
	month_name [ 11 ] = "Nov"
	month_name [ 12 ] = "Dec"

	day_count [  1 ] = 31
	day_count [  2 ] = 28
	day_count [  3 ] = 31
	day_count [  4 ] = 30
	day_count [  5 ] = 31
	day_count [  6 ] = 30
	day_count [  7 ] = 31
	day_count [  8 ] = 31
	day_count [  9 ] = 30
	day_count [ 10 ] = 31
	day_count [ 11 ] = 30
	day_count [ 12 ] = 31

	line_type = 0
	line_count = 0
}

{
# First record
	if ( line_type == 0 )
	{
		line_type = 1

		days_gone = DAY
		for ( i = 1 ; i < MONTH ; i++ )
			days_gone = days_gone + day_count [ i ]
	}

# File name record
	if ( line_type == 1 )
	{
		line_type = 2

		file_name = $1
		num_elem = split ( file_name, parts, "." )
		if ( num_elem > 1 )
			file_type = parts [ num_elem ]
		else
			file_type = ""
		next
	}

# Age record	
	line_type = 1

	# Ignore .cmake.state
	if ( file_type == "stat" )
		next 

	file_day = substr ( $2, 1, 2 )

	file_year = substr ( $2, 8, 2 )
	if ( file_year > 50 )
		file_year = file_year + 1900
	else
		file_year = file_year + 2000

	days = days_gone

	if ( file_year == YEAR )
	{
		for ( i = 1 ; i <= 12 ; i++ )
		{
			if ( substr ( $2, 4, 3 ) == month_name [ i ] )
				break 

			days = days - day_count [ i ]
		}

		days = days - file_day 
	}
	else
	{
		for ( i = 12 ; i > 1 ; i-- )
		{
			days = days + day_count [ i ] 

			if ( substr ( $2, 4, 3 ) == month_name [ i ] )
				break 
		}

		days = days - file_day 
	}
	
	file_month = i

	if ( days >= AGE )
	{
		printf ( "     Age=%03d %02d/%02d/%04d %s\n", days, file_month, file_day, file_year, file_name ) 

		line_count++
	}

}
	
END {
	if ( line_count == 0 )
	{
		print "    --- No Files ---"
		exit 1
	}
}
	
