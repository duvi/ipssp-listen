#include <math.h>
#include <time.h>
#include "common.h"
#include "utils.h"
#include "talk.h"

void init_pos()
{
    struct position_pos *p_temp_pos;
    p_temp_pos = p_start_pos;

    while (p_temp_pos)
	{
	p_temp_pos->diff_sum_db = 0;
	p_temp_pos->diff_sum_mon = 0;
	p_temp_pos = p_temp_pos->next;
	}
}

void return_pos_highest()
{
    double diff_sum = 0.0; char *helyszin = "Ismeretlen";
    struct position_pos *p_temp_pos;
    p_temp_pos = p_start_pos;
    while (p_temp_pos)
	{
	if (DETAIL) fprintf(message, " POS: %10s SUM_DIFF: %2.10f SUM_MON: %i CALC: %2.4f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db, p_temp_pos->diff_sum_mon, p_temp_pos->diff_sum_db/p_temp_pos->diff_sum_mon);
	if ((p_temp_pos->diff_sum_db / p_temp_pos->diff_sum_mon) > diff_sum)
	    {
	    diff_sum = p_temp_pos->diff_sum_db / p_temp_pos->diff_sum_mon;
	    helyszin = p_temp_pos->nev;
	    coord_x = p_temp_pos->x;
	    coord_y = p_temp_pos->y;
	    }
	p_temp_pos = p_temp_pos->next;
	}
    fprintf(message, "Helyszin: %s \n", helyszin);
}

void compare(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double diff_sum = 0.0; char *helyszin = "Ismeretlen";

    int megvan = 0;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));
	    p_temp_mon = p_temp_sta->monitor;
	    do
	        {
//	        fprintf(message, " MONITOR: %s SIGNAL: -%i dBm DIST: %i cm TIME: %s", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, dbm2cm(p_temp_mon->signal, p_temp_sta->channel), ctime(&p_temp_mon->time_rcv));
	        fprintf(message, " MONITOR: %s SIGNAL: -%i dBm DIST: %i cm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, dbm2cm(p_temp_mon->signal, p_temp_sta->channel), (int)(time(NULL)-p_temp_mon->time_rcv));
		p_temp_pos = p_start_pos;
		do
		    {
//		    fprintf(message, " POS: %s", p_temp_pos->nev);
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 3)) )
			    {
//    			    fprintf(message, " MAP: %i DIFF: %2.10f\n", p_temp_pmon->signal,  (1.0 / abs(p_temp_mon->signal-p_temp_pmon->signal)));
			    p_temp_pos->diff_sum_db += ( (1.0 / abs(p_temp_mon->signal-p_temp_pmon->signal)) < 1.0 ? (1.0 / abs(p_temp_mon->signal-p_temp_pmon->signal)) : 1.0 );
//			    p_temp_pos->diff_sum_mw += fabs(dbm2mw(0-p_temp_mon->signal)-dbm2mw(0-p_temp_pmon->signal));
//			    p_temp_pos->diff_sum_mw += dbm2mw(0-p_temp_pos->diff_sum_db);
//			    p_temp_pos->diff_sum_cm += 1.0 / abs(dbm2cm(p_temp_mon->signal, p_temp_sta->channel)-dbm2cm(p_temp_pmon->signal, p_temp_sta->channel));
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
		    p_temp_pos = p_temp_pos->next;
		    }
		while (p_temp_pos);
	        p_temp_mon = p_temp_mon->next_mon;
	        }
	    while (p_temp_mon);
	    megvan = 1;

	    p_temp_pos = p_start_pos;
	    do
		{
//		fprintf(message, " POS: %10s SUM_DIFF: %3i dBM | %2.10f mW = %3i dB | %i cm\n", p_temp_pos->nev, (1.0 / p_temp_pos->diff_sum_db), p_temp_pos->diff_sum_mw, mw2dbm(p_temp_pos->diff_sum_mw), (1.0 / p_temp_pos->diff_sum_cm));
		if (DETAIL) fprintf(message, " POS: %10s SUM_DIFF: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		if (p_temp_pos->diff_sum_db > diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    }
	    
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);
	    fprintf(message, "Helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_offs(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double diff_sum = 0.0; char *helyszin = "Ismeretlen";

    int megvan = 0;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));
	    p_temp_mon = p_temp_sta->monitor;
	    do
	        {
	        fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
		p_temp_pos = p_start_pos;
		do
		    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 3)) )
			    {
			    p_temp_pos->diff_sum_db += ( (1.0 / abs(p_temp_mon->signal-offset-p_temp_pmon->signal)) < 1.0 ? (1.0 / abs(p_temp_mon->signal-offset-p_temp_pmon->signal)) : 1.0 );
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
		    p_temp_pos = p_temp_pos->next;
		    }
		while (p_temp_pos);
	        p_temp_mon = p_temp_mon->next_mon;
	        }
	    while (p_temp_mon);
	    megvan = 1;

	    p_temp_pos = p_start_pos;
	    do
		{
		if (DETAIL) fprintf(message, " POS: %10s SUM_DIFF: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		if (p_temp_pos->diff_sum_db > diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    }
	    
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);
	    fprintf(message, "Helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_diff(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon, *p_def_mon, *p_def_pmon;

    double calc = 0.0;

    int megvan = 0;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_mon = p_temp_sta->monitor;
	    p_def_mon  = p_temp_mon;
	    do
		{
		if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
		if (p_temp_mon->signal < p_def_mon->signal)
		    {
		    p_def_mon = p_temp_mon;
		    }
	        p_temp_mon = p_temp_mon->next_mon;
		}
	    while (p_temp_mon);
	    if (DETAIL) fprintf(message, "Default monitor: %s \n", inet_ntoa(p_def_mon->monitor_address));

	    p_temp_pos = p_start_pos;
	    do
	        {
		if (DETAIL) fprintf(message, "POS: %s \n", p_temp_pos->nev);

		p_temp_pmon = p_temp_pos->monitor;
		p_def_pmon  = NULL;
		do
		    {
		    if (memcmp(p_temp_pmon->serial, p_def_mon->serial, MACLEN) == 0)
		        {
		        p_def_pmon = p_temp_pmon;
			}
		    p_temp_pmon = p_temp_pmon->next_mon;
		    }
		while (p_temp_pmon);
		if (p_def_pmon == NULL)
		    {
//		    fprintf(message, "No posmonitor found \n");
		    p_temp_pos = p_temp_pos->next;
		    continue;
		    }
//		fprintf(message, "Default MONITOR: %s SIGNAL: %-i \n", inet_ntoa(p_def_pmon->monitor_address), p_def_pmon->signal);

		p_temp_mon = p_temp_sta->monitor;		
		do
		    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (memcmp(p_temp_pmon->serial, p_def_mon->serial, MACLEN)) && (p_temp_mon->time_rcv > (time(NULL) - 4)) )
			    {

			    calc = 	(
					 (
					  (1.0*abs(p_def_mon->signal-p_temp_mon->signal))
					  <
					  (fabs(p_def_pmon->mean-p_temp_pmon->mean))
					 )
					 ?
					 (
					  (1.0*(p_def_mon->signal-p_temp_mon->signal))
					  /
					  (p_def_pmon->mean-p_temp_pmon->mean)
					 )
					 :
					 (
					  (p_def_pmon->mean-p_temp_pmon->mean)
					  /
					  (1.0*(p_def_mon->signal-p_temp_mon->signal))
					 )
					);
/*
			    calc =	(
					 (
					  (abs(p_def_mon->signal-p_temp_mon->signal))
					  <
					  (abs(p_def_pmon->signal-p_temp_pmon->signal))
					 )
					 ?
					 (
					  (1.0*(p_def_mon->signal-p_temp_mon->signal))
					  /
					  (1.0*(p_def_pmon->signal-p_temp_pmon->signal))
					 )
					 :
					 (
					  (1.0*(p_def_pmon->signal-p_temp_pmon->signal))
					  /
					  (1.0*(p_def_mon->signal-p_temp_mon->signal))
					 )
					);
*/
			    fprintf(message, " MONITOR: %s SIGNAL: %i DIFF: %2.10f ", inet_ntoa(p_temp_pmon->monitor_address), p_temp_pmon->signal, calc);

			    p_temp_pos->diff_sum_db += calc;
			    p_temp_pos->diff_sum_mon ++;
			    fprintf(message, " SUM_DIFF: %2.10f \n", p_temp_pos->diff_sum_db);
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
		    p_temp_mon = p_temp_mon->next_mon;
		    }
		while (p_temp_mon);
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);
	    megvan = 1;

	    return_pos_highest();
	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_sort(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    int megvan = 0;

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	    p_temp_pos = p_start_pos;
	    double diff_sum = 0.0; char *helyszin = "Ismeretlen";
	    do
		{
		p_temp_pos->diff_sum_db = 0.0;
		p_temp_pos->diff_sum_mon = 0;
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);

	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_pos = p_start_pos;
	    do
	        {
		p_temp_mon = p_temp_sta->monitor;
		p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon && p_temp_mon)
			{
			if (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0)
			    {
			    p_temp_pos->diff_sum_db += pow(0.1, p_temp_pos->diff_sum_mon);
			    }
			p_temp_pos->diff_sum_mon++;
			p_temp_pmon = p_temp_pmon->next_mon;
			p_temp_mon = p_temp_mon->next_mon;
			}
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);
	    megvan = 1;

	    p_temp_pos = p_start_pos;
	    do
		{
		if (DETAIL) fprintf(message, " POS: %10s SUM_MON: %i CALC: %1.8f \n", p_temp_pos->nev, p_temp_pos->diff_sum_mon, p_temp_pos->diff_sum_db);
		if (p_temp_pos->diff_sum_db >= diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    }
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);
	    if (DETAIL) fprintf(message, "CALC: %1.8f, Helyszin: %s\n", diff_sum, helyszin);
	    if (DETAIL) printf("CALC: %1.8f, Helyszin: %s\n", diff_sum, helyszin);

	    p_temp_pos = p_start_pos;
	    do
		{
		if (p_temp_pos->diff_sum_db == diff_sum) fprintf(message, " POS: %10s SUM_MON: %i CALC: %1.8f \n", p_temp_pos->nev, p_temp_pos->diff_sum_mon, p_temp_pos->diff_sum_db);
		p_temp_pos = p_temp_pos->next;
		}
	    while (p_temp_pos);

	    p_temp_sta = p_temp_sta->next_sta;
	    }

	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_dist(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double diff_sum = 0.0; char *helyszin = "Ismeretlen";

    int megvan = 0;
    double dist;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
//	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_mon = p_temp_sta->monitor;
//	    p_def_mon  = p_temp_mon;
	    while (p_temp_mon)
		{
//	        if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
		if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
//		if (p_temp_mon->signal < p_def_mon->signal)
//		    {
//		    p_def_mon = p_temp_mon;
//		    }
	        p_temp_mon = p_temp_mon->next_mon;
		}
//	    if (DETAIL) fprintf(message, "Default monitor: %s \n", inet_ntoa(p_def_mon->monitor_address));

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
	        {
//		fprintf(message, "\n POS: %s ", p_temp_pos->nev);

//		p_temp_pmon = p_temp_pos->monitor;
//		p_def_pmon  = NULL;
//		do
//		    {
//		    if (memcmp(p_temp_pmon->serial, p_def_mon->serial, MACLEN) == 0)
//		        {
//		        p_def_pmon = p_temp_pmon;
//			}
//		    p_temp_pmon = p_temp_pmon->next_mon;
//		    }
//		while (p_temp_pmon);
//		if (p_def_pmon == NULL)
//		    {
//		    fprintf(message, "No posmonitor found \n");
//		    p_temp_pos = p_temp_pos->next;
//		    continue;
//		    }
//		fprintf(message, "Default MONITOR: %s SIGNAL: %-i \n", inet_ntoa(p_def_pmon->monitor_address), p_def_pmon->signal);

		p_temp_mon = p_temp_sta->monitor;		
		while (p_temp_mon)
	    	    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 4)) )
			    {
//    			    fprintf(message, " MONITOR: %s SIGNAL: %i DIFF: %2.10f ", inet_ntoa(p_temp_pmon->monitor_address), p_temp_pmon->signal,  ( (abs(p_def_mon->signal-p_temp_mon->signal) < abs(p_def_pmon->signal-p_temp_pmon->signal)) ? (1.0*(p_def_mon->signal-p_temp_mon->signal)) / (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) : (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) / (1.0*(p_def_mon->signal-p_temp_mon->signal)) ));
    			    p_temp_pos->diff_sum_db += norm_dist(p_temp_pmon->mean, p_temp_pmon->std_dev, p_temp_mon->signal);
//			    p_temp_pos->diff_sum_mon ++;
//			    fprintf(message, " SUM_DIFF: %2.10f \n", p_temp_pos->diff_sum_db);
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
	    	    p_temp_mon = p_temp_mon->next_mon;
	    	    }
		if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		p_temp_pos = p_temp_pos->next;
		}
	    megvan = 1;

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_sta->position)
		    {
		    dist = distance(p_temp_sta->position, p_temp_pos)/10.0;
		    }
		else
		    {
		    dist = 0;
		    }
		if (dist > 3)
		    {
		    p_temp_pos->diff_sum_db *= (3/dist);
		    }
