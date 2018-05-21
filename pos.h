#ifndef _POS_H_
#define _POS_H_


int del_pos(char adat[MAXBUFLEN]);			//Terkeppont torlese

void rec_pos(struct sender * adat, struct sockaddr_in kuldo);		//Terkeppont rogzitese

void list_pos(void);					//Terkeppontok listazasa

void get_pos(void);					//Terkeppontok listazasa

void sort_pos(void);					//Poziciok sorba rendezese

void clear_map(void);					//Terkep torlese

int save_map(char adat[MAXBUFLEN]);			//Terkep mentese

int load_map(char adat[MAXBUFLEN]);			//Terkep betoltese

int load_pos(struct record_pos * adat);			//Pozicio betoltese


#endif /* _POS_H_ */
