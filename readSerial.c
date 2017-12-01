#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_STR_LEN 255

int setTermiosInterface(int fd, int speed, int parity, int waitTime)
{
  int isBlockingMode;
  struct termios tty;

  isBlockingMode = 0;
  if(waitTime < 0 || waitTime > 255)
  {
    isBlockingMode = 1;
  }

  memset(&tty, 0, sizeof(tty));
  if(tcgetattr(fd, &tty) != 0)
  {
    printf("ERROR: %s\n", strerror(errno)); 
    return -1;
  }
  
  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

  tty.c_iflag &= ~IGNBRK;

  tty.c_lflag = 0;

  tty.c_oflag = 0;

  tty.c_cc[VMIN] = (1 == isBlockingMode) ? 1 : 0;
  tty.c_cc[VTIME] = (1 == isBlockingMode) ? 0 : waitTime;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_cflag |= (CLOCAL |CREAD);

  tty.c_cflag &= ~(PARENB | PARODD);

  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if(tcsetattr(fd, TCSANOW, &tty) != 0)
  {
    printf("ERROR: %s\n", strerror(errno));	  
    return -1;
  }
  return 0;
}


int main()
{
  char readBuff[MAX_STR_LEN];
  int fd;
  int i = 0;
  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  setTermiosInterface(fd,11520, 0, 20);
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
