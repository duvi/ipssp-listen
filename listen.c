/*
** Helymeghatározó "szerver" program
*/


#include <pthread.h>

#include "common.h"
#include "compare.h"
#include "listen.h"
#include "pos.h"
#include "sta.h"
#include "talk.h"
#include "utils.h"


int main(void)
{

    p_record = (struct record_pos *) malloc(sizeof(struct record_pos));

    if (konfig())
	{
	perror("konfiguracios hiba");
	return 1;
	}

    p_start_sta = NULL;
    p_start_pos = NULL;

    char char_mac[CHARMACLEN];
    u_char in_mac[MACLEN];

    int time_max;
    int time_i;
/*
    char sql[512];
    MYSQL_RES *sql_result;
    MYSQL_RES *sql_sub_result;
    MYSQL_ROW row;
    MYSQL_ROW sub_row;
*/
    int sockfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int numbytes;
    socklen_t addr_len;
    char buf[MAXBUFLEN];


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
	perror("socket");
	exit(1);
	}

    my_addr.sin_family = AF_INET; // host byte order
    my_addr.sin_port = htons(COMMPORT); // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatikusan kitöltődik a saját IP-mel
    memset(&(my_addr.sin_zero), '\0', 8); // kinullázza a struktúra többi részét

    if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
	{
	perror("bind");
	exit(1);
	}

    addr_len = sizeof(struct sockaddr);

    pthread_t thread;
    if (pthread_create(&thread, NULL, data_coll, NULL))
	{
	perror("Thread");
	exit(1);
	}

    pthread_t thread_park;
    if (pthread_create(&thread_park, NULL, park_coll, NULL))
	{
	perror("Thread");
	exit(1);
	}

    if (DELETE)
	{
        pthread_t thread_periodic_del;
        if (pthread_create(&thread_periodic_del, NULL, periodic_del, NULL))
	    {
	    perror("Thread_periodic_del");
	    exit(1);
	    }
	}


    while(1)
	{
	if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
	    {
	    perror("recvfrom");
	    exit(1);
	    }

	buf[numbytes] = '\0';

	if(DEBUG) {printf("from %s ",inet_ntoa(their_addr.sin_addr));
		   printf("port %d ",ntohs(their_addr.sin_port));
		   printf("contains %s\n",buf);
		   printf("packet is %d bytes long\n",numbytes);}

	if (strstr(buf, "get_map_name"))
	    {
	    asprintf(&message_str, "mapname %s", p_record->nev);
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "get_num_pos"))
	    {
	    asprintf(&message_str, "posnum %i", (p_record->num + 1));
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "show_all"))
	    {
	    message=fopen("logs/position.log", "w");
	    list_sta(1);
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "list_sta"))
	    {
	    message=fopen("logs/position.log", "w");
	    list_sta(0);
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "list_pos"))
	    {
	    message=fopen("logs/position.log", "w");
	    list_pos();
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "show_sta"))
	    {
	    message=fopen("logs/position.log", "w");

	    sscanf(buf, "%*s %12s ", char_mac);
	    str2hex(char_mac, in_mac);
	    show_sta(in_mac);
	
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "del_sta"))
	    {
	    message=fopen("logs/position.log", "w");

	    sscanf(buf, "%*s %12s ", char_mac);
	    del_sta(char_mac);

	    fclose(message);

	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "record_sta"))
	    {
	    message=fopen("logs/position.log", "w");

	    sscanf(buf, "%*s %12s ", char_mac);
	    record_sta(char_mac);

	    fclose(message);

	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "rec_sta"))
	    {
	    message=fopen("logs/position.log", "w");
		fprintf(message, "Eddigi rogzito MAC " MACSTR "\n", MAC2STR(rec_sta));
	    sscanf(buf, "%*s %12s ", char_mac);
	    str2hex(char_mac, rec_sta);
		fprintf(message, "Uj rogzito MAC " MACSTR "\n", MAC2STR(rec_sta));
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "rec_single"))
	    {
	    message=fopen("logs/position.log", "w");
		sscanf(buf, "%*s %i %i,%i", &p_record->enabled, &p_record->x, &p_record->y);
		if (p_record->enabled)
		    {
		    p_record->num++;
		    FILE *recording, *index;
		    recording = fopen("recording", "w");
		    fprintf(recording, "%i,%i", p_record->x, p_record->y);
		    fclose(recording);

		    char dirname[50];
		    sprintf(dirname, "positions/%s/index.pos", p_record->nev);
		    index = fopen(dirname, "a");
		    fprintf(index, "position %i,%i,%i\n", p_record->num, p_record->x, p_record->y);
		    fclose(index);

		    fprintf(message, "Rogzites bekapcsolva \n");
		    }
		else
		    {
		    remove("recording");

		    //Uj pozicio betoltese memoriaba
		    if (load_pos(p_record))
		        {
		        fprintf(message, "Sikertelen betoltes! \n");
		        }
		    else
		        {
		        fprintf(message, "Sikeres betoltes! \n");
		        }

		    //Poziciok kiirasa temp fileba
		    fprintf(message, "Poziciok kiirase temp fileba: ");
		    if (p_start_pos)
			{
			if (save_map("save_map temp"))
			    {
			    fprintf(message, "Sikertelen mentes! \n");
			    }
			else
			    {
			    fprintf(message, "Sikeres mentes! \n");
			    }
			}
		    else
			{
			fprintf(message, "Nincs rogzitett pozicio! \n");
			}

		    //Poziciok betoltese temp filebol adatbazisba
		    fprintf(message, "Poziciok betoltese temp filebol adatbazisba: ");
		    if (load_map("load_map temp"))
			{
			fprintf(message, "Sikertelen betoltes! \n");
			}
		    else
			{
			fprintf(message, "Sikeres betoltes! \n");
			}

		    //Temp file torlese
		    fprintf(message, "Temp file torlese: ");
		    if (remove("temp.map"))
			{
			fprintf(message, "Sikertelen torles! \n");
			}
		    else
			{
				fprintf(message, "Sikeres torles! \n");
			}

		    fprintf(message, "Rogzites kikapcsolva \n");
		    }
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "set_num_pos"))
	    {
	    int pos_newnum;
	    sscanf(buf, "%*s %i ", &pos_newnum);
	    p_record->num = (pos_newnum - 1);

	    message=fopen("logs/position.log", "w");
	    fprintf(message, "Pozicio rogzites kovetkezo sorszam beallitva: %i \n", (p_record->num + 1));
	    fclose(message);

	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "load_single"))
	    {
	    message=fopen("logs/position.log", "w");
	    sscanf(buf, "%*s %*s %i %i ", &time_i, &time_max);
//	    for (time_i = 1; time_i <= time_max; time_i++)
//		{
	    sprintf(char_mac, MACSTR, MAC2STR(rec_sta));
	    del_sta(char_mac);
	    load_single(time_i, time_max);
//	    fprintf(message, "%i ", time_i);
	    if (strstr(buf, "compare") && p_start_pos)
		{
		compare(rec_sta);
		}
	    if (strstr(buf, "comp_offs") && p_start_pos)
		{
		comp_offs(rec_sta);
		}
	    if (strstr(buf, "comp_diff") && p_start_pos)
		{
		comp_diff(rec_sta,0);
		}
	    if (strstr(buf, "comp_sort") && p_start_pos)
		{
		comp_sort(rec_sta);
		}
	    if (strstr(buf, "comp_norm") && p_start_pos)
		{
		comp_norm(rec_sta);
		}
	    if (strstr(buf, "comp_dist") && p_start_pos)
		{
		comp_dist(rec_sta);
		}
//		}
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "offset"))
	    {
	    message=fopen("logs/position.log", "w");
		fprintf(message, "Eddigi offset %i \n", offset);
	    sscanf(buf, "%*s %i ", &offset);
		fprintf(message, "Uj offset %i \n", offset);
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "comp"))
	    {
	    sscanf(buf, "%s %12s ", command, char_mac);
	    str2hex(char_mac, in_mac);
	    message=fopen("logs/position.log", "w");
	    message_str = NULL;
	    Sasprintf(message_str, "coords ");

	    if (p_start_pos)
		{
		if (strstr(command, "compare"))
		    {
		    compare(in_mac);
		    }
		if (strstr(command, "comp_offs"))
		    {
		    comp_offs(in_mac);
		    }
		if (strstr(command, "comp_diff"))
		    {
		    comp_diff(in_mac,0);
		    }
		if (strstr(command, "comp_area"))
		    {
		    comp_diff(in_mac,1);
		    }
		if (strstr(command, "comp_sort"))
		    {
		    comp_dist(in_mac);
		    }
		if (strstr(command, "comp_norm"))
		    {
		    comp_norm(in_mac);
		    }
		if (strstr(command, "comp_dist"))
		    {
		    comp_dist(in_mac);
		    }
		if (strstr(command, "comp_felezo"))
		    {
		    comp_felezo(in_mac);
		    }
		if (strstr(command, "comp_knn"))
		    {
		    comp_knn(in_mac);
		    }
		}
	    else
		{
		fprintf(message, "Nincs mentett pozicio!");
		}
		
	    fclose(message);
	    if (coord_x && coord_y)
		{
		Sasprintf(message_str, "%s%i,%i", message_str, coord_x, coord_y);
		coord_x = 0;
		coord_y = 0;
		talk(message_str, ntohs(their_addr.sin_port));
		}
	    else
		{
		talk("done", ntohs(their_addr.sin_port));
		}
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "del_pos"))
	    {
	    message_str = NULL;
	    Sasprintf(message_str, "message ");
	    if (!del_pos(buf))
		{
		Sasprintf(message_str, "%sNincs ilyen pozicio az adatbazisban\n", message_str);
		}
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
//	    if (p_start_pos) save_map("save_map autosave");
	    continue;
	    }

	if (strstr(buf, "clear_map"))
	    {
	    message_str = NULL;
	    Sasprintf(message_str, "message ");
	    clear_map();
	    Sasprintf(message_str, "%sTerkep torolve.\n", message_str);
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "save_map"))
	    {
	    message_str = NULL;
	    Sasprintf(message_str, "message ");
	    if (p_start_pos)
		{
		if (save_map(buf))
		    {
		    Sasprintf(message_str, "%sSikertelen mentes! \n", message_str);
		    }
		else
		    {
		    Sasprintf(message_str, "%sSikeres mentes! \n", message_str);
		    }
		}
	    else
		{
		Sasprintf(message_str, "%sNincs rogzitett pozicio! \n", message_str);
		}
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "load_map"))
	    {
	    message_str = NULL;
	    Sasprintf(message_str, "message ");
	    message=fopen("logs/position.log", "w");
	    if (load_map(buf))
		{
		Sasprintf(message_str, "%sSikertelen betoltes! \n", message_str);
		}
	    else
		{
		Sasprintf(message_str, "%sSikeres betoltes! \n", message_str);
		}
	    talk(message_str, ntohs(their_addr.sin_port));
	    free(message_str);
	    continue;
	    }

	if (strstr(buf, "set_session"))
	    {
	    message=fopen("logs/position.log", "w");

	    sscanf(buf, "%*s %s ", p_record->nev);

	    char dirname[40];
	    sprintf(dirname, "positions/%s", p_record->nev);
	    struct stat st;
	    if (stat(dirname,&st))
		{
		mkdir(dirname, 0755);
		fprintf(message, "A konyvtar meg nem letezik. \n");
		}
	    else
		{
		fprintf(message, "A konyvtar mar letezik. \n");
		}
	    fprintf(message, "Pozicio rogzites session beallitva: %s \n", p_record->nev);
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "load_pos"))
	    {
	    message=fopen("logs/position.log", "w");

	    char adat[25];
	    char dirname[60];
	    sscanf(buf, "%*s %s %s ", p_record->nev, p_record->mac);

	    FILE *index;
	    sprintf(dirname, "positions/%s/index.pos", p_record->nev);
	    if ((index = fopen(dirname, "r")) != NULL)
		{
		while(fgets(adat, 25, index)!=NULL)
		    {
		    if (strstr(adat, "position"))
			{
			sscanf(adat, "%*s %i,%i,%i", &p_record->num, &p_record->x, &p_record->y);
			if (load_pos(p_record))
			    {
			    fprintf(message, "Sikertelen betoltes! \n");
			    }
			else
			    {
			    fprintf(message, "Sikeres betoltes! \n");
			    }
			}
		    }
		}
	    else
		{
		fprintf(message, "Sikertelen betoltes! \n");
		}
	    fclose(index);

	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "load_sql"))
	    {
	    message=fopen("logs/position.log", "w");
//Memoria es mysql poziciok torlese
	    message_str = NULL;
	    Sasprintf(message_str, "Memoria es mysql poziciok torlese:\n");
	    clear_map();
	    Sasprintf(message_str, "%sTerkep torolve.\n", message_str);
	    fprintf(message, message_str);
	    free(message_str);

//Poziciok betoltese memoriaba
	    char adat[25];
	    char dirname[60];
	    sscanf(buf, "%*s %s %s ", p_record->nev, p_record->mac);

	    FILE *index;
	    sprintf(dirname, "positions/%s/index.pos", p_record->nev);
	    if ((index = fopen(dirname, "r")) != NULL)
		{
		while(fgets(adat, 25, index)!=NULL)
		    {
		    if (strstr(adat, "position"))
			{
			sscanf(adat, "%*s %i,%i,%i", &p_record->num, &p_record->x, &p_record->y);
			if (load_pos(p_record))
			    {
			    fprintf(message, "Sikertelen betoltes! \n");
			    }
			else
			    {
			    fprintf(message, "Sikeres betoltes! \n");
			    }
			}
		    }
		}
	    else
		{
		fprintf(message, "Sikertelen betoltes! \n");
		}
	    fclose(index);

//Pozicio kiirasa temp fileba
	    fprintf(message, "Poziciok kiirase temp fileba: ");
	    if (p_start_pos)
		{
		if (save_map("save_map temp"))
		    {
		    fprintf(message, "Sikertelen mentes! \n");
		    }
		else
		    {
		    fprintf(message, "Sikeres mentes! \n");
		    }
		}
	    else
		{
		fprintf(message, "Nincs rogzitett pozicio! \n");
		}

//Poziciok betoltese temp filebol adatbazisba
	    fprintf(message, "Poziciok betoltese temp filebol adatbazisba: ");
	    if (load_map("load_map temp"))
		{
		fprintf(message, "Sikertelen betoltes! \n");
		}
	    else
		{
		fprintf(message, "Sikeres betoltes! \n");
		}

//Temp file torlese
	    fprintf(message, "Temp file torlese: ");
	    if (remove("temp.map"))
		{
		fprintf(message, "Sikertelen torles! \n");
		}
	    else
		{
		fprintf(message, "Sikeres torles! \n");
		}

//Befejezes
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }

	if (strstr(buf, "load_sta"))
	    {
	    message=fopen("logs/position.log", "w");
	    if (load_sta(buf))
		{
		fprintf(message, "Sikertelen betoltes! \n");
		}
	    else
		{
		fprintf(message, "Sikeres betoltes! \n");
		}
	    fclose(message);
	    talk("done", ntohs(their_addr.sin_port));
	    continue;
	    }
	}

    close(sockfd);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int konfig(void)
{
    FILE *config;
    if ((config = fopen("listen.ini","r")) == NULL) return 1;

    char char_mac[CHARMACLEN];

    p_record->enabled = 0;
    p_record->num = 0;
    sprintf (p_record->nev, "default");

    offset = 0;
    DEBUG = 0;
    DETAIL = 0;
    DELETE = 1;
    DEL_INT = 30;
    DEL_TIMEOUT = 10;
    COMMPORT = 1000;
    DATAPORT = 1001;
    str2hex("0013ce100ba2", rec_sta);

    char adat[25];

    while(fgets(adat, 25, config)!=NULL)
	{
	if (strstr(adat, "debug"))
	    {
	    sscanf(adat, "%*s %i ", &DEBUG);
	    continue;
	    }
	if (strstr(adat, "detail"))
	    {
	    sscanf(adat, "%*s %i ", &DETAIL);
	    continue;
	    }
	if (strstr(adat, "delete"))
	    {
	    sscanf(adat, "%*s %i ", &DELETE);
	    continue;
	    }
	if (strstr(adat, "del_interval"))
	    {
	    sscanf(adat, "%*s %i ", &DEL_INT);
	    continue;
	    }
	if (strstr(adat, "del_timeout"))
	    {
	    sscanf(adat, "%*s %i ", &DEL_TIMEOUT);
	    continue;
	    }
	if (strstr(adat, "comm_port"))
	    {
	    sscanf(adat, "%*s %i ", &COMMPORT);
	    continue;
	    }
	if (strstr(adat, "data_port"))
	    {
	    sscanf(adat, "%*s %i ", &DATAPORT);
	    continue;
	    }
	if (strstr(adat, "park_port"))
	    {
	    sscanf(adat, "%*s %i ", &PARKPORT);
	    continue;
	    }
	if (strstr(adat, "rec_sta"))
	    {
	    sscanf(adat, "%*s %12s ", char_mac);
	    str2hex(char_mac, rec_sta);    
	    continue;
	    }
	}
    fclose(config);

    printf("Recording station       " MACSTR "\n", MAC2STR(rec_sta));
    printf("Offset                  %i dBm \n", offset);
    printf("Command UDP port        %i \n", COMMPORT);
    printf("Data UDP port           %i \n", DATAPORT);
    printf("Park UDP port           %i \n", PARKPORT);
    printf("Debug mode              "); if (DEBUG) printf("ON\n"); else printf("OFF\n");
    printf("Detail compare          "); if (DETAIL) printf("ON\n"); else printf("OFF\n");
    printf("Periodic station delete ");
	if (DELETE)
	    {
	    printf("ON\n");
	    printf("    Period:             %i s\n", DEL_INT);
	    printf("    Timeout:            %i s\n", DEL_TIMEOUT);
	    }
	else printf("OFF\n");

    record_stations();
    int i;
    printf("Rogzito stationok: \n");
    for (i = 0; i < MAXRECSTA; i++) {
        if (strlen(rec_stations[i]) != 0) printf("%s \n", rec_stations[i]);
    }

    if (COMMPORT == DATAPORT)
	{
	printf("ERROR: Incoming port is equal to data port!\n");
	return 1;
	}

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
void *data_coll()
{
    int sockfd;
    struct sockaddr_in my_addr;	//my address information
    struct sockaddr_in their_addr;	//their address information
    int numbytes;
    socklen_t addr_len;
    struct sender *adat_be;
    time(&rec_time);

    printf("Capture thread started\n");

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
        perror("socket");
        exit(1);
        }

    my_addr.sin_family = AF_INET; // host byte order
    my_addr.sin_port = htons(DATAPORT); // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatikusan kitöltődik a saját IP-mel
    memset(&(my_addr.sin_zero), '\0', 8); // kinullázza a struktúra többi részét

    if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
        {
        perror("bind");
        exit(1);
        }

    addr_len = sizeof(struct sockaddr);

    while (1)
	{
	adat_be = (struct sender *) malloc(sizeof(struct sender));
	if ((numbytes=recvfrom(sockfd,adat_be, sizeof(struct sender), 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
	    {
	    perror("recvfrom");
	    exit(1);
	    }

	if(DEBUG)
	    {
	    printf("from %s ", inet_ntoa(their_addr.sin_addr));
	    printf("packet is %d bytes long\n",numbytes);
	    printf("monitor: " MACSTR "\n", MAC2STR(adat_be->monitor));
	    printf("station: " MACSTR "\n", MAC2STR(adat_be->station));
	    printf("signal: %i\n",adat_be->signal);
	    printf("channel: %i\n",adat_be->channel);
	    }

	beolvas(adat_be, their_addr);
	free(adat_be);
	continue;
	}
}

////////////////////////////////////////////////////////////////////////////////
void *park_coll()
{
    int sockfd;
    struct sockaddr_in my_addr;	//my address information
    struct sockaddr_in their_addr;	//their address information
    int numbytes;
    socklen_t addr_len;
    struct park_send *adat_be;

    char sql[512];

    printf("Parking sensor thread started\n");

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
        perror("socket");
        exit(1);
        }

    my_addr.sin_family = AF_INET; // host byte order
    my_addr.sin_port = htons(PARKPORT); // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatikusan kitöltődik a saját IP-mel
    memset(&(my_addr.sin_zero), '\0', 8); // kinullázza a struktúra többi részét

    if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
        {
        perror("bind");
        exit(1);
        }

    addr_len = sizeof(struct sockaddr);

    while (1)
	{
	adat_be = (struct park_send *) malloc(sizeof(struct park_send));
	if ((numbytes=recvfrom(sockfd,adat_be, sizeof(struct park_send), 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
	    {
	    perror("recvfrom");
	    exit(1);
	    }

	if(DEBUG)
	    {
	    printf("DEBUG_PARK from %s %d bytes\n", inet_ntoa(their_addr.sin_addr), numbytes);
	    printf("DEBUG_PARK id: 0x%02x\n", adat_be->id);
	    printf("DEBUG_PARK free: 0x%02x\n", adat_be->free);
	    }

	snprintf(sql, sizeof sql, "REPLACE INTO `park_data`(`park_id`,`free`) VALUES(%c,%c)", adat_be->id, adat_be->free);
//	snprintf(sql, sizeof sql, "REPLACE INTO `park_data`(`park_id`,`free`) VALUES(%x,%x)", adat_be->id, adat_be->free);
	mysql_putx(sql);

	free(adat_be);
	continue;
	}
}

////////////////////////////////////////////////////////////////////////////////
void *periodic_del()
{
    char sql[512];
    MYSQL_RES *sql_result;
    MYSQL_ROW row;

    time_t rawtime;

    printf("Delete thread started\n");
    sleep(DEL_INT);

    while (1)
	{
	message = fopen("logs/delete.log", "a");
	time(&rawtime);
	fprintf(message, "TIME: %s", ctime(&rawtime));

	snprintf(sql, sizeof sql, "SELECT `sta_id` FROM `station_list` WHERE `record` = 0 AND `time_last` < (NOW() - INTERVAL %i SECOND)", DEL_TIMEOUT);
	sql_result = mysql_getx(sql);

	while ((row = mysql_fetch_row(sql_result)))
	    {
	    del_sta(row[0]);
	    }
	mysql_free_result(sql_result);

	fclose(message);
	sleep(DEL_INT);
	}
}
