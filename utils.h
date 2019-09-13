#ifndef _UTILS_H_
#define _UTILS_H_

#include <mysql.h>


//MySQL adat put
int mysql_putx(char sql[512]);

//MySQL adat update
int mysql_update(char sql[512]);

//MySQL adat get
MYSQL_RES *mysql_getx(char sql[512]);

//dBm -> mW
double dbm2mw(int sig_db);

//mW -> dBm
int mw2dbm(double sig_mw);

//Jelerosseg -> tavolsag
int dbm2cm(int sig_db, int chan);

//MAC string -> hexa
void str2hex(char bejovo[CHARMACLEN], u_char kimeno[MACLEN]);

//Jelszint atlagolas
double average(int signal[], int n);

//Jelszint szoras
double deviation(int signal[], int n, double mean);

//Jelszint eloszlas
double norm_dist(double mean, double std_dev, int x);

//Ket pont tavolsaga
double distance(struct position_pos *a, struct position_pos *b);

//Kereses tombben
int in_array(char arr[MAXRECSTA][CHARMACLEN], int len, char target[CHARMACLEN]);


#endif /* _UTILS_H_ */
