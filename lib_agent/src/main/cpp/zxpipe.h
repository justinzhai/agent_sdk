#ifndef __ZX_PIPE__
#define __ZX_PIPE__

//创建pipe
int pipe_init();

int pipe_release();

int pipe_create(int * fd);

void pipe_close(int * fd);

int pipe_write(int fd, const unsigned char * data, int len);

int pipe_read(int fd, char *buf, int bufsize);

#endif	//__ZX_PIPE__


