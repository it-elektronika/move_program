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
    printf("\n Error opening ttyACM0\n");
  }
  else
  {
    printf("\n ttyACM0 Opened Successfully\n");
  }
  
  struct termios SerialPortSettings;	/* Create the structure                          */

  tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

	/* Setting the Baud rate */
  cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
  cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */

	/* 8N1 Mode */
  SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
  SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
  SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
  SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */

  SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
  SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 

  SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
  SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

  SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/

	/* Setting Time outs */


  if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
  {
    printf("ERROR ! in Setting attributes\n");
  }   
  else
  {
    printf("\n  BaudRate = 9600 \n  StopBits = 1 \n  Parity   = none\n");
  }		
  
  char write_buffer[] = "START*";
  
  int i;
  /*
  write_buffer[0] = "S";
  write_buffer[1] = "T";
  write_buffer[2] = "A";
  write_buffer[3] = "R";
  write_buffer[4] = "T";
  */
  //for(i = 0; i < 5; ++i)
  //{
  write(fd, "ATZ\r", 4);
  //}

  close(fd); /* Close the serial port */
}
