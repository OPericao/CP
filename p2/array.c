//Ejercicio 4

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "options.h"

#define DELAY_SCALE 1000
#define MAX_WAIT 10000


struct array {
    int size;
    int *arr;
};

struct thr_args {
    pthread_mutex_t* mutex;
    int id;
    int iterations;
    int delay;
    struct array *arr;
    int* cont;
    pthread_mutex_t* mu;
};

void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}

void* increment(void* p)
{
    int pos, val;

    struct thr_args *arg=p;

    while(1){
        pthread_mutex_lock(arg->mu);
        if(*arg->cont >= arg->iterations){
            pthread_mutex_unlock(arg->mu);
            break;
        }
        *(arg->cont) = *(arg->cont) + 1;
        pthread_mutex_unlock(arg->mu);

        pos = rand() % arg->arr->size;
        pthread_mutex_lock(&arg->mutex[pos]);

        val = arg->arr->arr[pos];
        apply_delay(arg->delay);

        val++;
        apply_delay(arg->delay);

        arg->arr->arr[pos] = val;
        apply_delay(arg->delay);
        printf("%d increasing position %d\n", arg->id, pos);

        pthread_mutex_unlock(&arg->mutex[pos]);
    }
    return 0;
}

void* move(void* p){
    int pos1,pos2,val;
    struct thr_args *arg=p;
    while(1) {
        pthread_mutex_lock(arg->mu);
        if(*arg->cont >= arg->iterations){
            pthread_mutex_unlock(arg->mu);
            break;
        }
        *(arg->cont) = *(arg->cont) + 1;
        pthread_mutex_unlock(arg->mu);

        pos1 = rand() % arg->arr->size;
        pos2 = rand() % arg->arr->size;
        if (pos1 != pos2) {
            while (1) {
                if ((pthread_mutex_trylock(&arg->mutex[pos1])) == 0) {
                    if ((pthread_mutex_trylock(&arg->mutex[pos2])) == 0) {
                        val = arg->arr->arr[pos1];
                        apply_delay(arg->delay);
                        val--;
                        apply_delay(arg->delay);
                        arg->arr->arr[pos1] = val;
                        apply_delay(arg->delay);
                        val = arg->arr->arr[pos2];
                        apply_delay(arg->delay);
                        val++;
                        apply_delay(arg->delay);
                        arg->arr->arr[pos2] = val;
                        apply_delay(arg->delay);
                        printf("Thread %d moving from %d to %d\n", arg->id, pos1, pos2);
                        pthread_mutex_unlock(&arg->mutex[pos1]);
                        pthread_mutex_unlock(&arg->mutex[pos2]);
                        break;
                    } else pthread_mutex_unlock(&arg->mutex[pos1]);
                    usleep(MAX_WAIT);
                }
            }
        }
    }
    return 0;
}

void print_array(struct array arr) {
    int total = 0;

    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}


int main (int argc, char **argv)
{
    struct options       opt;
    struct array         arr;

    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 5;    //5
    opt.size         = 10;   //10
    opt.iterations   = 100;  //100
    opt.delay        = 1000; //1000

    read_options(argc, argv, &opt);

    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));

    memset(arr.arr, 0, arr.size * sizeof(int));

    pthread_mutex_t *auxMutex = calloc(opt.size,sizeof(pthread_mutex_t));
    struct thr_args *arg = calloc((2*opt.num_threads),sizeof(*arg));
    pthread_t *thr = calloc((2*opt.num_threads),sizeof(*thr));

    int *contador = malloc(sizeof(int));
    int *contador2 = malloc(sizeof(int));
    *contador = 0;
    *contador2 = 0;

    pthread_mutex_t *mux = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_t *mux2 = malloc(sizeof(pthread_mutex_t));

    for(int i=0;i<opt.size;i++){
        pthread_mutex_init(&auxMutex[i], NULL);
    }

    for(int i=0;i<2*opt.num_threads;i++) {
        arg[i].mutex = auxMutex;
        arg[i].delay = opt.delay;
        arg[i].iterations = opt.iterations;
        arg[i].arr = &arr;
        arg[i].id = i;
        if (i < opt.num_threads){
            arg[i].cont = contador;
            arg[i].mu = mux;
            pthread_create(&thr[i], NULL, increment, &arg[i]);
        }
        else {
            arg[i].cont = contador2;
            arg[i].mu = mux2;
            pthread_create(&thr[i], NULL, move, &arg[i]);
        }
    }

    for(int i=0;i<2*opt.num_threads;i++){
        pthread_join(thr[i],NULL);
    }

    for(int i=0;i<opt.size;i++) {
        pthread_mutex_destroy(&auxMutex[i]);
    }

    free(auxMutex);
    free(arg);
    free(thr);
    free(contador);
    free(contador2);
    free(mux);
    free(mux2);

    print_array(arr);
    free(arr.arr);
    return 0;
}
