#ifndef  __SEMX_H__
#define __SEMX_H__


typedef struct _semx_t semx_t;

semx_t * semaphore_create();
int semaphore_wait(semx_t *sem);
int semaphore_post(semx_t *sem);
int semphore_close(semx_t *sem);


#endif //  __SEMX_H__



