#ifndef _STA_H_
#define _STA_H_


void del_sta(char char_mac[CHARMACLEN]);		//Station torlese

void record_sta(char char_mac[CHARMACLEN]);		//Station rogzites kapcsolasa

//Rogzito stationok betoltese
void record_stations();

void beolvas(struct sender * adat, struct sockaddr_in kuldo);		//Új adat fogadása


#endif /* _STA_H_ */
