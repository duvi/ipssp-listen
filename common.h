#ifndef _COMMON_H_
#define _COMMON_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include <sys/stat.h>


#define MAXBUFLEN 100
#define MAXMAPLEN 30
#define MAXPOSLEN 30
#define MACLEN 6
#define CHARMACLEN 13
#define IPLEN 16
#define TX 20
#define PI 3.14159265
#define EEE 2.71828183
#define MAXRECSTA 10

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x%02x%02x%02x%02x%02x"


//Safe asprintf macro
#define Sasprintf(write_to, ...) {\
    char *tmp_string_for_extend = write_to; \
    asprintf(&(write_to), __VA_ARGS__); \
    free(tmp_string_for_extend); \
}

struct monitor_pos {
	struct in_addr monitor_address;
	u_char serial[MACLEN];
	int signal;
	double mean;
	double std_dev;
	time_t time_rcv;
	int n;
	int signals[100];
	struct monitor_pos *next_mon;
};

struct station_pos {
	u_char station_address[MACLEN];
	time_t time_last;
	int channel;
	int record;
	struct position_pos *position;
	struct position_pos *pos_xy;
	struct monitor_pos *monitor;
	struct station_pos *next_sta;
};

struct position_pos {
	char nev[MAXPOSLEN];
	time_t time_rec;
	double diff_sum_db;
	int diff_sum_mon;
	int x;
	int y;
	struct monitor_pos *monitor;
	struct position_pos *next;
};

struct sender {
	u_char monitor[MACLEN];
	u_char station[MACLEN];
	u_char signal;
	u_char channel;
};

struct park_send {
	u_char id;
	u_char free;
};

struct record_pos {
	int enabled;
	int x;
	int y;
	int num;
	char nev[MAXPOSLEN];
	char mac[CHARMACLEN];
};


struct station_pos *p_start_sta;
struct position_pos *p_start_pos, *p_new_pos;
struct record_pos *p_record;

FILE *message;
char *message_str;

time_t rec_time;

int coord_x;
int coord_y;

char rec_stations[MAXRECSTA][CHARMACLEN];

char command[10];

u_char rec_sta[MACLEN];
int offset;
int DEBUG;
int DETAIL;
int DELETE;
int COMMPORT;
int DATAPORT;
int PARKPORT;
int DEL_INT;
int DEL_TIMEOUT;


#endif /* _COMMON_H_ */
