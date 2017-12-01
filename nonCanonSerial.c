      #include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>
      #include <termios.h>
      #include <stdio.h>
      #include <string.h>     
      #include <stdlib.h>
      #include <errno.h>
      #include <unistd.h>

      #define BAUDRATE B115200
      #define MODEMDEVICE "/dev/ttyUSB0"
      #define _POSIX_SOURCE 1 /* POSIX compliant source */
      #define FALSE 0
      #define TRUE 1
      #define MAX_STR_LEN 255
       


      volatile int STOP=FALSE; 
       
      int main()
      {
        int fd,c, res;
        struct termios oldtio,newtio;
        char buf[255];
        
        fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
        if (fd <0) {perror(MODEMDEVICE); exit(-1); }
        
        tcgetattr(fd,&oldtio); /* save current port settings */
        
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        
        /* set input mode (non-canonical, no echo,...) */
        newtio.c_lflag = 0;
         
        newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
        char sendBuff[MAX_STR_LEN];
        memset(&sendBuff[0], 0, MAX_STR_LEN);
        snprintf(&sendBuff[0], MAX_STR_LEN, "I");
        snprintf(&sendBuff[1], MAX_STR_LEN, "00");
        snprintf(&sendBuff[3], MAX_STR_LEN, "A1");
        snprintf(&sendBuff[5], MAX_STR_LEN, "*");
  
        write(fd, sendBuff, strlen(&sendBuff[0]));

        
      //  while (STOP==FALSE) {       /* loop for input */
      //   res = read(fd,buf,255);   /* returns after 5 chars have been input */
       ///   buf[res]=0;               /* so we can printf... */
       //   printf(":%s:%d\n", buf, res);
       //   if (buf[0]=='*') STOP=TRUE;
       // }
        tcsetattr(fd,TCSANOW,&oldtio);
      }
