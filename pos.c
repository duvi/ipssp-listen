#include <time.h>
#include "common.h"
#include "utils.h"

int del_pos(char adat[MAXBUFLEN])
{
    char torlendo[MAXPOSLEN];
    sscanf(adat, "%*s %s ", torlendo);

    struct position_pos *p_temp_pos, *p_prev_pos;
    struct monitor_pos *p_temp_mon, *p_prev_mon;
    p_temp_pos = p_start_pos;

    while (p_temp_pos)
	{
	if (strcmp(torlendo, p_temp_pos->nev) == 0)
	    {
	    p_temp_mon = p_temp_pos->monitor;
	    p_prev_mon = NULL;
	    while (p_temp_mon)
		{
		p_prev_mon = p_temp_mon->next_mon;
		free(p_temp_mon);
		p_temp_mon = p_prev_mon;
		}
	    Sasprintf(message_str, "%sPozicio %s torolve. \n", message_str, p_temp_pos->nev);
	    if (p_temp_pos == p_start_pos)
		{
		p_start_pos = p_temp_pos->next;
		free(p_temp_pos);
		}
	    else
		{
		p_prev_pos->next = p_temp_pos->next;
		free(p_temp_pos);
		}
	    return 1;
	    break;
	    }
	else
	    {
	    p_prev_pos = p_temp_pos;
	    p_temp_pos = p_temp_pos->next;
	    }
	}
    return 0;
}

void clear_map(void)
{
    char sql[512];

    snprintf(sql, sizeof sql, "TRUNCATE TABLE position_list");
    mysql_putx(sql);
    snprintf(sql, sizeof sql, "TRUNCATE TABLE position_data");
    mysql_putx(sql);
//    snprintf(sql, sizeof sql, "TRUNCATE TABLE monitor_list");
//    mysql_putx(sql);

    struct position_pos *p_temp_pos, *p_prev_pos;
    struct monitor_pos *p_temp_mon, *p_prev_mon;
    p_temp_pos = p_start_pos;

    while (p_temp_pos)
	{
	p_temp_mon = p_temp_pos->monitor;
	p_prev_mon = NULL;
	while (p_temp_mon)
	    {
	    p_prev_mon = p_temp_mon->next_mon;
	    free(p_temp_mon);
	    p_temp_mon = p_prev_mon;
	    }
	p_prev_pos = p_temp_pos;
	Sasprintf(message_str, "%sPozicio %s torolve. \n", message_str, p_temp_pos->nev);
	free(p_temp_pos);
	p_temp_pos = p_prev_pos->next;
	}
    p_start_pos = NULL;
    p_record->num = 0;
}

void rec_pos(struct sender * adat, struct sockaddr_in kuldo)	//Pozicio rogzites
{
    struct monitor_pos *p_new_mon, *p_temp_mon;
    p_temp_mon = p_new_pos->monitor;

    int csere_mon = 0;

    while (p_temp_mon)
	{
	if (memcmp(adat->monitor, p_temp_mon->serial, MACLEN) == 0)
	    {
	    if (adat->signal < p_temp_mon->signal)
	        {
	        p_temp_mon->signal		= adat->signal;
	        p_temp_mon->monitor_address = kuldo.sin_addr;
//		p_temp_mon->time_rcv	= time(NULL);
	        }

	    csere_mon = 1;
	    break;
	    }
	else
	    {
	    p_temp_mon = p_temp_mon->next_mon;
	    }
	}
    if (csere_mon == 0)
        {
        p_new_mon = (struct monitor_pos *) malloc(sizeof(struct monitor_pos));

        memcpy(p_new_mon->serial, adat->monitor, MACLEN);
        p_new_mon->signal	   = adat->signal;
        p_new_mon->monitor_address = kuldo.sin_addr;
//	p_new_mon->time_rcv	   = time(NULL);
	p_new_mon->next_mon	   = p_new_pos->monitor;
		
	p_new_pos->monitor	   = p_new_mon;
	}
}

