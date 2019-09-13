#ifndef _LISTEN_H_
#define _LISTEN_H_


//Main thread for commands
int konfig(void);

//Data collection thread
void *data_coll();

//Preiodic delete thread
void *periodic_del();


#endif /* _LISTEN_H_ */
