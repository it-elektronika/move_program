#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

int counter = 0;
void main()
{
  int fd;
  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
  if(fd == -1)
  {
    printf("\n Error opening ttyACM1\n");
  }
  else
  {
    fcntl(fd, F_SETFL, 0);
    printf("\n ttyACM0 Opened Successfully\n");
  }
  

  //struct termios SerialPortSettings;	/* Create the structure                          */

  //tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

	/* Setting the Baud rate */
  //cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
  //cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */

	/* 8N1 Mode */
 // SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
 // SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
 // SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
 // SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */

 // SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
 // SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 

  //SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
  //SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

  //SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/

	/* Setting Time outs */
  //SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
  //SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */


 // if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
 // {
 //   printf("\n  ERROR ! in Setting attributes");
 // }   
 // else
 // {
 //   printf("\n  BaudRate = 9600 \n  StopBits = 1 \n  Parity   = none\n");
 // }		
  
 

    /*------------------------------- Read data from serial port -----------------------------*/
  
  tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */
  int stop = 0;
  int res;

  char buff[255];   /* Buffer to store the data received              */
  int i = 0;
  while(stop == 0)
  {
    res = read(fd, buff, 255);
    buff[res] = 0;
    printf(":%s:%d\n", buff, res);
    if(buff[0] == "*")
    {
      stop = 1;
    }
  }	  
  //while((bytes_read=read(fd, &read_buffer, 1) > 5))
  //{
  //  printf("READ BUFFER : %c\n",read_buffer[i]);
  //}
  //bytes_read = read(fd,&read_buffer,sizeof(read_buffer)); /* Read the data                   */
  //printf("\n\n  Bytes Rxed -%d", bytes_read); /* Print the number of bytes read */
  //printf("\n\n  ");
  //printf("bytes read: %d\n", bytes_read);
  //for(i=0;i<bytes_read;i++)	 /*printing only the received characters*/
  //{
  //  printf("READ BUFFER : %c\n",read_buffer[i]);
  //}
  
    //printf("\n +----------------------------------+\n\n\n");
  
  close(fd); /* Close the serial port */
}
