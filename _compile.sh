gcc -D_GNU_SOURCE -c *.c -g3 -ggdb3 -Wall -Wextra -Wno-format-zero-length `mysql_config --cflags --libs`
gcc *.o -lm -lpthread -o listen -g3 -ggdb3 -Wall -Wextra `mysql_config --cflags --libs`
rm *.o
