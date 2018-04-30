/**
* @autor: Rene Bolf
* @date:
* @file: project2.c
* @brief: Implementácia synchronizačného problému The Senate Bus Problem
* @see: http://www.fit.vutbr.cz/study/courses/IOS/public/Lab/projekty/projekt2/projekt2.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/mman.h>
#define SHMAT_ERROR (void*)-1
#define SHM_SIZE sizeof(int)

/**
 * Struktrua zdielanych premmien
 **/
 typedef struct
 {
     int actionC; //pocitadlo operacii
     int count_riders; // pocitadlo riderov na zastavke
     int riders_in_bus; //kolko riderov nastupilo do busu
     int all_generated; //vsetci riders boli vygenerovaní
     int how_many; //
     int in_time;
 }TShm;
TShm *shm;

/*semaphores*/
sem_t *bus = NULL;
sem_t *riders_allonaboard = NULL;
sem_t *finish = NULL;
sem_t *semaphore1 = NULL;
sem_t *write_line = NULL;

FILE *pfile;
int shm_id = 0;


void close_semaphores()
{
   sem_close(bus);
   sem_close(riders_allonaboard);
   sem_close(finish);
   sem_close(semaphore1);
   sem_close(write_line);

   //odalokovanie miesta semaphora
   sem_unlink("/xbolfr00bus");
   sem_unlink("/xbolfr00riders_allonaboard");
   sem_unlink("/xbolfr00finish");
   sem_unlink("/xbolfr00semaphore1");
   sem_unlink("/xbolfr00write_line");
}
void clean()
{

    close_semaphores();

    shmdt(&shm->actionC);
    shmdt(&shm->count_riders);
    shmdt(&shm->riders_in_bus);
    shmdt(&shm->all_generated);
    shmdt(&shm->how_many);
    shmdt(&shm->in_time);

    shmctl(shm_id, IPC_RMID, NULL);

}

int load()
{
    if((shm_id = shmget(IPC_PRIVATE, sizeof(TShm), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Cannot allocate shm!\n");
        clean();
        return -1;
    }
    if((shm = shmat(shm_id, (void*)0,0)) == SHMAT_ERROR)
    {
        fprintf(stderr, "Cannot load shm\n");
        clean();
        return -1;
    }


    if ((bus = sem_open("/xbolfr00bus", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore bus\n");
        clean();
        return -1;
    }

    if ((riders_allonaboard = sem_open("/xbolfr00riders_allonaboard", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore riders_allonaboard\n");
        clean();
        return -1;
    }

    if ((finish = sem_open("/xbolfr00finish", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore finish\n");
        clean();
        return -1;
    }

    if ((semaphore1 = sem_open("/xbolfr00semaphore1", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore1\n");
        clean();
        return -1;
    }

    if ((write_line = sem_open("/xbolfr00write_line", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore write_line\n");
        clean();
        return -1;
    }

    shm->actionC = 1;
    shm->count_riders = 0;
    shm->riders_in_bus = 0;
    shm->all_generated = 0;
    shm->how_many = 0;
    shm->in_time = 0;


return 0;
}


void process_riders(int C, int i)
{

    sem_wait(write_line);
    shm->actionC = shm->actionC + 1;
    fprintf(pfile, "%d: RID %d: start\n",shm->actionC,i);
    sem_post(write_line);

    sem_wait(semaphore1);

    shm->in_time=shm->in_time + 1;
    shm->actionC = shm->actionC + 1;
    sem_wait(write_line);
    shm->count_riders = shm->count_riders + 1;
    fprintf(pfile, "%d: RID %d: enter: %d\n",shm->actionC,i,shm->in_time);
    sem_post(write_line);

    sem_post(semaphore1);
    sem_wait(bus);

    shm->in_time = 0;
    shm->how_many = shm->how_many + 1;
    sem_wait(write_line);

    shm->actionC = shm->actionC + 1;
    fprintf(pfile, "%d: RID %d: boarding\n",shm->actionC,i);
    sem_post(write_line);

    shm->count_riders = shm->count_riders - 1;
    if(shm->count_riders == 0 || shm->how_many == C)
        sem_post(riders_allonaboard);
    else
        sem_post(bus);
    sem_wait(finish);
    sem_wait(write_line);

    shm->actionC = shm->actionC + 1;
    fprintf(pfile,"%d: RID %d: finish\n", shm->actionC,i);
    sem_post(write_line);
    exit(0);
}

void process_bus(int R, int C, int ABT)
{
    sem_wait(write_line);
    fprintf(pfile, "%d: BUS: start\n",shm->actionC);
    sem_post(write_line);

    while(shm->how_many <= R && shm->all_generated != 1)
    {
        sem_wait(semaphore1);

        sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(pfile, "%d: BUS: arrival\n",shm->actionC);
        sem_post(write_line);

        if(shm->count_riders > 0)
        {
            int pom;
            if(C <= shm->count_riders)
                pom = C;
            else
                pom = shm->count_riders;
            sem_wait(write_line);
            shm->actionC = shm->actionC + 1;
            fprintf(pfile, "%d: BUS: start boarding: %d\n",shm->actionC,pom);
            sem_post(write_line);

            shm->how_many = 0;
            sem_post(bus);
            sem_wait(riders_allonaboard);
            shm->riders_in_bus += shm->how_many;

             sem_wait(write_line);
            shm->actionC = shm->actionC + 1;
            fprintf(pfile, "%d: BUS: end boarding: 0\n",shm->actionC);
             sem_post(write_line);
        }
         sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(pfile, "%d: BUS: depart\n",shm->actionC);
         sem_post(write_line);

        sem_post(semaphore1);

        usleep(rand()%(1000*ABT+1));

        sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(pfile, "%d: BUS: end\n",shm->actionC);
        sem_post(write_line);

        for(int i = 0; i < shm->how_many; i++)
            sem_post(finish);
    }
    sem_wait(write_line);
    shm->actionC = shm->actionC + 1;
    fprintf(pfile, "%d: BUS: finish\n",shm->actionC);
    sem_post(write_line);
    exit(0);
}

void gen_riders(int R, int C, int ART)
{
    pid_t R_id[R];

    for(int i = 1; i <= R; i++)
    {
        usleep(rand()%(1000*ART+1));

        R_id[i] = fork();
        if (R_id[i] == 0)
        {
            process_riders(C,i);
        }
        else if (R_id[i] < 0)
        {
            kill(-1*getpid(),SIGTERM);
        }
    }
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
    /*
        ART je max hodnota doby v milisekundach po ktorej je generovany novy proces rider
        ABT je maximalna hodnota doby v milisekundach počas ktorej proces bus simuluje jazdu
    */
    int R, C, ART, ABT;
    int var = params(&R, &C, &ART, &ABT, argv);
    if (var == 1)
        return 1;

    pfile = fopen("proj2.out", "w");
    if(pfile == NULL)
    {
        fprintf(stderr,"Cannot create file.\n");
        return 1;
    }
    setbuf(pfile,NULL);

    if(load() == -1)
    {
        fclose(pfile);
        clean();
        return 1;
    }

    pid_t p_bus = fork();

    if (p_bus == 0)
    {
        //child
        process_bus(R,C,ABT);
    }
    else if (p_bus > 0)//p_bus > 0
    {
        //parent
        pid_t riders_generator = fork();

        if (riders_generator == 0)
        {
            //child
            pid_t waiting;

            gen_riders(R,C,ART);

            while((waiting = wait(0)) > 0);
            shm->all_generated=1;
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
    else
    {
        fclose(pfile);
        clean();
        return 1;
    }

    fclose(pfile);
    clean();
    return 0;
}