if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f DIST: %2.2f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db, dist);
		p_temp_pos = p_temp_pos->next;
		}

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_pos->diff_sum_db > diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    p_temp_sta->position = p_temp_pos;
		    }
	    
		p_temp_pos = p_temp_pos->next;
		}
	    fprintf(message, "Helyszin: %s \n", helyszin);
	    printf("Helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_felezo(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos, *p_prev_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double dist;
    double diff_sum;
    char *helyszin;

    int megvan = 0;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_mon = p_temp_sta->monitor;
	    while (p_temp_mon)
		{
		if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
	        p_temp_mon = p_temp_mon->next_mon;
		}

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
	        {
		p_temp_mon = p_temp_sta->monitor;		
		while (p_temp_mon)
	    	    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 4)) )
			    {
//    			    fprintf(message, " MONITOR: %s SIGNAL: %i DIFF: %2.10f ", inet_ntoa(p_temp_pmon->monitor_address), p_temp_pmon->signal,  ( (abs(p_def_mon->signal-p_temp_mon->signal) < abs(p_def_pmon->signal-p_temp_pmon->signal)) ? (1.0*(p_def_mon->signal-p_temp_mon->signal)) / (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) : (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) / (1.0*(p_def_mon->signal-p_temp_mon->signal)) ));
    			    p_temp_pos->diff_sum_db += norm_dist(p_temp_pmon->mean, p_temp_pmon->std_dev, p_temp_mon->signal);
//			    fprintf(message, " SUM_DIFF: %2.10f \n", p_temp_pos->diff_sum_db);
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
	    	    p_temp_mon = p_temp_mon->next_mon;
	    	    }
		if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		p_temp_pos = p_temp_pos->next;
		}
	    megvan = 1;

