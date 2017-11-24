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
  char sendBuff[MAX_STR_LEN];
  int fd;
  memset(&sendBuff[0], 0, MAX_STR_LEN);
  snprintf(&sendBuff[0], MAX_STR_LEN, "START");
  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
  if(fd == -1)
  {
    printf("\n Error opening ttyACM0\n");
  }
  else
  {
    printf("\n ttyACM0 Opened Successfully\n");
  }

  int counter = 0;
  //while(counter < 100000)
  //{
  write(fd, &sendBuff[0], strlen(&sendBuff[0]));
  //  counter++;
  //}
}







