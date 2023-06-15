#include "semx.h"
#include "zxstruct.h"
#include <stdlib.h>
#include <semaphore.h>
#include "system.h"


typedef struct _semx_t
{
	sem_t h;
}semx_t;

semx_t *semaphore_create()
{
	semx_t *sem = (semx_t *)malloc(sizeof(semx_t));
	if (sem == NULL) return sem;
	if (-1 == sem_init(&sem->h, 0, 0)) {
		free(sem);
		return NULL;
	}
	return sem;
}

int semaphore_wait(semx_t *sem)
{
	return sem_wait(&sem->h);
}

int semaphore_post(semx_t *sem)
{
	return sem_post(&sem->h);
}

int semphore_close(semx_t *sem)
{
	if (-1 == sem_destroy(&sem->h))
		return RET_ERROR;
	free(sem);
	return RET_OK;
}
