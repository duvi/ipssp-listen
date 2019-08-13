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

void del_sta(char char_mac[CHARMACLEN])
{
    char sql[512];
    int result;

    snprintf(sql, sizeof sql, "DELETE FROM `station_data` WHERE `sta_id` = '%s'", char_mac);
    mysql_putx(sql);
    snprintf(sql, sizeof sql, "DELETE FROM `station_list` WHERE `sta_id` = '%s'", char_mac);
    result = mysql_update(sql);

    if (result == 0) {
        fprintf(message, "Nincs a %s MAC az adatbazisban. \n", char_mac);
    }
    if (result == 1) {
        fprintf(message, "%s torolve. \n", char_mac);
    }
}

void record_stations() {
    char sql[512];
    int i;
    MYSQL_RES *sql_result;
    MYSQL_ROW row;

    snprintf(sql, sizeof sql, "SELECT `sta_id` FROM `station_list` WHERE `record` = '1' LIMIT %i", MAXRECSTA);
    sql_result = mysql_getx(sql);

    for (i = 0; i < MAXRECSTA; i++) {
        strcpy(rec_stations[i], "\0");
    }

    i = 0;
    while ((row = mysql_fetch_row(sql_result))) {
        strcpy(rec_stations[i], row[0]);
        i++;
    }

    mysql_free_result(sql_result);
}

void record_sta(char char_mac[CHARMACLEN])
{
    char sql[512];
    int result;

    snprintf(sql, sizeof sql, "UPDATE `station_list` SET `record` = NOT `record` WHERE `sta_id` = '%s'", char_mac);
    result = mysql_update(sql);
    if (result == 0) {
        fprintf(message, "Nincs a %s MAC az adatbazisban. \n", char_mac);
    }
    if (result == 1) {
        fprintf(message, "%s rogzitese atkapcsolva. \n", char_mac);
        record_stations();
        int i;
        fprintf(message, "Rogzito stationok: \n");
        for (i = 0; i < MAXRECSTA; i++) {
            if (strlen(rec_stations[i]) != 0) fprintf(message, "%s \n", rec_stations[i]);
        }
    }
}

void beolvas(struct sender * adat, struct sockaddr_in kuldo)		//Új adat fogadása
{
    char sql[512];
    char char_mac[CHARMACLEN];

    sprintf(char_mac, MACSTR, MAC2STR(adat->station));

    snprintf(sql, sizeof sql, "REPLACE INTO `station_data`(`sta_id`,`mon_id`,`signal`) VALUES('%s','" MACSTR "',%i)", char_mac, MAC2STR(adat->monitor), adat->signal);
    mysql_putx(sql);

//  snprintf(sql, sizeof sql, "REPLACE INTO `station_list`(`sta_id`,`channel`) VALUES('%s',%i)", char_mac, adat->channel);
    snprintf(sql, sizeof sql, "INSERT INTO `station_list`(`sta_id`,`channel`,`r`,`g`,`b`) \
                                VALUES('%s',%i,FLOOR(RAND()*255),FLOOR(RAND()*255),FLOOR(RAND()*255)) \
                                ON DUPLICATE KEY UPDATE `time_last`=NOW(), `channel`=%i", char_mac, adat->channel, adat->channel);
    mysql_putx(sql);

    snprintf(sql, sizeof sql, "REPLACE INTO `monitor_data`(`mon_id`,`ip`) VALUES('" MACSTR "','%s')", MAC2STR(adat->monitor), inet_ntoa(kuldo.sin_addr));
    mysql_putx(sql);

    if (p_record->enabled && in_array(rec_stations, 10, char_mac)) {
        if (DEBUG) printf("Recording data of %s\n", char_mac);
        FILE *record_file;
        char filename_rec[40];
        char dirname[41];
        sprintf(dirname, "positions/%s/%s", p_record->nev, char_mac);
        struct stat st;
        if (stat(dirname,&st)) {
            mkdir(dirname, 0755);
        }

        sprintf(filename_rec, "%s/%03i.pos", dirname, p_record->num);
        record_file = fopen(filename_rec, "a");
        fprintf(record_file, "record " MACSTR " %s %i \n", MAC2STR(adat->monitor), inet_ntoa(kuldo.sin_addr), adat->signal);
        fclose(record_file);
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
    char char_mac[CHARMACLEN];
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
