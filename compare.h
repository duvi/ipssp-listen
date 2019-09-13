#ifndef _COMPARE_H_
#define _COMPARE_H_


//Helymeghatarozo algoritmus
void compare(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus offset
void comp_offs(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus ap diff
void comp_diff(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus monitor order every station
void comp_sort(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus eloszlas tavolsag-sulyozassal
void comp_dist(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus eloszlas koordinata atlagolassal
void comp_felezo(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus eloszlas
void comp_norm(u_char in_mac[MACLEN]);

//Helymeghatarozo algoritmus KNN
void comp_knn(u_char in_mac[MACLEN]);


#endif /* _COMPARE_H_ */
