#include <time.h>
#include <mysql.h>
#include "common.h"
#include "utils.h"
#include "compare.h"
#include <sys/stat.h>

void list_sta(int sta_detail)			//Stationok listazasa
{
    char sql[512];
    MYSQL_RES *sql_result;
    MYSQL_RES *sql_sub_result;
    MYSQL_ROW row;
    MYSQL_ROW sub_row;

    snprintf(sql, sizeof sql, "SELECT `sta_id`, `channel` FROM `station_list`");
    sql_result = mysql_getx(sql);

    while ((row = mysql_fetch_row(sql_result)))
	{
	fprintf(message, "STATION: %s CHANNEL: %s \n", row[0], row[1]);

	if (sta_detail)
	    {
	    snprintf(sql, sizeof sql, "SELECT `monitor_data`.`ip`, `station_data`.`signal`, `station_data`.`time_rcv` \
	                                FROM `station_data` \
	                                LEFT JOIN `monitor_data` \
	                                ON `station_data`.`mon_id` = `monitor_data`.`mon_id` \
	                                WHERE `station_data`.`sta_id` = '%s'", row[0]);
	    sql_sub_result = mysql_getx(sql);

	    while ((sub_row = mysql_fetch_row(sql_sub_result)))
		{
		fprintf(message, "MONITOR: %s SIGNAL: -%s dBm TIME: %s\n", sub_row[0], sub_row[1], sub_row[2]);
		}
	    mysql_free_result(sql_sub_result);
	    }
	}
    mysql_free_result(sql_result);
}

void show_sta(u_char in_mac[MACLEN])		//Egy station kiirasa
{
    char sql[512];
    MYSQL_RES *sql_result;
    MYSQL_RES *sql_sub_result;
    MYSQL_ROW row;
    MYSQL_ROW sub_row;

    snprintf(sql, sizeof sql, "SELECT `sta_id`, `channel` FROM `station_list` WHERE `sta_id` = '" MACSTR "'", MAC2STR(in_mac));
    sql_result = mysql_getx(sql);

    if ((row = mysql_fetch_row(sql_result)))
	{
	fprintf(message, "STATION: %s CHANNEL: %s \n", row[0], row[1]);

	snprintf(sql, sizeof sql, "SELECT `monitor_data`.`ip`, `station_data`.`signal`, `station_data`.`time_rcv` \
	                            FROM `station_data` \
	                            LEFT JOIN `monitor_data` \
	                            ON `station_data`.`mon_id` = `monitor_data`.`mon_id` \
	                            WHERE `station_data`.`sta_id` = '%s'", row[0]);
	sql_sub_result = mysql_getx(sql);

	while ((sub_row = mysql_fetch_row(sql_sub_result)))
	    {
	    fprintf(message, "MONITOR: %s SIGNAL: -%s dBm TIME: %s\n", sub_row[0], sub_row[1], sub_row[2]);
	    }
	mysql_free_result(sql_sub_result);
	}
    else
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban \n", MAC2STR(in_mac));
	}

    mysql_free_result(sql_result);
}

void del_sta(u_char in_mac[MACLEN])
{
    struct station_pos *p_temp_sta, *p_prev_sta;
    struct monitor_pos *p_temp_mon, *p_prev_mon;
    p_temp_sta = p_start_sta;

    int megvan = 0;

    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    p_temp_mon = p_temp_sta->monitor;
	    p_prev_mon = NULL;
	    while (p_temp_mon)
		{
		p_prev_mon = p_temp_mon->next_mon;
		free(p_temp_mon);
		p_temp_mon = p_prev_mon;
		}
	    free(p_temp_sta->position);
	    free(p_temp_sta->pos_xy);
	    if (p_temp_sta == p_start_sta)
		{
		p_start_sta = p_temp_sta->next_sta;
		free(p_temp_sta);
		}
	    else
		{
		p_prev_sta->next_sta = p_temp_sta->next_sta;
		free(p_temp_sta);
		}
	    fprintf(message, "STATION " MACSTR " torolve. \n", MAC2STR(p_temp_sta->station_address));
	    megvan = 1;
	    break;
	    }
	else
	    {
	    p_prev_sta = p_temp_sta;
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban \n", MAC2STR(in_mac));
	}
}

void record_sta(u_char in_mac[MACLEN])
{
    struct station_pos *p_temp_sta;
    p_temp_sta = p_start_sta;

    int megvan = 0;

    while (p_temp_sta)
	{
	if (memcmp(in_mac, p_temp_sta->station_address, MACLEN) == 0)
	    {
	    if (p_temp_sta->record == 1)
		{
		p_temp_sta->record = 0;
		fprintf(message, MACSTR " rogzitese kikapcsolva. \n", MAC2STR(in_mac));
		}
	    else
		{
		p_temp_sta->record = 1;
		fprintf(message, MACSTR " rogzitese bekapcsolva. \n", MAC2STR(in_mac));
		}
	    megvan = 1;
	    break;
	    }
	else
	    {
	    p_temp_sta = p_temp_sta->next_sta;
	    }
	}
    if (megvan == 0)
	{
	fprintf(message, "Nincs a " MACSTR " MAC az adatbazisban \n", MAC2STR(in_mac));
	}
}

