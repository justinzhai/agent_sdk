#include "zxsocket.h"
#include "zxstruct.h"
#include "zxlog.h"
#include "string.h"
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>

int is_bind_addr_set = 0;
char bind_addr[32] = { 0 };

void socket_close(int fd) 
{
	close(fd);
}

int socket_set_noblock(int fd)
{
	return fcntl(fd, F_SETFL, O_NONBLOCK);
}

int socket_create_nonblock(int *_fd)
{
	struct sockaddr_in server_addr;

	int fd;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		LOGE("socket_create socket error = %d", errno);
		return RET_ERROR;
	}

	// socket bind ip 
	if (is_bind_addr_set) {
		LOGE("bind ip [%s]", bind_addr);
		memset(&server_addr, 0, sizeof(server_addr));

		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(bind_addr);
		server_addr.sin_port = htons(0);

		int ret = bind(fd, (sockaddr *)&server_addr, sizeof(server_addr));
		if (ret != 0) {
			LOGE("socket bind error error = %d", errno);
		}
	}

	int ret = socket_set_noblock(fd);
	if (ret == RET_ERROR) {
		socket_close(fd);
		LOGE("socket_create fcntl error = %d", errno);
		return RET_ERROR;
	}
	*_fd = fd;
	return RET_OK;
}

int socket_connect(int refer, int fd, const char *szHost, int port)
{
	if (NULL == szHost || 0 == port) {
		LOGE("socket_connect param error");
		return RET_ERROR;
	}

#if defined(linux)
	struct timeval tv;
	gettimeofday(&tv,NULL);
	LOGI("id: %3d, dns host: %s", refer, szHost);
#endif

	struct sockaddr_in addr_dst = { 0 };
	struct hostent *host = gethostbyname(szHost);

#if defined(linux)
	struct timeval tv1;
	gettimeofday(&tv1,NULL);
	LOGI("id: %3d, dns const: %d", refer, int(tv1.tv_sec - tv.tv_sec)*1000 + int(tv1.tv_usec-tv.tv_usec)/1000);
#endif
	if (NULL == host) {
		LOGE("socket_connect gethostbyname error");
		return RET_ERROR;
	}

	addr_dst.sin_family	= AF_INET;
	addr_dst.sin_port	= htons(port); //接收端端口
	addr_dst.sin_addr	= *((struct in_addr*)host->h_addr);

	int ret = connect(fd, (struct sockaddr *)&addr_dst, sizeof(addr_dst));
	if (ret < 0) {
		if (errno != EINPROGRESS) {
			LOGE("socket_connect connect error ret = %d fd = %d errno = %d", ret, fd, errno);
			return RET_ERROR;
		}
	}
	return RET_OK;
}

int socket_connect(int refer, int fd, const struct sockaddr_in *addr, int port)
{
	if (NULL == addr || 0 == port) {
		LOGE("socket_connect param error");
		return RET_ERROR;
	}

	struct sockaddr_in addr_dst = { 0 };

	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(port); //接收端端口
	addr_dst.sin_addr = addr->sin_addr;

	int ret = connect(fd, (struct sockaddr *)&addr_dst, sizeof(addr_dst));
	if (ret < 0) {
		if (errno != EINPROGRESS) {
			LOGE("socket_connect connect error ret = %d fd = %d errno = %d", ret, fd, errno);
			return RET_ERROR;
		}
	}
	return RET_OK;
}

int socket_read(int fd, char *buf, int bufsize)
{
	return read(fd, buf, bufsize);
}

int socket_write(int fd, char *buf, int bufsize)
{
	return write(fd, buf, bufsize);
}

void glob_bind_addr_set(const char *ip)
{
	is_bind_addr_set = 1;
	strcpy(bind_addr, ip);
}