void list_pos(void)		//Positionok listazasa
{
    char sql[512];
    MYSQL_RES *sql_result;
    MYSQL_RES *sql_sub_result;
    MYSQL_ROW row;
    MYSQL_ROW sub_row;

    snprintf(sql, sizeof sql, "SELECT pos_id, x, y, time_rec FROM position_list");
    sql_result = mysql_getx(sql);

    while ((row = mysql_fetch_row(sql_result)))
	{
	fprintf(message, "POSITION: %s [%s,%s] TIME: %s \n", row[0], row[1] ,row[2], row[3]);

	snprintf(sql, sizeof sql, "SELECT position_data.signal, position_data.mean, position_data.std_dev, monitor_list.ip \
	                            FROM position_data \
	                            LEFT JOIN monitor_list \
	                            ON position_data.mon_id = monitor_list.mon_id \
	                            WHERE position_data.pos_id = '%s' \
	                            ORDER BY position_data.mean", row[0]);
	sql_sub_result = mysql_getx(sql);

	    while ((sub_row = mysql_fetch_row(sql_sub_result)))
		{
		fprintf(message, "	MON: %s SIG: -%s dBm   MEAN: -%s dBm  DEV: %s \n", sub_row[3], sub_row[0], sub_row[1], sub_row[2]);
		}
	    mysql_free_result(sql_sub_result);
	}
    mysql_free_result(sql_result);
}

int save_map(char adat[MAXBUFLEN])		//Terkep mentese
{
    char savename[30];
    char filename[MAXMAPLEN];
    FILE *positions;

    sscanf(adat, "%*s %s ", filename);
    sprintf(savename, "%s.map", filename);

    if ((positions = fopen(savename,"w")) == NULL) return 1;

    struct position_pos *p_temp_pos;
    struct monitor_pos *p_temp_mon;
    p_temp_pos = p_start_pos;

    do
	{
	fprintf(positions, "POSITION: %s [%i,%i] %i\n", p_temp_pos->nev, p_temp_pos->x, p_temp_pos->y, (int)p_temp_pos->time_rec);
	p_temp_mon = p_temp_pos->monitor;
	while (p_temp_mon)
	    {
	    fprintf(positions, "MON: " MACSTR " %s %i %2.4f %2.4f\n", MAC2STR(p_temp_mon->serial), inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->signal, p_temp_mon->mean, p_temp_mon->std_dev);
	    p_temp_mon = p_temp_mon->next_mon;
	    }
	p_temp_pos = p_temp_pos->next;
	}
    while (p_temp_pos);

    fclose(positions);
    return 0;
}

int load_map(char adat[MAXBUFLEN])		//Terkep betoltese
{
    char sql[512];

    char loadname[30];
    char filename[MAXMAPLEN];
    FILE *positions;

    sscanf(adat, "%*s %s ", filename);
    sprintf(loadname, "%s.map", filename);

    if ((positions = fopen(loadname,"r")) == NULL) return 1;

    char sor[60];
    char in_ip[IPLEN];
    char char_mac[13];
    int pos_id;
    int x;
    int y;
    int time_scan;
    int signal;
    double mean;
    double std_dev;

    snprintf(sql, sizeof sql, "TRUNCATE TABLE position_list");
    mysql_putx(sql);
    snprintf(sql, sizeof sql, "TRUNCATE TABLE position_data");
    mysql_putx(sql);
//    snprintf(sql, sizeof sql, "TRUNCATE TABLE monitor_list");
//    mysql_putx(sql);

    while(fgets(sor, 60, positions) != NULL)
	{
	if (strstr(sor, "POSITION"))
	    {
	    sscanf(strstr(sor, "POSITION:"), "POSITION: %*i [%i,%i] %i ", &x, &y, &time_scan);

	    snprintf(sql, sizeof sql, "INSERT INTO position_list(x,y) VALUES(%i,%i)", x, y);
	    pos_id = mysql_putx(sql);

	    continue;
	    }
	if (strstr(sor, "MON"))
	    {
	    sscanf(sor, "%*s %12s %s %i %lf %lf ", char_mac, in_ip, &signal, &mean, &std_dev);

	    snprintf(sql, sizeof sql, "INSERT INTO position_data(pos_id,mon_id,signal,mean,std_dev) \
	                                VALUES(%i,'%s',%i,'%2.4f','%2.4f')", pos_id, char_mac, signal, mean, std_dev);
	    mysql_putx(sql);
	    snprintf(sql, sizeof sql, "INSERT INTO monitor_list(mon_id,ip) \
	                                VALUES('%s','%s')", char_mac, in_ip) ;
	    mysql_putx(sql);

	    continue;
	    }
	}
    fclose(positions);
    return 0;
}

