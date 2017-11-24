#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_STR_LEN 7

int main()
{
  char readBuff[MAX_STR_LEN];
  int fd;
  int i = 0;
  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
  while(1)
  {
    ssize_t len;
    memset(&readBuff[0], 0, MAX_STR_LEN);
    len = read(fd, &readBuff[0], MAX_STR_LEN);
     
    for(i = 0; i < len; ++i)
    { 
       //if(readBuff[i] == 'S')
       //{
       //  printf("S WORD\n");	       
       //}	       
       printf("len: %ld, i: %d, %c\n",len,  i, readBuff[i]);
    }
  }
}
