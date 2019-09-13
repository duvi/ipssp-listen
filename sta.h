#ifndef _STA_H_
#define _STA_H_


//Station torlese
void del_sta(char char_mac[CHARMACLEN]);

//Station rogzites kapcsolasa
void record_sta(char char_mac[CHARMACLEN]);

//Rogzito stationok betoltese
void record_stations();

//Új adat fogadása
void beolvas(struct sender * adat, struct sockaddr_in kuldo);


#endif /* _STA_H_ */
