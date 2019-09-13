#ifndef _POS_H_
#define _POS_H_


//Terkeppont torlese
int del_pos(char adat[MAXBUFLEN]);

//Terkep torlese
void clear_map(void);

//Terkep mentese
int save_map(char adat[MAXBUFLEN]);

//Terkep betoltese
int load_map(char adat[MAXBUFLEN]);

//Pozicio betoltese
int load_pos(struct record_pos * adat);


#endif /* _POS_H_ */