int load_pos(struct record_pos * adat)		//Pozicio betoltese
{

    char loadname[80];
    FILE *position;

    sprintf(loadname, "positions/%s/%s/%03i.pos", adat->nev, adat->mac, adat->num);
    if(DEBUG) printf("positions/%s/%s/%03i.pos x%i y%i\n", adat->nev, adat->mac, adat->num, adat->x, adat->y);

    if ((position = fopen(loadname,"r")) == NULL) return 1;

    char sor[70];
    char in_ip[IPLEN];
    char char_mac[13];
    u_char in_mac[MACLEN];
    int in_signal;

    int megvan;

    struct position_pos *p_new_pos;
    struct monitor_pos *p_temp_mon, *p_new_mon;

    p_new_pos = (struct position_pos *) malloc(sizeof(struct position_pos));


    sprintf(p_new_pos->nev, "%i", adat->num);
    p_new_pos->x		= adat->x;
    p_new_pos->y		= adat->y;
    p_new_pos->time_rec		= time(NULL);
    p_new_pos->monitor		= NULL;
    p_new_pos->next		= p_start_pos;
    p_start_pos			= p_new_pos;

    while(fgets(sor, 70, position) != NULL)
	{
	if (strstr(sor, "record"))
	    {
	    sscanf(sor, "%*s %12s %s %i ", char_mac, in_ip, &in_signal);
	    str2hex(char_mac, in_mac);
	    p_temp_mon = p_new_pos->monitor;
	    megvan = 0;
	    while (p_temp_mon)
		{
		if (memcmp(in_mac, p_temp_mon->serial, MACLEN) == 0)
		    {
		    p_temp_mon->signals[p_temp_mon->n] = in_signal;
		    p_temp_mon->n++;
		    megvan = 1;
		    break;
		    }
		else
		    {
		    p_temp_mon = p_temp_mon->next_mon;
		    }
		}
	    if (megvan == 0)
		{
		p_new_mon = (struct monitor_pos *) malloc(sizeof(struct monitor_pos));

		memcpy(p_new_mon->serial, in_mac, MACLEN);
		inet_aton(in_ip, &p_new_mon->monitor_address);
		p_new_mon->signals[0]		= in_signal;
		p_new_mon->n			= 1;
		p_new_mon->signal		= 0;
		p_new_mon->mean			= 0;
		p_new_mon->std_dev		= 0;
		p_new_mon->time_rcv		= time(NULL);
		p_new_mon->next_mon		= p_new_pos->monitor;

		p_new_pos->monitor		= p_new_mon;
		}
	    }
	}

    p_temp_mon = p_new_pos->monitor;
    while(p_temp_mon)
	{
	p_temp_mon->mean	= average(p_temp_mon->signals, p_temp_mon->n);
	p_temp_mon->std_dev	= deviation(p_temp_mon->signals, p_temp_mon->n, p_temp_mon->mean);
	p_temp_mon->signal	= p_temp_mon->mean;

	fprintf(message,"%s mean:%2.8f dev:%2.8f \n", inet_ntoa(p_temp_mon->monitor_address), p_temp_mon->mean, p_temp_mon->std_dev);
	p_temp_mon = p_temp_mon->next_mon;
	}

    fclose(position);
    return 0;
}
