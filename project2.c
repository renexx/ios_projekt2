/** @autor: Rene Bolf
*    @date :
*    @file: project2.c
*    @brief:Implementácia synchronizačného problému The Senate Bus Problem
*    @see :http://www.fit.vutbr.cz/study/courses/IOS/public/Lab/projekty/projekt2/projekt2.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<string.h>
#include <errno.h>
#include<limits.h>

#include <unistd.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/mman.h>

#define MMAP(pointer){
    (pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}
#define mysleep(max_time)
{
    if (max_time != 0) sleep(rand() % max_time);
}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}
/*share variable*/
int *A;
int *CR;
int *MAX_IN_BUS;
int *HOW_GET_IN;
int *ALL_FLAG;
int *IN_TIME;

int shm_a;
int shm_cr;
int shm_maxinbus;
int shm_howgetin;
int shm_allflag
int shm_intime;


/*semaphores*/
sem_t *bus = NULL;
sem_t *riders_allonaboard = NULL;
sem_t *riders_ended = NULL;
sem_t *semaphore1 = NULL;
sem_t *write = NULL;

FILE *pfile;

int load()
{
    pfile = fopen("proj2.out", "w");
    if(pfile == NULL) {
		fprintf(stderr,"Cannot create file.\n");
		return 1;
    }
    setbuf(pfile,NULL);
}
    if((bus = sem_open("/xbolfr00bus", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        return -1;
    if((riders_allonaboard = sem_open("/xbolfr00riders_allonaboard", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        return -1;
    if((riders_ended = sem_open("/xbolfr00riders_ended", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        return -1;
    if((bus = sem_open("/xbolfr00semaphore1", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
        return -1;
    if((write = sem_open("/xbolfr00write", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
        return -1;

    shm_a = shm_open("/xbolfr00a", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_cr = shm_open("/xbolfr00cr", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_maxinbus = shm_open("/xbolfr00maxinbus", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_howgetin = shm_open("/xbolfr00howgetin", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_allflag = shm_open("/xbolfr00allflag", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_intime = shm_open("/xbolfr00intime", O_CREAT | O_EXCL | O_RDWR, 0666);
    if(shm_a == ERROR || shm_cr == ERROR || shm_maxinbus == ERROR || shm_howgetin == ERROR || shm_allflag == ERROR || shm_intime == ERROR)
    {
        fprintf(stderr,"Cannot load source\n");
        clean();
        exit(0);
    }
    ftruncate(shm_a, sizeof(int));
    ftruncate(shm_cr, sizeof(int));
    ftruncate(shm_maxinbus, sizeof(int));
    ftruncate(shm_howgetin, sizeof(int));
    ftruncate(shm_allflag, sizeof(int));
    ftruncate(shm_intime, sizeof(int));

    A = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_a, 0);
    CR = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_cr, 0);
    MAX_IN_BUS = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_maxinbus, 0);
    HOW_GET_IN = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_howgetin, 0);
    ALL_FLAG = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_allflag, 0);
    IN_TIME = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_intime, 0);
    if(A == MAP_FAILED || CR == MAP_FAILED || MAX_IN_BUS == MAP_FAILED || HOW_GET_IN == MAP_FAILED || ALL_FLAG == MAP_FAILED || IN_TIME == MAP_FAILED)
    {
        fprintf(stderr,"Cannot load source!\n");
        clean();
        exit(0);
    }
}

void close_semaphores()
{
    //UNMAP(share_variable);

    sem_close(bus);
    sem_close(riders_allonaboard);
    sem_close(riders_ended);
    sem_close(semaphore1);
    sem_close(write);
    //odalokovanie miesta semaphora
    sem_unlink("/xbolfr00bus");
    sem_unlink("/xbolfr00riders_allonaboard");
    sem_unlink("/xbolfr00riders_ended");
    sem_unlink("/xbolfr00semaphore1");
    sem_unlink("/xbolfr00write");
}
void clean()
{
    close_semaphores();
    fclose(pfile);

    munmap(A, sizeof(int));
    munmap(CR, sizeof(int));
    munmap(MAX_IN_BUS, sizeof(int));
    munmap(HOW_GET_IN, sizeof(int));
    munmap(ALL_FLAG, sizeof(int));
    munmap(IN_TIME, sizeof(int));

    shm_unlink("/xbolfr00a");
    close(shm_a);
    shm_unlink("/xbolfr00cr");
    close(shm_cr);
    shm_unlink("/xbolfr00xbolfr00maxinbus");
    close(shm_maxinbus);
    shm_unlink("/xbolfr00xbolfr00howgetin");
    close(shm_howgetin);
    shm_unlink("/xbolfr00xbolfr00allflag");
    close(shm_allflag);
    shm_unlink("/xbolfr00xbolfr00intime");
    close(shm_intime);
}
void process_riders()
{
    exit(0);
}
void process_bus(int *C, int *ABT)
{
    sem_wait(write);
    fprintf(pfile, "%d\t\t: BUS\t\t: start\n",*A);
    sem_post(write);
    while(*MAX_IN_BUS <= R && *ALL_FLAG != 1){
        sem_wait(semaphore1);
        sem_wait(write);
        (*A)++;
        fprintf(pfile, "%d\t\t: BUS\t\t: arrival\n",*A);
        sem_post(write);
        if(*CR > 0)
        {
            sem_wait(write);
            (*A)++;
            fprintf(pfile, "%d\t\t: BUS\t\t: start boarding: %lu\n",*A,(*CR > C ? C : *CR));
            *HOW_GET_IN = 0;
            sem_post(bus);
            sem_wait(riders_allonaboard);
            *MAX_IN_BUS += *HOW_GET_IN;
            sem_wait(write);
            (*A)++;
            fprintf(pfile, "%d\t\t: BUS\t\t: end boarding: 0\n",*A);
            sem_post(write);
        }
    sem_wait(write);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: depart:\n",*A);
    sem_post(write);
    sem_post(semaphore1);
    usleep(rand()%(1000*ABT+1));
    sem_wait(write);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: end\n",*A);
    sem_post(write);
    for(int i = 0; i < *HOW_GET_IN; i++)
        sem_post(riders_ended);
    }
    sem_wait(write);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: finish\n",*A);
    sem_post(w);
    exit(0);
}
void gen_riders(int R, int delay)
{
    for(int i; i <= R; i++)
    {
        pid_t R_id = fork();
        if (R_id == 0)
        {
            process_rider(R);
        }
    mysleep(delay);
    }
    exit(0);
}
int params(int *R, int *C, int *ART, int *ABT, char **argv)
{
    *R = atoi(argv[1]);
    *C = atoi(argv[2]);
    *ART = atoi(argv[3]);
    *ABT = atoi(argv[4]);
    if (*R > 0 && *C > 0 && *ART >= 0 && *ART <= 1000 && *ABT >= 0 && *ABT <= 1000)
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
    int R, C, ART, ABT;
    int var = params(&R, &C, &ART, &ABT, argv);
    if (var == 1)
        return 1;
    if(load() == -1)
    {
        close_semaphores();
        return -1;
    }
    else
    {

    }
    pid_t p_bus = fork();
    if (p_bus == 0)
    {
        //child
        process_bus(&C,&ABT);

    }
    else //p_bus > 0
    {
        //parent
        pid_t riders_generator = fork();
        if (riders_generator == 0)
        {
            //child
            gen_riders();
        }
        else
        {
            //parent
            waitpid(riders_generator,NULL,0);
            waitpid(p_bus,NULL,0);
        }
        gen_riders();
    }
    close_semaphores();
    exit(0);
    return 0;
}
