#include <stdio.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "../core/core.h"

#include "no_os_time.h"

static void parse_time( uint32 sec,  uint16 *year_out, uint16 *month_out, uint16 *day_out,
                        uint16 *hour_out, uint16 *minute_out, uint16 *second_out );
// sunday 0
// monday 1 tuesday 2 wednes
uint16 get_weekday( uint16 year, uint16 mon, uint16 day )
{

    uint16 week_day = 0;
    if ( ( 1 == mon ) || ( 2 == mon ) )
    {
        mon += 12;
        year--;
    }
    week_day = ( day + 1 + 2*mon + 3 * ( mon + 1 ) / 5 + year +
                 year/4 - year/100 + year/400 ) % 7;

    return week_day;
}

static uint16 mon_day_table1[13] = { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }; // -1 to 0, first element.
static uint16 mon_day_table2[13] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

void no_os_get_ymd_hms_w( uint32 secs, int8 *value )
{
    //int8 *week_str[7] = { "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };
    //int8 *week_str[7] = { "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday" };
    //int8 *week_str[7] = { "sun", "mon", "tue", "wed", "thur", "fri", "sat" };
    //int8 *week_str[7] = { "[0]", "[1]", "[2]", "[3]", "[4]", "[5]", "[6]" };
    int8 *week_str[7] = { "0", "1", "2", "3", "4", "5", "6" };
    uint32 total_secs = secs;

    //total_secs += 60 * 60 * time_zone;
    //total_secs += 28880;

    uint16 year_out, month_out, day_out, hour_out, minute_out, second_out;

    parse_time( total_secs, &year_out, &month_out, &day_out, &hour_out, &minute_out, &second_out );

    // 2012-02-02 02:02:02 19 char
    os_sprintf( value, "%04d-%02d-%02d %02d_%02d_%02d[%s]", year_out, month_out, day_out,
                            hour_out, minute_out, second_out,
                                week_str[get_weekday(year_out, month_out, day_out)] );
}

boolean no_os_sec_is_valid( uint32 secs )
{
    boolean ret = FALSE;
    uint16 year_out, month_out, day_out, hour_out, minute_out, second_out;
    parse_time( secs, &year_out, &month_out, &day_out, &hour_out, &minute_out, &second_out );

    if ( ( year_out >= 2015 ) && ( year_out <= 2025 ) )
    {
        ret = TRUE;
    }

    return ret;
}

uint32 no_os_get_day_secs( uint32 value )
{
    // value += 28880;
    // 24*3600 = 86400
    return ( value % ( 86400 ) );
}

uint16 no_os_get_weekday( uint32 value )
{
    uint16 year_out, month_out, day_out, hour_out, minute_out, second_out;
    parse_time( value, &year_out, &month_out, &day_out, &hour_out, &minute_out, &second_out );

    return get_weekday( year_out, month_out, day_out );
}

static void parse_time( uint32 sec,  uint16 *year_out, uint16 *month_out, uint16 *day_out,
                        uint16 *hour_out, uint16 *minute_out, uint16 *second_out )
{
    uint32 total_secs = sec;
    uint32 second = ( uint32 )( total_secs % 60 );
    long total_mins = ( long )( total_secs / 60 );
    uint32 minute = ( uint32 )( total_mins % 60 );
    long total_hours = total_mins / 60;
    uint32 hour = ( uint32 )( total_hours % 24 );
    uint32 total_days = ( uint32 )( total_hours / 24 );

    uint32 base_year = 1970;

    uint32 year  = base_year + total_days / 366;
    uint32 month = 1;
    uint32 day   = 1;

    uint16  diff_days;
    boolean leap_year;
    while ( 1 )
    {
        uint32 diff = ( year -  base_year ) * 365;
        diff += (year - 1) / 4 - ( base_year - 1) / 4;
        diff -= ((year - 1) / 100 - ( base_year - 1) / 100);
        diff += (year - 1) / 400 - ( base_year - 1) / 400;

        diff_days = total_days - diff;

        leap_year = ( ( ( year % 4 == 0 ) && ( year % 100 != 0 ) ) || ( year % 400 == 0 ) );
        if ( ( !leap_year && diff_days < 365 ) || ( leap_year && diff_days < 366 ) )
        {
            break;
        }
        else
        {
            year++;
        }
    }

    int32 i;
    uint16 *month_days;

    if ( diff_days >= 59 && leap_year)
    {
        month_days = mon_day_table1;
    }
    else
    {
        month_days = mon_day_table2;
    }

    for ( i = 12; i >= 1; i-- )
    {
        if ( diff_days >= (uint32)month_days[i] )
        {
            month = (uint32)i;
            day = diff_days - month_days[i] + 1;
            break;
        }
    }
    *year_out = year;
    *month_out = month;
    *day_out = day;
    *hour_out = hour;
    *minute_out = minute;
    *second_out = second;
}
