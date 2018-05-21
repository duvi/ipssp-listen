#include <math.h>
#include <mysql.h>
#include "common.h"


int mysql_putx(char sql[512])
{
    int result;
    MYSQL *conn;
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "iparking", "1p4rk1n6", "iparking", 0, NULL, 0);

    mysql_query(conn, sql);

    result = mysql_insert_id(conn);

    mysql_close(conn);

    if (DEBUG) printf("SQL: %s\n", sql);

    return result;
}

MYSQL_RES *mysql_getx(char sql[512])
{
    MYSQL *conn;
    MYSQL_RES *result;
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "iparking", "1p4rk1n6", "iparking", 0, NULL, 0);

    mysql_query(conn, sql);
    result = mysql_store_result(conn);

    mysql_close(conn);

    if (DEBUG) printf("SQL: %s\n", sql);

    return result;
}

double dbm2mw(int sig_db)
{
	double sig_mw;
	sig_mw = pow(10.0, ((sig_db) / 10.0));
	return sig_mw;
}

int mw2dbm(double sig_mw)
{
	if (sig_mw == 0) return 0;
	int sig_db;
	sig_db = 10 * log10(sig_mw);
	return sig_db;
}

int dbm2cm(int sig_db, int chan)
{
	int dist_cm;
	dist_cm = ( 100.0 * (pow(10.0, ((sig_db+TX) / 20.0))) * (300.0 / (2407.0 + chan * 5)) / (4.0 * PI) );
	return dist_cm;
}

void str2hex(char bejovo[13], u_char kimeno[MACLEN])
{
	int i, j;
	for (i = 0; i < MACLEN; i++)
	    {
	    kimeno[i] = 0x00;
	    for (j = 0; j < 2; j++)
		{
		switch (bejovo[2*i+j])
		    {
		    case  48: kimeno[i] +=  0 * (16 - 15 * j); break;
		    case  49: kimeno[i] +=  1 * (16 - 15 * j); break;
		    case  50: kimeno[i] +=  2 * (16 - 15 * j); break;
		    case  51: kimeno[i] +=  3 * (16 - 15 * j); break;
		    case  52: kimeno[i] +=  4 * (16 - 15 * j); break;
		    case  53: kimeno[i] +=  5 * (16 - 15 * j); break;
		    case  54: kimeno[i] +=  6 * (16 - 15 * j); break;
		    case  55: kimeno[i] +=  7 * (16 - 15 * j); break;
		    case  56: kimeno[i] +=  8 * (16 - 15 * j); break;
		    case  57: kimeno[i] +=  9 * (16 - 15 * j); break;
		    case  97: kimeno[i] += 10 * (16 - 15 * j); break;
		    case  98: kimeno[i] += 11 * (16 - 15 * j); break;
		    case  99: kimeno[i] += 12 * (16 - 15 * j); break;
		    case 100: kimeno[i] += 13 * (16 - 15 * j); break;
		    case 101: kimeno[i] += 14 * (16 - 15 * j); break;
		    case 102: kimeno[i] += 15 * (16 - 15 * j); break;
		    }
		}
	    }
}

double average(int signal[], int n)
{
	double atlag;
	int sum = 0;
	int i;
	
	for (i = 0; i < n; i++)
	    {
	    sum += signal[i];
	    }

	atlag = (double)sum / n;
//	printf("mean: %2.8f ", atlag);

	return atlag;
}	

double deviation(int signal[], int n, double mean)
{
	double std_dev;
	double sum = 0.0;
	int i;
	
	for (i = 0; i < n; i++)
	    {
	    sum += pow((signal[i]-mean), 2.0);
	    }
	
	if (n == 1)
	    {
	    std_dev = 0;
	    }
	else
	    {
	    std_dev = sqrt(sum/(n-1));
	    }
//	printf("std_dev: %2.8f \n", std_dev);

	return std_dev;
}

double norm_dist(double mean, double std_dev, int x)
{
	double fx;
	fx = pow(EEE, -0.5*(pow(((x-mean)/std_dev),2.0))) / (std_dev*sqrt(2*PI));
	return fx;
}

double distance(struct position_pos *a, struct position_pos *b)
{
	double dist;
	dist = sqrt((a->x - b->x)*(a->x - b->x)+(a->y - b->y)*(a->y - b->y));
//	fprintf(message,"a:[%i,%i] b:[%i,%i] d=%2.4f \n", a->x, a->y, b->x, b->y, dist);
	return dist;
}
