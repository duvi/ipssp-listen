#ifndef _UTILS_H_
#define _UTILS_H_

#include <mysql.h>


int mysql_putx(char sql[512]);				//MySQL adat put

int mysql_update(char sql[512]);			//MySQL adat update

MYSQL_RES *mysql_getx(char sql[512]);			//MySQL adat get

double dbm2mw(int sig_db);				//dBm -> mW

int mw2dbm(double sig_mw);				//mW -> dBm

int dbm2cm(int sig_db, int chan);			//Jelerosseg -> tavolsag

void str2hex(char bejovo[13], u_char kimeno[MACLEN]);	//MAC string -> hexa

double average(int signal[], int n);

double deviation(int signal[], int n, double mean);

double norm_dist(double mean, double std_dev, int x);

double distance(struct position_pos *a, struct position_pos *b);

int in_array(char arr[MAXRECSTA][13], int len, char target[13]);


#endif /* _UTILS_H_ */
