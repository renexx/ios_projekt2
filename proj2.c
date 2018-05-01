/**
* @autor: Rene Bolf
* @date: 1.5.2018
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
#define SHMGET_ERROR -1
#define SLEEP_ART rand()%((ART+1)*1000) //uspanie pre parameter ART
#define SLEEP_ABT rand()%((ABT+1)*1000) //uspanie pre parameter ABT
#define LOCKED 0 // semafor zavrety(cervena) = 0
#define UNLOCKED 1 //semafor otvoreny(zelena) = 1
#define PROCESS_GROUP -1*getpid(),SIGTERM  //kill pre skupinu procesov
#define PROCESS getpid(),SIGTERM //kill pre proces

enum ERROR
{
    ERROR_OK, // vracia 0 ked je vsetko spravne
    ERROR_SHM, //error zdielanej premennej
    ERROR_SEM, //error semaforov
    ERROR_BAD_PARAM, //zle zadane parametre
    ERROR_FILE, //zle nacitany subor
    ERROR_LOAD //zle nacitany load
};
/**
 * Struktrua zdielanych premmien
 **/
 typedef struct
 {
     int actionC; //pocitadlo operacii
     int count_riders; // pocitadlo riderov na zastavke
     int riders_in_bus; //kolko riderov nastupilo do busu
     int all_generated; //vsetci riders boli vygenerovaní
     int how_many;
     int in_time;
 }TShm;
TShm *shm;
#define SHM_SIZE sizeof(TShm)
/*semafory*/
sem_t *bus = NULL;
sem_t *riders_allonaboard = NULL;
sem_t *finish = NULL;
sem_t *semaphore1 = NULL;
sem_t *write_line = NULL; //semafor na zapisovanie do pamati

FILE *output_file;
int shm_id = 0;

void close_semaphores()
{
    //zatvorenie semaforov
   sem_close(bus);
   sem_close(riders_allonaboard);
   sem_close(finish);
   sem_close(semaphore1);
   sem_close(write_line);
   //odalokovanie miesta semaforov
   sem_unlink("/xbolfr00bus");
   sem_unlink("/xbolfr00riders_allonaboard");
   sem_unlink("/xbolfr00finish");
   sem_unlink("/xbolfr00semaphore1");
   sem_unlink("/xbolfr00write_line");
}
void clean()
{
    close_semaphores();
    //detached zdielanych premennych
    shmdt(&shm->actionC);
    shmdt(&shm->count_riders);
    shmdt(&shm->riders_in_bus);
    shmdt(&shm->all_generated);
    shmdt(&shm->how_many);
    shmdt(&shm->in_time);
    //odalokovanie zdielanej premenej
    shmctl(shm_id, IPC_RMID, NULL);
}
int load()
{
    //alokovanie zdielanej premennej
    if((shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666)) == SHMGET_ERROR)
    {
        fprintf(stderr, "Cannot allocate shm_id!\n");
        clean();
        return ERROR_SHM;
    }
    if((shm = shmat(shm_id, NULL ,0)) == SHMAT_ERROR)
    {
        fprintf(stderr, "Cannot allocate shm\n");
        clean();
        return ERROR_SHM;
    }
