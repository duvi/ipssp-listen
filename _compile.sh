gcc -D_GNU_SOURCE -c *.c -ggdb3 -Wall -Wextra -Wno-format-zero-length `mysql_config --cflags --libs`
gcc *.o -lm -lpthread -o listen -ggdb3 -Wall -Wextra `mysql_config --cflags --libs`
rm *.o