//adott tavolsagnal messzebb levo pontok ertekenek csokkentese
	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_sta->position)
		    {
		    dist = distance(p_temp_sta->position, p_temp_pos)/10.0;
		    }
		else
		    {
		    dist = 0;
		    }
		if (dist > 3)
		    {
		    p_temp_pos->diff_sum_db *= (3/dist);
		    }
		if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f DIST: %2.2f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db, dist);
		p_temp_pos = p_temp_pos->next;
		}

//az elozo pozicio mentese
	    p_prev_pos = p_temp_sta->position;

//a legvaloszinubb pont (legnagyobb ertek) megkeresese
	    diff_sum = 0.0;
	    helyszin = "Ismeretlen";

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_pos->diff_sum_db > diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    p_temp_sta->position = p_temp_pos;
		    }
		p_temp_pos = p_temp_pos->next;
		}
	    fprintf(message, "Kiszamolt helyszin: %s \n", helyszin);
	    printf("Kiszamolt helyszin: %s \n", helyszin);

//a kiszamolt helyszin es az elozo helyszin felezopontja
    printf("p_temp_sta->pos_xy->x: %i\n", p_temp_sta->pos_xy->x);
    printf("p_temp_sta->position->x: %i\n", p_temp_sta->position->x);
    printf("p_prev_pos->x: %i\n", p_prev_pos->x);
    printf("p_temp_sta->pos_xy->y: %i\n", p_temp_sta->pos_xy->y);
    printf("p_temp_sta->position->y: %i\n", p_temp_sta->position->y);
    printf("p_prev_pos->y: %i\n", p_prev_pos->y);
	    if (p_prev_pos)
		{
		p_temp_sta->pos_xy->x = (p_temp_sta->position->x + p_prev_pos->x) / 2;
		p_temp_sta->pos_xy->y = (p_temp_sta->position->y + p_prev_pos->y) / 2;
		}
	    else
		{
		p_temp_sta->pos_xy->x = p_temp_sta->position->x;
		p_temp_sta->pos_xy->y = p_temp_sta->position->y;
		}

