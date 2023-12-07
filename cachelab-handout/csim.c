#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define arrlen 32
static int s;
static int E;
static int b;
char filename[1000];


int main(int argc, char* argv[])
{
    int optchr;
    while (-1 != (optchr = getopt(argc, argv, "s:E:b:t:"))) {
        
        switch (optchr)
        {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            default:
                perror("error happens");
        }
        
    }
    return 0;
}