int load_sta(char adat[MAXBUFLEN])
{
    char loadname[30];
    char filename[MAXMAPLEN];
    FILE *stations;

    sscanf(adat, "%*s %s ", filename);
    sprintf(loadname, "%s.sta", filename);

    if ((stations = fopen(loadname,"r")) == NULL) return 1;

    char sor[70];
    char char_mac[13];
    char in_ip[IPLEN];
    int in_time;

    struct station_pos *p_new_sta, *p_prev_sta, *p_temp_sta;
    struct monitor_pos *p_new_mon, *p_prev_mon;
    p_new_sta = NULL;
    p_prev_sta = NULL;
    p_temp_sta = p_start_sta;

    while(fgets(sor, 70, stations) != NULL)
	{
	if (strstr(sor, "STATION"))
	    {
	    p_new_mon = NULL;
	    p_prev_mon = NULL;

	    p_new_sta = (struct station_pos *) malloc(sizeof(struct station_pos));
	    sscanf(strstr(sor, "STATION:"), "STATION: %12s %*s %i ", char_mac, &p_new_sta->channel);
	    str2hex(char_mac, p_new_sta->station_address);    

	    p_new_sta->monitor		= NULL;
	    p_new_sta->next_sta		= NULL;

	    if (p_prev_sta)
		p_prev_sta->next_sta	= p_new_sta;
	    else
		p_start_sta		= p_new_sta;
	    p_prev_sta = p_new_sta;

	    continue;
	    }
	if (strstr(sor, "MONITOR") && p_new_sta)
	    {
	    sscanf(strstr(sor, "TIME:"), "TIME: %i ", &in_time);
	    if (in_time < 6)
		{
		p_new_mon = (struct monitor_pos *) malloc(sizeof(struct monitor_pos));
		sscanf(sor, "%*s %s %*s %i ", in_ip, &p_new_mon->signal);
		inet_aton(in_ip, &p_new_mon->monitor_address);
		p_new_mon->signal		= -p_new_mon->signal;
		p_new_mon->time_rcv		= time(NULL);
		p_new_mon->next_mon		= NULL;
		
		if (strstr(sor, "192.168.100.201")) str2hex("00188418a0d1", p_new_mon->serial); //p_new_mon->serial=8650018372;
		if (strstr(sor, "192.168.100.202")) str2hex("00188416c469", p_new_mon->serial); //p_new_mon->serial=8648002337;
		if (strstr(sor, "192.168.100.203")) str2hex("0018841d6ef9", p_new_mon->serial); //p_new_mon->serial=8650019434;
		if (strstr(sor, "192.168.100.204")) str2hex("001884192359", p_new_mon->serial); //p_new_mon->serial=8648002863;
		if (strstr(sor, "192.168.100.205")) str2hex("001884189521", p_new_mon->serial); //p_new_mon->serial=8651012747;
		if (strstr(sor, "192.168.100.206")) str2hex("0018841709c9", p_new_mon->serial); //p_new_mon->serial=8651010415;
		if (strstr(sor, "192.168.100.207")) str2hex("0018841a2821", p_new_mon->serial); //p_new_mon->serial=8649013564;
		if (strstr(sor, "192.168.100.208")) str2hex("0018841b4211", p_new_mon->serial); //p_new_mon->serial=8650018963;
		if (strstr(sor, "192.168.100.209")) str2hex("001884170a31", p_new_mon->serial); //p_new_mon->serial=8650016040;

		if (p_prev_mon)
		    p_prev_mon->next_mon	= p_new_mon;
		else
		    p_new_sta->monitor		= p_new_mon;
		p_prev_mon = p_new_mon;
		}

	    continue;
	    }
	}
    if (p_new_sta)
	p_new_sta->next_sta = p_temp_sta;
    fclose(stations);
    return 0;
}

int load_single(int time_i, int time_max)
{
    FILE *stations;
    char sor[70];
    char in_ip[IPLEN];
//    char char_mac_sta[13];
    char char_mac_mon[13];
    int in_time;
    int in_signal;
    int in_channel;
    struct sender *adat_be;
    struct sockaddr_in their_addr;

    char loadname[30];
    sprintf(loadname, "test.pos");

    fprintf(message, "%s\n", loadname);
    if ((stations = fopen(loadname,"r")) == NULL) return 1;
//    if ((stations = fopen("logs/record.log","r")) == NULL) return 1;

    adat_be = (struct sender *) malloc(sizeof(struct sender));

    in_time = 0;

    while(fgets(sor, 70, stations) != NULL)
	{
	if (strstr(sor, "record") && (in_time < time_max))
	    {
//	    sscanf(sor, "%*s %i %s %s %s %i %i ", &in_time, char_mac_sta, char_mac_mon, in_ip, &in_signal, &in_channel);
	    sscanf(sor, "%*s %s %s %i ", char_mac_mon, in_ip, &in_signal);
	    in_channel = 1;
	    in_time++;
//	    in_time -= 1000;
//	    if ((in_time > (time_max - 3)) && (in_time <= time_max))
//		{
		inet_aton(in_ip, &their_addr.sin_addr);
//		str2hex(char_mac_sta, adat_be->station);    
		memcpy(adat_be->station, rec_sta, MACLEN);    
		str2hex(char_mac_mon, adat_be->monitor);    
		adat_be->signal = (char)in_signal;
		adat_be->channel = (char)in_channel;
		beolvas(adat_be, their_addr);
//		if (DEBUG) fprintf(message, "record %i " MACSTR " " MACSTR " %s %i %i \n", in_time, MAC2STR(adat_be->station), MAC2STR(adat_be->monitor), inet_ntoa(their_addr.sin_addr), adat_be->signal, adat_be->channel);
		if (DETAIL) fprintf(message, "record " MACSTR " " MACSTR " %s %i %i \n", MAC2STR(adat_be->station), MAC2STR(adat_be->monitor), inet_ntoa(their_addr.sin_addr), adat_be->signal, adat_be->channel);

		if (in_time > 8)
		    {
		    fprintf(message, "%i ", in_time);
		    comp_dist(rec_sta);
		    }

		continue;
//		}
	    }
	}
    fclose(stations);
    free(adat_be);
    return 0;
}