//az ehhez legkozelebb levo referenciapont megkeresese, es beallitasa vegleges helyzetkent
	    dist = 100.0; //vagy valami jo nagy, hogy legyen honnan csokkenteni
	    helyszin = "Ismeretlen";

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (dist > distance(p_temp_sta->pos_xy, p_temp_pos)/1.0)
		    {
		    dist = distance(p_temp_sta->pos_xy, p_temp_pos)/1.0;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    p_temp_sta->position = p_temp_pos;
		    }
		p_temp_pos = p_temp_pos->next;
		}
	    fprintf(message, "Vegleges helyszin: %s \n", helyszin);
	    printf("Vegleges helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_norm(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double diff_sum = 0.0; char *helyszin = "Ismeretlen";

    int megvan = 0;
//    double dist;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_mon = p_temp_sta->monitor;
//	    p_def_mon  = p_temp_mon;
	    while (p_temp_mon)
		{
		if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
//		if (p_temp_mon->signal < p_def_mon->signal)
//		    {
//		    p_def_mon = p_temp_mon;
//		    }
	        p_temp_mon = p_temp_mon->next_mon;
		}
//	    if (DETAIL) fprintf(message, "Default monitor: %s \n", inet_ntoa(p_def_mon->monitor_address));

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
	        {
//		fprintf(message, "\n POS: %s ", p_temp_pos->nev);

//		p_temp_pmon = p_temp_pos->monitor;
//		p_def_pmon  = NULL;
//		do
//		    {
//		    if (memcmp(p_temp_pmon->serial, p_def_mon->serial, MACLEN) == 0)
//		        {
//		        p_def_pmon = p_temp_pmon;
//			}
//		    p_temp_pmon = p_temp_pmon->next_mon;
//		    }
//		while (p_temp_pmon);
//		if (p_def_pmon == NULL)
//		    {
//		    fprintf(message, "No posmonitor found \n");
//		    p_temp_pos = p_temp_pos->next;
//		    continue;
//		    }
//		fprintf(message, "Default MONITOR: %s SIGNAL: %-i \n", inet_ntoa(p_def_pmon->monitor_address), p_def_pmon->signal);

		p_temp_mon = p_temp_sta->monitor;		
		while (p_temp_mon)
		    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 4)) )
			    {
//			    fprintf(message, " MONITOR: %s SIGNAL: %i DIFF: %2.10f ", inet_ntoa(p_temp_pmon->monitor_address), p_temp_pmon->signal,  ( (abs(p_def_mon->signal-p_temp_mon->signal) < abs(p_def_pmon->signal-p_temp_pmon->signal)) ? (1.0*(p_def_mon->signal-p_temp_mon->signal)) / (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) : (1.0*(p_def_pmon->signal-p_temp_pmon->signal)) / (1.0*(p_def_mon->signal-p_temp_mon->signal)) ));
			    p_temp_pos->diff_sum_db += norm_dist(p_temp_pmon->mean, p_temp_pmon->std_dev, p_temp_mon->signal);
//			    p_temp_pos->diff_sum_mon ++;
//			    fprintf(message, " SUM_DIFF: %2.10f \n", p_temp_pos->diff_sum_db);
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
		    p_temp_mon = p_temp_mon->next_mon;
		    }
		if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		p_temp_pos = p_temp_pos->next;
		}
	    megvan = 1;
