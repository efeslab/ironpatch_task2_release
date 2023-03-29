#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <string.h>
#include "klee.h"
#include "bumper.h"

//////////////////////////////////////////////////////
////////////////// Meaningful stub ///////////////////
//////////////////////////////////////////////////////
// superglue will define these
int* symbolic_ready = NULL;
struct can_frame* symbolic_data = NULL;

// superglue will look at this variable at the end of the execution
struct Bumper decoy_bumper;
int g_i = 0;

//void* mymalloc( size_t size ) {
//	return NULL;
//}

clock_t g_time = 0;

clock_t clock(void)
{
	return (g_time += 120000);
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout)
{
	return symbolic_ready[g_i];
}

ssize_t read(int fd, void *buf, size_t count)
{
	memcpy(buf, symbolic_data + g_i, sizeof(struct can_frame));
	g_i++;
	return sizeof(struct can_frame);
}
//////////////////////////////////////////////////////
////////////////// Empty stub ////////////////////////
//////////////////////////////////////////////////////

int bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
	return 0;
}

FILE *fopen(const char *filename, const char *mode)
{
	return NULL;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	return 0;
}

int socket(int domain, int type, int protocol)
{
	return 0;
}

char *strcpy(char *dest, const char *src)
{
	return NULL;
}

int ioctl(int fd, unsigned long request, ...)
{
       return 0;
}

int setsockopt(int socket, int level, int option_name,
const void *option_value, socklen_t option_len)
{
	return 0;
}

int open(const char *pathname, int flags)
{
	return 0;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
	return 0;
}

int fprintf(FILE *stream, const char *format, ...)
{
	return 0;
}
