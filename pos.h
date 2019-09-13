#ifndef _POS_H_
#define _POS_H_


int del_pos(char adat[MAXBUFLEN]);			//Terkeppont torlese

void clear_map(void);					//Terkep torlese

int save_map(char adat[MAXBUFLEN]);			//Terkep mentese

int load_map(char adat[MAXBUFLEN]);			//Terkep betoltese

int load_pos(struct record_pos * adat);			//Pozicio betoltese


#endif /* _POS_H_ */