/*
	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_sta->position)
		    {
		    dist = distance(p_temp_sta->position, p_temp_pos)/10.0;
		    }
		else
		    {
		    dist = 0;
		    }
		if (dist > 3)
		    {
		    p_temp_pos->diff_sum_db *= (3/dist);
		    }
if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f DIST: %2.2f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db, dist);
		p_temp_pos = p_temp_pos->next;
		}
*/
	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_pos->diff_sum_db > diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    p_temp_sta->position = p_temp_pos;
		    }

		p_temp_pos = p_temp_pos->next;
		}
	    fprintf(message, "Helyszin: %s \n", helyszin);
	    printf("Helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

void comp_knn(u_char in_mac[MACLEN])
{
    time_t rawtime;
    time(&rawtime);

    struct station_pos *p_temp_sta;

    struct position_pos *p_temp_pos;

    struct monitor_pos *p_temp_mon, *p_temp_pmon;

    double diff_sum = 10000.0; char *helyszin = "Ismeretlen";

    int megvan = 0;

    init_pos();

    p_temp_sta = p_start_sta;
    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (DETAIL) fprintf(message, "STATION: " MACSTR " CHANNEL: %i TIME: %s", MAC2STR(p_temp_sta->station_address), p_temp_sta->channel, ctime(&rawtime));

	    p_temp_mon = p_temp_sta->monitor;
	    while (p_temp_mon)
		{
		if (DETAIL) fprintf(message, " MONITOR: %s SIGNAL: -%i dBm TIME: %i s ago\n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, (int)(time(NULL)-p_temp_mon->time_rcv));
	        p_temp_mon = p_temp_mon->next_mon;
		}

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
	        {
		p_temp_mon = p_temp_sta->monitor;
		while (p_temp_mon)
		    {
		    p_temp_pmon = p_temp_pos->monitor;
		    while (p_temp_pmon)
			{
			if ( (memcmp(p_temp_pmon->serial, p_temp_mon->serial, MACLEN) == 0) && (p_temp_mon->time_rcv > (time(NULL) - 4)) )
			    {
			    p_temp_pos->diff_sum_db += ((dbm2mw(-p_temp_pmon->signal) - dbm2mw(-p_temp_mon->signal)) * (dbm2mw(-p_temp_pmon->signal) - dbm2mw(-p_temp_mon->signal)));
			    }
			p_temp_pmon = p_temp_pmon->next_mon;
			}
		    p_temp_mon = p_temp_mon->next_mon;
		    }
		if (DETAIL) fprintf(message, " POS: %10s CALC: %2.10f \n", p_temp_pos->nev, p_temp_pos->diff_sum_db);
		p_temp_pos = p_temp_pos->next;
		}
	    megvan = 1;

	    p_temp_pos = p_start_pos;
	    while (p_temp_pos)
		{
		if (p_temp_pos->diff_sum_db < diff_sum)
		    {
		    diff_sum = p_temp_pos->diff_sum_db;
		    helyszin = p_temp_pos->nev;
		    coord_x = p_temp_pos->x;
		    coord_y = p_temp_pos->y;
		    p_temp_sta->position = p_temp_pos;
		    }

		p_temp_pos = p_temp_pos->next;
		}

	    fprintf(message, "Helyszin: %s \n", helyszin);
	    printf("Helyszin: %s \n", helyszin);

	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban", MAC2STR(in_mac));
	}
}

