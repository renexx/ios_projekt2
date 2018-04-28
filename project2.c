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
#define ERROR (-1)
#define CHYBA2 (2)
/*share variable*/
int *A = NULL;
int id_A = 0;
int *CR = NULL;
int id_CR = 0;
int *MAX_IN_BUS = NULL;
int id_MAX_IN_BUS = 0;
int *ALL_FLAG = NULL;
int id_ALL_FLAG = 0;
int *HOW_GET_IN = NULL;
int id_HOW_GET_IN = 0;
int *IN_TIME = NULL;
int id_IN_TIME = 0;

/*semaphores*/
sem_t *bus = NULL;
sem_t *riders_allonaboard = NULL;
sem_t *riders_ended = NULL;
sem_t *semaphore1 = NULL;
sem_t *write_line = NULL;

FILE *pfile;

void clean()
{
    sem_close(bus);
    sem_close(riders_allonaboard);
    sem_close(riders_ended);
    sem_close(semaphore1);
    sem_close(write_line);
    //odalokovanie miesta semaphora
    sem_unlink("/xbolfr00bus");
    sem_unlink("/xbolfr00riders_allonaboard");
    sem_unlink("/xbolfr00riders_ended");
    sem_unlink("/xbolfr00semaphore1");
    sem_unlink("/xbolfr00write_line");
    shmctl(id_A, IPC_RMID, NULL);
	shmctl(id_CR, IPC_RMID, NULL);
	shmctl(id_MAX_IN_BUS, IPC_RMID, NULL);
	shmctl(id_ALL_FLAG, IPC_RMID, NULL);
	shmctl(id_HOW_GET_IN, IPC_RMID, NULL);
    shmctl(id_IN_TIME, IPC_RMID, NULL);
}
int load()
{
    pfile = fopen("proj2.out", "w");
    if(pfile == NULL) {
		fprintf(stderr,"Cannot create file.\n");
		return 1;
    }
    setbuf(pfile,NULL);

    if ((id_A = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }

    if ((A = shmat(id_A, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }

    if ((id_CR = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((CR = shmat(id_CR, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((id_MAX_IN_BUS = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((MAX_IN_BUS = shmat(id_MAX_IN_BUS, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((id_ALL_FLAG = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((ALL_FLAG = shmat(id_ALL_FLAG, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((id_HOW_GET_IN = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((HOW_GET_IN = shmat(id_HOW_GET_IN, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((id_IN_TIME = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }
    if ((IN_TIME = shmat(id_IN_TIME, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Cannot load source\n");
        clean();
        return -1;
    }

        *A = 1;
        *CR = 0;
        *MAX_IN_BUS = 0;
        *ALL_FLAG = 0;
        *HOW_GET_IN = 0;
        *IN_TIME = 0;

    if ((bus = sem_open("/xbolfr00bus", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore\n");
        clean();
        return -1;
    }
    if ((riders_allonaboard = sem_open("/xbolfr00riders_allonaboard", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore\n");
        clean();
        return -1;
    }
    if ((riders_ended = sem_open("/xbolfr00riders_ended", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore\n");
        clean();
        return -1;
    }
    if ((semaphore1 = sem_open("/xbolfr00semaphore1", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore\n");
        clean();
        return -1;
    }
    if ((write_line = sem_open("/xbolfr00write_line", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore\n");
        clean();
        return -1;
    }
    return 0;
}

void process_riders(int C,int i)
{
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile, "%d\t\t: RID %d\t\t: start\n",*A,i);
    sem_post(write_line);
    sem_wait(semaphore1);
    (*IN_TIME)++;
    (*A)++;
    sem_wait(write_line);
    (*CR)++;
    fprintf(pfile, "%d\t\t: RID %d\t\t: enter: %d\n",*A,i,*IN_TIME);
    sem_post(write_line);
    sem_post(semaphore1);
    sem_wait(bus);
    *IN_TIME=0;
    (*HOW_GET_IN)++;
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile, "%d\t\t: RID %d\t\t: boarding\n",*A,i);
    sem_post(write_line);
    (*CR)--;
    if(*CR == 0 || *HOW_GET_IN == C)
    {
        sem_post(riders_allonaboard);
    }
    else
        sem_post(bus);
    sem_wait(riders_ended);
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile,"%d\t\t: RID %d\t\t: finish\n", *A,i);
    sem_post(write_line);
    exit(0);
}
void process_bus(int *R, int *C, int *ABT)
{
    sem_wait(write_line);
    fprintf(pfile, "%d\t\t: BUS\t\t: start\n",*A);
    sem_post(write_line);
    while(*MAX_IN_BUS <= *R && *ALL_FLAG != 1){
        sem_wait(semaphore1);
        sem_wait(write_line);
        (*A)++;
        fprintf(pfile, "%d\t\t: BUS\t\t: arrival\n",*A);
        sem_post(write_line);
        if(*CR > 0)
        {
            sem_wait(write_line);
            (*A)++;
            fprintf(pfile, "%d\t\t: BUS\t\t: start boarding: %d\n",*A,(*CR > *C ? *C : *CR));
            *HOW_GET_IN = 0;
            sem_post(bus);
            sem_wait(riders_allonaboard);
            *MAX_IN_BUS += *HOW_GET_IN;
            sem_wait(write_line);
            (*A)++;
            fprintf(pfile, "%d\t\t: BUS\t\t: end boarding: 0\n",*A);
            sem_post(write_line);
        }
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: depart\n",*A);
    sem_post(write_line);
    sem_post(semaphore1);
    usleep(rand()%(1000*(*ABT)+1));
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: end\n",*A);
    sem_post(write_line);
    for(int i = 0; i < *HOW_GET_IN; i++)
        sem_post(riders_ended);
    }
    sem_wait(write_line);
    (*A)++;
    fprintf(pfile, "%d\t\t: BUS\t\t: finish\n",*A);
    sem_post(write_line);
    exit(0);
}
void gen_riders(int *R, int C, int *ART)
{
    pid_t R_id[*R];
    for(int i = 1; i <= *R+1; i++)
    {
        usleep(rand()%(1000*(*ART)+1));
        R_id[i] = fork();
        if (R_id[i] == 0)
        {
            process_riders(C,i);
        }
        if (R_id[i] < 0)
        {
            kill(-1*getpid(),SIGTERM);
        }
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
        clean();
        return -1;
    }
    else
    {

    }
    pid_t p_bus = fork();
    if (p_bus == 0)
    {
        //child
        process_bus(&R,&C,&ABT);

    }
    else //p_bus > 0
    {
        //parent
        pid_t riders_generator = fork();
        if (riders_generator == 0)
        {
            //child
            pid_t waitpid;

            gen_riders(&R,C,&ART);
            while((waitpid = wait(0)) > 0);
            *ALL_FLAG=1;
            exit(0);
        }
        else if(riders_generator > 0)
        {
            //parent
            waitpid(riders_generator,NULL,0);
            waitpid(p_bus,NULL,0);
        }
        else
        {
            kill(-1*getpid(),SIGTERM);
        }
    }
    if(p_bus < 0)
        return -1;
    clean();
    fclose(pfile);
    return 0;
}
