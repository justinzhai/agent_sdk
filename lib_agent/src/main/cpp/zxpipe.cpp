#include "zxpipe.h"
#include "zxsocket.h"
#include "zxstruct.h"
#include "zxlog.h"
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int pipe_init()
{
	return RET_OK;
}

int pipe_release()
{
	return RET_OK;
}

int pipe_create(int * fd)
{
	if (pipe(fd) < 0) return RET_ERROR;
	return RET_OK;
}

void pipe_close(int * fd)
{
	if (fd[0] != 0) close(fd[0]);
	if (fd[1] != 0) close(fd[1]);
}

int pipe_write(int fd, const unsigned char * data, int len)
{
	int offset = 0;
	while (true) {
		int write_len = socket_write(fd, (char *)data + offset, len - offset);
		if (write_len < 0) break;
		offset += write_len;
		if (offset == len) break;
	}
	return offset;
}

int pipe_read(int fd, char *buf, int bufsize)
{
	return socket_read(fd, buf, bufsize);
}