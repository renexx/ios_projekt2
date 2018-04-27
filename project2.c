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
#include<limits.h>

#define MMAP(poiner)
{
    (pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}
#define mysleep(max_time)
{
    if (max_time != 0) sleep(rand() % max_time);
}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}

int *share_variable = NULL;
sem_t *semaphore1 = NULL;
FILE *pfile;

int load()
{
    pfile = fopen("proj2.out", "w");
    MMAP(share_variable);
    if((semaphore1 = sem_open("/xbolfr00", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
        return -1;
    return 0;
}

void close_semaphores()
{
    UNMAP(share_variable);
    sem_close(semaphore1);
    //odalokovanie miesta semaphora
    sem_unlink("/xbolfr00");
}
void process_riders()
{
    exit(0);
}
void process_bus(int delay, int cap)
{
    while(true)
    {

    }
    exit(0);
}
void gen_riders(int riders, int delay)
{
    for(int i; i <= riders; i++)
    {
        pid_t riders_id = fork();
        if (riders_id == 0)
        {
            process_rider(riders);
        }
    mysleep(delay);
    }
    exit(0);
}
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
    if(load() == -1)
    {
        close_semaphores;
        return -1;
    }
    else
    {

    }
    pid_t bus = fork();
    if (bus == 0)
    {
        //child
        process_bus(delay, cap);

    }
    else
    {
        //parent
        pid_t riders_generator = fork();
        if (riders_generator == 0)
        {
            gen_riders();
        }
        gen_riders();
    }
    close_semaphores();
    exit(0);
    return 0;
}