//inicializacia a otvorenie pomenovanych semaforov
    if ((bus = sem_open("/xbolfr00bus", O_CREAT | O_EXCL, 0666, LOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore bus\n");
        clean();
        return ERROR_SEM;
    }
    if ((riders_allonaboard = sem_open("/xbolfr00riders_allonaboard", O_CREAT | O_EXCL, 0666, LOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore riders_allonaboard\n");
        clean();
        return ERROR_SEM;
    }
    if ((finish = sem_open("/xbolfr00finish", O_CREAT | O_EXCL, 0666, LOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore finish\n");
        clean();
        return ERROR_SEM;
    }
    if ((semaphore1 = sem_open("/xbolfr00semaphore1", O_CREAT | O_EXCL, 0666, UNLOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore1\n");
        clean();
        return ERROR_SEM;
    }
    if ((write_line = sem_open("/xbolfr00write_line", O_CREAT | O_EXCL, 0666, UNLOCKED)) == SEM_FAILED)
    {
        fprintf(stderr, "Cannot load semaphore write_line\n");
        clean();
        return ERROR_SEM;
    }
//inicializacia zdielanych premennych
    shm->actionC = 1;
    shm->count_riders = 0;
    shm->riders_in_bus = 0;
    shm->all_generated = 0;
    shm->how_many = 0;
    shm->in_time = 0;
return ERROR_OK;
}
void process_riders(int C, int i)
{
    sem_wait(write_line);
    shm->actionC = shm->actionC + 1;
    fprintf(output_file, "%d\t\t: RID %d\t\t: start\n",shm->actionC,i);
    sem_post(write_line);

    sem_wait(semaphore1);

    shm->in_time=shm->in_time + 1;
    shm->actionC = shm->actionC + 1;
    sem_wait(write_line);
    shm->count_riders = shm->count_riders + 1;
    fprintf(output_file, "%d\t\t: RID %d\t\t: enter: %d\n",shm->actionC,i,shm->in_time);
    sem_post(write_line);

    sem_post(semaphore1);
    sem_wait(bus);

    shm->in_time = 0;
    shm->how_many = shm->how_many + 1;
    sem_wait(write_line);

    shm->actionC = shm->actionC + 1;
    fprintf(output_file, "%d\t\t: RID %d\t\t: boarding\n",shm->actionC,i);
    sem_post(write_line);

    shm->count_riders = shm->count_riders - 1;
    if(shm->how_many == C  || shm->count_riders == 0)
        sem_post(riders_allonaboard);
    else
        sem_post(bus);
    sem_wait(finish);
    sem_wait(write_line);

    shm->actionC = shm->actionC + 1;
    fprintf(output_file,"%d\t\t: RID %d\t\t: finish\n", shm->actionC,i);
    sem_post(write_line);
    exit(0);
}
void process_bus(int R, int ABT)
{
    sem_wait(write_line);
    fprintf(output_file, "%d\t\t: BUS\t\t: start\n",shm->actionC);
    sem_post(write_line);
    while(1)
    {
    if(shm->all_generated == 1 || R == shm->how_many) break;
        sem_wait(semaphore1);

        sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(output_file, "%d\t\t: BUS\t\t: arrival\n",shm->actionC);
        sem_post(write_line);

        if(shm->count_riders > 0)
        {
            sem_wait(write_line);
            shm->actionC = shm->actionC + 1;
            fprintf(output_file, "%d\t\t: BUS\t\t: start boarding: %d\t\n",shm->actionC,shm->count_riders);
            sem_post(write_line);

            shm->how_many = 0;
            sem_post(bus);
            sem_wait(riders_allonaboard);
            shm->riders_in_bus += shm->how_many;

            sem_wait(write_line);
            shm->actionC = shm->actionC + 1;
            fprintf(output_file, "%d\t\t: BUS\t\t: end boarding: %d\n",shm->actionC,shm->count_riders);
            sem_post(write_line);
        }
        sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(output_file, "%d\t\t: BUS\t\t: depart\n",shm->actionC);
        sem_post(write_line);

        sem_post(semaphore1);
        usleep(SLEEP_ABT);

        sem_wait(write_line);
        shm->actionC = shm->actionC + 1;
        fprintf(output_file, "%d\t\t: BUS\t\t: end\n",shm->actionC);
        sem_post(write_line);

        for(int i = 0; i < shm->how_many; i++)
            sem_post(finish);
    }
    sem_wait(write_line);
    shm->actionC = shm->actionC + 1;
    fprintf(output_file, "%d\t\t: BUS\t\t: finish\n",shm->actionC);
    sem_post(write_line);
    exit(0);
}

void gen_riders(int R, int C, int ART)
{
    srand(time(NULL));
    pid_t R_id[R];
    for(int i = 1; i <= R; i++)
    {
        usleep(SLEEP_ART);
        R_id[i] = fork();
        if (R_id[i] == 0)
        {
            process_riders(C,i);
        }
        else if (R_id[i] < 0)
        {
            kill(PROCESS_GROUP);
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
        return ERROR_OK;
    else
    {
        fprintf(stderr, "Parametre su zadane v zlych intervaloch alebo niesu cele cisla\n");
        return ERROR_BAD_PARAM;
    }
}
int main (int argc, char *argv[])
{
    if(argc != 5)
    {
        fprintf(stderr, "Nespravny pocet argumentov.\n");
        return ERROR_BAD_PARAM;
    }
    /*
        ART je max hodnota doby v milisekundach po ktorej je generovany novy proces rider
        ABT je maximalna hodnota doby v milisekundach počas ktorej proces bus simuluje jazdu
    */
    int R, C, ART, ABT;
    int var = params(&R, &C, &ART, &ABT, argv);
    if (var == ERROR_BAD_PARAM)
        return ERROR_BAD_PARAM;

    output_file = fopen("proj2.out", "w");
    if(output_file == NULL)
    {
        fprintf(stderr,"Cannot create file.\n");
        return ERROR_FILE;
    }
    setbuf(output_file,NULL);

    if(load() != ERROR_OK)
    {
        fclose(output_file);
        clean();
        return ERROR_LOAD;
    }
    pid_t p_bus = fork();
    if (p_bus == 0)
    {
        //child
        process_bus(R,ABT);
    }
    else if (p_bus > 0)
    {
        //parent
        pid_t riders_generator = fork();
        if (riders_generator == 0)
        {
            //child
            gen_riders(R,C,ART);
            while(wait(NULL) > 0);
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
           kill(PROCESS);
        }
    }
    else
    {
        fclose(output_file);
        clean();
        return EXIT_FAILURE;
    }

    fclose(output_file);
    clean();
    return ERROR_OK;
}
