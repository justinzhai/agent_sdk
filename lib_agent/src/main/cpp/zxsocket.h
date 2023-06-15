#ifndef __ZX_SOCKET__
#define __ZX_SOCKET__


//´´½¨·Ç×èÈûsocket
int socket_create_nonblock(int * _fd);

void socket_close(int fd);

int socket_connect(int refer, int fd, const char * szHost, int port);

int socket_connect(int refer, int fd, const struct sockaddr_in * addr, int port);


int socket_read(int fd, char *buf, int bufsize);


int socket_write(int fd, char *buf, int bufsize);


void glob_bind_addr_set(const char *ip);

#endif	//__ZX_SOCKET__


