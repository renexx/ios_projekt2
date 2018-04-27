/** @autor: Rene Bolf
*    @date :
*    @file: project2.c
*    @brief:Implementácia synchronizačného problému The Senate Bus Problem
*    @see :http://www.fit.vutbr.cz/study/courses/IOS/public/Lab/projekty/projekt2/projekt2.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>
#include<string.h>
#include <errno.h>



int params(int *riders, int *capacity, int *art, int *abt, char **argv)
{
    *riders = atoi(argv[1]);
    *capacity = atoi(argv[2]);
    *art = atoi(argv[3]);
    *abt = atoi(argv[4]);
    if (*riders > 0 && *capacity > 0 && *art >= 0 && *art <= 1000 && *abt >= 0 && *abt <= 1000)
        return 0;
    else
    {
        fprintf(stderr, "Parametre su zadane v zlych intervaloch\n");
        return 1;
    }

}

int main (int argc, char *argv[])
{
    if(argc != 5)
    {
        fprintf(stderr, "Nespravny pocet argumentov\n");
        return 1;
    }
    /* ART je max hodnota doby v milisekundach po ktorej je generovany novy proces rider
       ABT je maximalna hodnota doby v milisekundach počas ktorej proces bus simuluje jazdu

    */
    int riders, capacity, art, abt;
    int var = params(&riders, &capacity, &art, &abt, argv);
    if (var == 1)
        return 1;

    return 0;
}
