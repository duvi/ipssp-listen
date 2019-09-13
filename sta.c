#include <time.h>
#include <mysql.h>
#include "common.h"
#include "utils.h"
#include "compare.h"
#include <sys/stat.h>

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
        char filename_rec[69];
        char dirname[53];
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
