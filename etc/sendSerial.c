#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
//#define MAX_STR_LEN 38
#define MAX_STR_LEN 6


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
  char readBuff[MAX_STR_LEN];
  int fd;
  memset(&sendBuff[0], 0, MAX_STR_LEN);
 /*
  snprintf(&sendBuff[0], MAX_STR_LEN, "I");
  snprintf(&sendBuff[1], MAX_STR_LEN, "00");
  snprintf(&sendBuff[3], MAX_STR_LEN, "CY");
  snprintf(&sendBuff[5], MAX_STR_LEN, "100000.000");
  snprintf(&sendBuff[15], MAX_STR_LEN, "4294967295");
  snprintf(&sendBuff[25], MAX_STR_LEN, "1");
  snprintf(&sendBuff[26], MAX_STR_LEN, "1*");
  snprintf(&sendBuff[27], MAX_STR_LEN, "1*");
  snprintf(&sendBuff[28], MAX_STR_LEN, "100");
  snprintf(&sendBuff[31], MAX_STR_LEN, "010");
  snprintf(&sendBuff[34], MAX_STR_LEN, "0");
  snprintf(&sendBuff[35], MAX_STR_LEN, "1");
  snprintf(&sendBuff[36], MAX_STR_LEN, "*");
*/
  
  
/*  
  sendBuff[0] = 0x00;
  sendBuff[1] = 0x03;
  sendBuff[2] = 0x00;
  sendBuff[3] = 0x02;
  sendBuff[4] = 0x03;
  sendBuff[5] = 0x07;
  sendBuff[6] = 0x00;
  sendBuff[7] = 0x00;
  sendBuff[8] = 0x00;
*/
  sendBuff[0] = 0x49;
  sendBuff[1] = 0x30;
  sendBuff[2] = 0x30;
  sendBuff[3] = 0x46;
  sendBuff[4] = 0x57;
  sendBuff[5] = 0x2a;
  
  
  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
 // setTermiosInterface(fd,115200, 0, 20);

  for(int i = 0; i < sizeof(sendBuff); i++)
  {
    printf("%c\n", sendBuff[i]);
  }

  if(fd == -1)
  {
    printf("\n Error opening USB0\n");
  }
  else
  {
    printf("\n ttyACM0 Opened Successfully\n");
  }

  if(!write(fd, sendBuff, strlen(&sendBuff[0])))
  {
    printf("Error writing to USB\n");
  }
  else
  {
    printf("Data sent successfully\n");
  }
  while(1)
  {
    ssize_t len;
    memset(&readBuff[0], 0, MAX_STR_LEN);
    len = read(fd, &readBuff[0], MAX_STR_LEN);
    int i;
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







