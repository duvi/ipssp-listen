#ifndef _STA_H_
#define _STA_H_


void list_sta(int sta_detail);				//Stationok listazasa

void show_sta(u_char in_mac[MACLEN]);			//Egy station kiirasa

void del_sta(u_char in_mac[MACLEN]);			//Station torlese

void record_sta(u_char in_mac[MACLEN]);			//Station rogzites kapcsolasa

void beolvas(struct sender * adat, struct sockaddr_in kuldo);		//Új adat fogadása

int load_sta(char adat[MAXBUFLEN]);			//Station lista betoltese

int load_single(int time_i, int time_max);			//Egy station betoltese


#endif /* _STA_H_ */