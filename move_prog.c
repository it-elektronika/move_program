
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "rs232.h"
#include "func_init.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600
#define DROWSAPP "71395"

SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
SDL_Texture *texture = NULL;
SDL_Color textColor = {255, 255, 255, 255};
SDL_Event event;
SDL_Point touchLocation = {-1, -1};
SDL_Rect fillRect = {0, 0, 0, 0};
TTF_Font *textFont = NULL;

FILE *fp;

struct editbox
{
  int x;
  int y;
  int w;
  int h;
  int posX;
  int posY;
  int hasFocus;
  char value[5];
};

int PiControlHandle_g = -1;
int start = 0;
int stop = 0;
int cycleCounter = 0;
int cycleCheck = 0;

int riseDetected = 0;
int fallDetected = 0;
int timer1 = 10;
int timer2 = 2;
int timestamp = 0;
int oldtimestamp = 0;
int rows = 24;
int columns = 32;
int program;

char passText[5];

char buffRows[20];
char buffColumns[20];
char numBuff[10];

struct editbox eb[10000];
int editBox_id_counter;

int curX = 0;
int curY = 0;

char buffX[20];
char buffY[20];
char buffCount[20];
int textureWidth = 0;
int textureHeight = 0;

int i = 0;
int move_count = 0;
int pattern_move_count = 1;
int pattern2_move_count = 1;

int dir_move;

int holes;

int cport_nr=16;        /* /dev/ttyS0 (COM1 on windows) */
int bdrate=115200;       /* 9600 baud */

int n;
int received = 0;
char mode[]={'8','N','1',0};
char str[24][512];
unsigned char buf[4096];

int pageNumber;

int flags;
int innited;

int init()    /* things needed to start sdl2 properly */
{  
  flags = IMG_INIT_JPG|IMG_INIT_PNG;
  innited = IMG_Init(flags);
  	
  if((SDL_Init(SDL_INIT_VIDEO||SDL_INIT_EVENTS)) != 0)
  {
    SDL_Log("Unable to initialize SDL:%s ", SDL_GetError());
    return 1;                                                                               
  }  

  window = SDL_CreateWindow("IT-Elektronika", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
  
  if (window == NULL)
  {
    return 1;
  }		  

  renderer = SDL_CreateRenderer(window, - 1, SDL_RENDERER_SOFTWARE);
  if(renderer == NULL)
  {
    ;
    //printf("RENDERER IS NULL\n");
  }

  if((innited&flags) != flags)
  {
    printf("IMG_INIT: Failed to init required jpg an png support!\n");
    printf("IMG_INIT: %s\n", IMG_GetError());
  }

  if(TTF_Init() == -1)
  {
    printf("TTF ERROR\n");
  }
  textFont = TTF_OpenFont("BebasNeue Bold.ttf", 60);
  
  return 15;
}

int comm_init()
{
  strcpy(str[0], "I00HT*");
  
  strcpy(str[1], "I00CX080000.000000000120011125501001*");     /* X plus */

  strcpy(str[2], "I00CX080000.000000000120001125501001*");     /* X minus */
  
  strcpy(str[3], "I00CY080000.000000000099701125501001*");     /* Y plus */
  
  strcpy(str[4], "I00CY080000.000000000099711125501001*");     /* Y minus */
  
  strcpy(str[5], "I00CX000550.000100000120001110001001*");    /* HOME X */

  strcpy(str[6], "I00CY000550.000100000120011110001001*");    /* HOME Y */
  
  strcpy(str[7], "I00CX010000.000000000020011110001001*");    /* HOME X - move off sensor */

  strcpy(str[8], "I00CY010000.000000000020001110001001*");    /* HOME Y -  move off sensor*/
  
  strcpy(str[9], "I00CX000900.000000000510011110001001*");    /* PARK X - to be defined */  
  
  strcpy(str[10],"I00CY000900.000000000270001110001001*");    /* PARK Y - to be defined*/
  
  strcpy(str[11], "I00SX*");  /* START X AXIS */
  
  strcpy(str[12], "I00SY*");  /* START Y AXIS */
  
  strcpy(str[13], "I00KX1*"); /* ENABLE X limit switch */
  
  strcpy(str[14], "I00KY1*"); /* ENABLE Y limit switch */

  strcpy(str[15], "I00KX0*"); /* DISABLE X limit switch */
  
  strcpy(str[16], "I00KY0*"); /* DISABLE Y limit switch */
  
  strcpy(str[17], "I00TX*"); /* STOP Y */
  
  strcpy(str[18], "I00TY*"); /* STOP X */

  strcpy(str[19], "N*"); /* RESET driver */

  strcpy(str[20], "I00TA*");  /* STOP all axes*/
  
  strcpy(str[21], "I00KS1*");

  strcpy(str[22], "I00CX000900.000000000510001110001001*");    /* PARK X - to be defined */  
  
  strcpy(str[23], "I00CY000900.000000000270011110001001*");    /* PARK Y - to be defined*/ 


  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");

    return(0);
  }
  return 1;
}

void freeTexture(void)  /* taking care of memory */
{
  if(texture != NULL)
  {
    SDL_DestroyTexture(texture);	  
    texture = NULL;
    textureWidth = 0;
    textureHeight = 0;
  }
}

void draw(void)     /* drawing background */
{
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
}

void render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) /* render something(object) to screen */
{
  SDL_Rect renderQuad;
  renderQuad.x = x;
  renderQuad.y = y;
  renderQuad.w = textureWidth;
  renderQuad.h = textureHeight;

  if(clip != NULL)
  {
    renderQuad.w = clip -> w;
    renderQuad.h = clip -> h;
  }
  SDL_RenderCopyEx(renderer, texture, clip, &renderQuad, angle, center, flip);
}

void drawTextBox(int i, int x, int y, int w, int h)  /*drawing one text box */
{
  if(eb[i].hasFocus == 0)
  {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, x, y, (x+w), y);
    SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
    SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
    SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  }
  if(eb[i].posX == curX && eb[i].posY == curY)
  {
    SDL_Rect fillRect = {x, y, w, h};  
    SDL_SetRenderDrawColor(renderer, 110, 70, 200, 70);
    SDL_RenderFillRect(renderer, &fillRect);
    SDL_RenderDrawLine(renderer, x, y, (x+w), y);
    SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
    SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
    SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  }  
}


void drawEbGrid(void)   /* drawing many textboxes */
{
  int i;
  
  for(i = 0; i < holes; ++i)
  {
    drawTextBox(i, eb[i].x, eb[i].y, eb[i].w, eb[i].h);
  }
}

int writeText(const char *text, SDL_Color textColor)  /* text to be written, after the call to this function a render function call must be made */
{
 	
  SDL_Surface* textSurface;
  
  textSurface = TTF_RenderText_Solid(textFont, text, textColor);

  freeTexture();

  if(textSurface == NULL)
  {
    printf("Unable to render text surface! SDL Error: %s\n", SDL_GetError());
  }
  else
  {
    texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if(texture == NULL)
    {
      printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
    }
    else
    {
      textureWidth = textSurface -> w;
      textureHeight = textSurface -> h;
    }
    SDL_FreeSurface(textSurface);
  }
  return texture != NULL;
}

void drawButton(int x,  int y, int w, int h)    /* button to start movement - obsolete - will use physical button */
{
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y, (x+w), y);
  SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
  SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
  SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  writeText("START: ", textColor);
  render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
  if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp)
  {
    start = 1;
  }
}

void eventUpdate()   /* handling touch events */
{
  while(SDL_PollEvent(&event) != 0 )
  {
    
    if(event.type == SDL_FINGERDOWN)  
    {
      
      timestamp = event.tfinger.timestamp;
      touchLocation.x = event.tfinger.x;
      touchLocation.y = event.tfinger.y;
    }
     
    if(event.type == SDL_QUIT)
    {
      program = 0;
    }
    
  }
}

void initVars(int x, int y, int w, int h)  /* handling struct for drawing grids */
{
  int drawRows = 0;
  int drawColumns = 0;
  int counter = 1;
  
  editBox_id_counter = 0;

  for(drawRows = 0; drawRows < rows ; ++drawRows)
  {
    for(drawColumns = 0; drawColumns < columns; ++drawColumns)
    {
      eb[editBox_id_counter].x = x;
      eb[editBox_id_counter].y = y;
      eb[editBox_id_counter].w = w;
      eb[editBox_id_counter].h = h;
      eb[editBox_id_counter].hasFocus = 0;
      eb[editBox_id_counter].posX = drawColumns;
      eb[editBox_id_counter].posY = drawRows;
      editBox_id_counter++;
      x = x + w;
      counter++;
    }
    x = 50;
    y = y - h;  
  }
}

void command(int number)  /*  sending serial communication command */
{
  RS232_cputs(cport_nr, str[number]);
  n = 0;
  buf[0] ='\0';
  received = 0;
  usleep(100000);
  
  n = RS232_PollComport(cport_nr, buf, 4095);
  
 /*
  while(received == 0)
  {
    if(n > 0)
    {
      buf[n] = 0;   // always put a "null" at the end of a string! 

      for(i=0; i < n; i++)
      {
        if(buf[i] < 32)  // replace unreadable control-codes by dots 
        {
          buf[i] = '.';
        }
      }
      printf("received %i bytes: %s\n", n, (char *)buf);
    }
    received = 1;
  }*/ 
}

void detectRise()
{
  int currentValue;
  int oldValue;
  currentValue = 0;
  oldValue = 0;
  riseDetected = 0;
  currentValue = readVariableValue("I_7");
  if(currentValue != oldValue)
  {
    if(currentValue == 1)
    {
      riseDetected = 1;
    }
    else
    {
      riseDetected = 0;
    }
    oldValue = currentValue;  
  }
}

void detectFall()
{
  int currentValue;
  int oldValue;
  currentValue = 1;
  oldValue = 1;
  fallDetected = 0;
  currentValue = readVariableValue("I_7");
  
  if(currentValue != oldValue)
  {
    if(currentValue == 0)
    {
      fallDetected = 1;
    } 
    else
    {
      fallDetected = 0;
    }
    oldValue = currentValue; 
  }
}

void handle_spring()
{
  int stage = 1;
  if(move_count < holes)
  {
    usleep(800000);

    while(stage == 1)
    {
      if(readVariableValue("I_3")==1)
      {
        stop = 1;
        break;
      }
      //timer
      struct timespec start, stop;
      double accum;
      clock_gettime(CLOCK_REALTIME, &start);
      while(!riseDetected)
      {
        detectRise();

        clock_gettime(CLOCK_REALTIME, &stop);

        accum = ( stop.tv_sec - start.tv_sec )
            + ( stop.tv_nsec - start.tv_nsec )
              / BILLION;
        
        if(accum > timer1)   /* preset timer1 */
        { 
          printf("PAUSE MODE - 10 seconds and no spring\n");
          while(!riseDetected)
          {            
            detectRise();  /* waiting for spring to come */
            draw();	
            drawEbGrid();
    
            writeText("PROGRAM MIRUJE", textColor);
            render(750, 400, NULL, 0.0, NULL, SDL_FLIP_NONE);

            writeText("CAKA NA VZMET", textColor);
            render(750, 500, NULL, 0.0, NULL, SDL_FLIP_NONE);
    
            SDL_RenderPresent(renderer);
            SDL_RenderClear(renderer);
          }
        }
      }
      stage = 2;
    }

    if(!stop)
    {
      writeVariableValue("O_2", 1);
      usleep(300000); 
      writeVariableValue("O_1", 1);
    } 
  
    while(stage == 2)
    {
      if(readVariableValue("I_3")==1)
      {
        stop = 1;
        break;
      }
      while(!fallDetected)
      {
        detectFall();
        clock_gettime(CLOCK_REALTIME, &stop);

        accum = ( stop.tv_sec - start.tv_sec )
            + ( stop.tv_nsec - start.tv_nsec )
              / BILLION;
        
        if(accum > timer2)   /* preset timer2 */
        { 
          while(!fallDetected)
          {
            detectFall();  /* waiting for spring to fall into place */
            draw();	
            drawEbGrid();
    
            writeText("PROGRAM MIRUJE", textColor);
            render(750, 400, NULL, 0.0, NULL, SDL_FLIP_NONE);

            writeText("CAKA DA PADE VZMET V BL.", textColor);
            render(700, 500, NULL, 0.0, NULL, SDL_FLIP_NONE);
    
            SDL_RenderPresent(renderer);
            SDL_RenderClear(renderer);
          }
        }
      }
      stage = 3;
    }
    if(!stop)
    { 
      usleep(500000);  
      writeVariableValue("O_2", 0);
      writeVariableValue("O_1", 0); 
    }
  }
}

void up()                     /*movement up */
{
  if(pageNumber != 5) 
  {
    handle_spring();
  }
  if(!stop)
  {
    curY++;	
  
    command(3);
    command(12);
  }
}
void down()                    /*movement down */
{
  if(pageNumber != 5) 
  {
    handle_spring();
  }
  if(!stop)
  {
    curY--;	
  
    command(4);
    command(12);
  } 
}
void left()                    /*movement left */
{
  if(pageNumber != 5) 
  {
    handle_spring();
  }
  if(!stop)
  {
    curX--;	

    command(2);
    command(11);
  } 
}
void right()                   /*movement right */
{
  if(pageNumber != 5) 
  {
    handle_spring();
  }
  if(!stop)
  {
    curX++;
  
    command(1);
    command(11);
  }
}

void home()                /*moving to home position */
{
  int x_triggered = 0;
  int y_triggered = 0;
  
  writeText("REF. POZICIJA", textColor);
  render(750, 300, NULL, 0.0, NULL, SDL_FLIP_NONE);
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
  
  command(15);
  command(16);
  command(5);             /* x home move */
  command(11);            /* start x home move */
  
  while(x_triggered == 0)
  {
    readVariableValue("I_5");   /* read X - limit switch */
    
    if(readVariableValue("I_5") == 1)
    {
      x_triggered = 1;
    }
    if(readVariableValue("I_3") == 1)
    {
      stop = 1;
      break;
    }
  }

  if(!stop)
  {
    command(19);
    usleep(1000000);
    command(7);      /* x move off limit */
    command(11);     /* start x move off limit */ 
    usleep(1000000);
  
  
    command(6);      /* y home move */
    command(12);     /* start y home move */
  
    while(y_triggered == 0)
    {
      readVariableValue("I_6"); /* read Y - limit switch */
    
      if(readVariableValue("I_6") == 1)
      {
        y_triggered = 1;
      }
      if(readVariableValue("I_3") == 1)
      {
        stop = 1;
        break;
      }
    }
    command(19);
    usleep(1000000);
    command(8);      /* y move off limit */
    command(12);     /* start y move off limit */
    curX = 0;
    curY = 0;
    move_count = 0;
    sleep(1);
  }
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
}



void up_button(int x,  int y)    /* drawing button to go up */
{
  SDL_Surface *imageSurface;
  freeTexture();
  imageSurface = IMG_Load("/home/pi/move_program/up.png");

  if(imageSurface == NULL)
  {
    printf("Unable to render image surface! SDL_ImageError: %s\n", IMG_GetError());
  }
  else
  {
    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if(texture == NULL)
    {
      printf("Unable to create texture from rendered image! SDL_ImageError: %s\n", SDL_GetError());
    }
    else
    {
      textureWidth = imageSurface -> w;
      textureHeight = imageSurface -> h;
    }
    SDL_FreeSurface(imageSurface);
  }

  render(x, y, NULL, 0.0, NULL, SDL_FLIP_NONE);
  
  if(touchLocation.x > x && touchLocation.x < x+100 && touchLocation.y > y && touchLocation.y < y + 100 && timestamp > oldtimestamp)
  {
    down();
    curY++;
  }
}

void down_button(int x,  int y)       /* drawing button to go down */
{
  SDL_Surface *imageSurface;
  freeTexture();
  imageSurface = IMG_Load("/home/pi/move_program/down.png");

  if(imageSurface == NULL)
  {
    printf("Unable to render image surface! SDL_ImageError: %s\n", IMG_GetError());
  }
  else
  {
    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if(texture == NULL)
    {
      printf("Unable to create texture from rendered image! SDL_ImageError: %s\n", SDL_GetError());
    }
    else
    {
      textureWidth = imageSurface -> w;
      textureHeight = imageSurface -> h;
    }
    SDL_FreeSurface(imageSurface);
  }	
  render(x, y, NULL, 0.0, NULL, SDL_FLIP_NONE);

  if(touchLocation.x > x && touchLocation.x < x+100 && touchLocation.y > y && touchLocation.y < y + 100 && timestamp > oldtimestamp)
  {
    up();
    curX++;
  }
}

void left_button(int x,  int y)              /* drawing button to go left */
{
  SDL_Surface *imageSurface;
  freeTexture();
  imageSurface = IMG_Load("/home/pi/move_program/left.png");

  if(imageSurface == NULL)
  {
    printf("Unable to render image surface! SDL_ImageError: %s\n", IMG_GetError());
  }
  else
  {
    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if(texture == NULL)
    {
      printf("Unable to create texture from rendered image! SDL_ImageError: %s\n", SDL_GetError());
    }
    else
    {
      textureWidth = imageSurface -> w;
      textureHeight = imageSurface -> h;
    }
    SDL_FreeSurface(imageSurface);
  }
  
  render(x, y, NULL, 0.0, NULL, SDL_FLIP_NONE);

  if(touchLocation.x > x && touchLocation.x < x+100 && touchLocation.y > y && touchLocation.y < y + 100 && timestamp > oldtimestamp)
  {
    right();
    curX--;
  }
}

void right_button(int x,  int y)       /* drawing button to go right */
{
  SDL_Surface *imageSurface;
  freeTexture();
  imageSurface = IMG_Load("/home/pi/move_program/right.png");

  if(imageSurface == NULL)
  {
    printf("Unable to render image surface! SDL_ImageError: %s\n", IMG_GetError());
  }
  else
  {
    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if(texture == NULL)
    {
      printf("Unable to create texture from rendered image! SDL_ImageError: %s\n", SDL_GetError());
    }
    else
    {
      textureWidth = imageSurface -> w;
      textureHeight = imageSurface -> h;
    }
    SDL_FreeSurface(imageSurface);
  }
  render(x, y, NULL, 0.0, NULL, SDL_FLIP_NONE);

  if(touchLocation.x > x && touchLocation.x < x+100 && touchLocation.y > y && touchLocation.y < y + 100 && timestamp > oldtimestamp)
  {
    left();
    curX--;
  }
}

void button(int x, int y, int w, int h, const char* text, int gotoNum) /* menu button - move to another page (settings/manual) */
{
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y, (x+w), y);
  SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
  SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
  SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  writeText(text, textColor);
  render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
  if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp)
  {
    pageNumber = gotoNum;
  }
}

void button_save(int x, int y, int w, int h)  /* save row/column data button */
{
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y, (x+w), y);
  SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
  SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
  SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  writeText("SHRANI", textColor);
  render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
  if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp)
  {
    system("rm param.txt");	  
    fp = fopen("/home/pi/move_program/param.txt", "w");
    fprintf(fp,"%d\n", rows);
    fprintf(fp, "%d\n", columns);  
    fclose(fp);
  }
}

void keypad(int x, int y, int w, int h)  /* drawing keypad for entering password */
{
  int i = 1;
  int j = 1;
  int k = 0;
  int nums = 1;
  int origX = x;
  int origY = y;
  
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y-h, (x+w*3), y-h);
  SDL_RenderDrawLine(renderer, (x+w*3), y-h, (x+w*3), y); 
  SDL_RenderDrawLine(renderer, (x+w*3), y, x, y);
  SDL_RenderDrawLine(renderer, x, y, x, y-h);
  writeText(passText, textColor);
  render(x, y - h , NULL, 0.0, NULL, SDL_FLIP_NONE); 

  for(j = 1; j < 4; ++j)
  {  
    for(i = 1; i < 4; ++i)
    {
      SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderDrawLine(renderer, x, y, (x+w), y);
      SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
      SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
      SDL_RenderDrawLine(renderer, x, (y+h), x, y);
      sprintf(numBuff, "%d", nums);
      writeText(numBuff, textColor);
      render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
      if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp && cycleCheck != cycleCounter)
      {
        cycleCheck = cycleCounter;
        if(passText[4] == '\0')
        {
          strcat(passText, numBuff);
        }
      }
      if(passText[4] != '\0')
      {
        int ret;
        ret = strcmp(DROWSAPP, passText);
        if(ret == 0)
        {
          pageNumber = 3;  /* move to page_manual */
        }
        memset(&passText[0], 0, 5);
      }
      x = x + w;
      nums++;
    }
    x = origX;
    y = y + h;
  }
  y = origY;
}

void admin(int x, int y, int w, int h, int gotoNum) /* move to admin area button */
{
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y, (x+w), y);
  SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
  SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
  SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  writeText(" . . . ", textColor);
  render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 

  if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp && cycleCounter != cycleCheck)
  {
    cycleCheck = cycleCounter;
    pageNumber = gotoNum;
    memset(&passText[0], 0, 5);
  }
}

void ss_grid()/* start movement procedure for even value grid */
{ 
  dir_move = 0;
  pattern2_move_count = 1;
  printf("SS GRID\n");
  writeVariableValue("O_3", 1);
  writeVariableValue("O_4", 0);
  
  command(9);
  command(11);
  command(10);
  command(12);
  usleep(10000000);
  stop = 0; 
  for(move_count=0;move_count<holes;++move_count)
  {
    draw();	
    drawEbGrid();
    
    sprintf(buffCount, "N:%d", move_count);
    writeText(buffCount, textColor);
    render(750, 400, NULL, 0.0, NULL, SDL_FLIP_NONE);

    sprintf(buffX, "X:%d", curX);
    writeText(buffX, textColor);
    render(750, 450, NULL, 0.0, NULL, SDL_FLIP_NONE);

    sprintf(buffY, "Y:%d", curY);
    writeText(buffY, textColor);
    render(750, 500, NULL, 0.0, NULL, SDL_FLIP_NONE);
    
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
    
    if(readVariableValue("I_3")==1) /* read STOP button value */
    {
      stop = 1;
    }
    if(stop == 1)
    {
      command(20);
      break;
    }

    if(readVariableValue("I_5") == 1 || readVariableValue("I_6") == 1)
    {
      stop = 1;
      writeVariableValue("O_5", 1);     
      command(19);
      break;
    }

    if(readVariableValue("I_5")==1)
    {
      stop = 1; 
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
      break;
    }
    else if(readVariableValue("I_6")==1)
    {
      stop = 1;
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
      break;
    }
    else if(readVariableValue("I_5")==1 && readVariableValue("I_6")==1)
    {
      stop = 1;
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
      break;
    }
    else if(readVariableValue("I_1") == 0)
    {
      stop = 1;
      writeVariableValue("O_5", 1); 
      writeVariableValue("O_3", 0);
      break;
    }
    else if(readVariableValue("I_8") == 1)
    { 
      stop = 1;
      writeVariableValue("O_5", 1); 
      writeVariableValue("O_3", 0);
      break;
    }
    else if(readVariableValue("I_9") == 1)
    {
      stop = 1;
      writeVariableValue("O_5", 1); 
      writeVariableValue("O_3", 0);
      break;
    }
    else
    {
      writeVariableValue("O_5", 0);
    }
    
    //if(move_count==0 && !stop)
    //{
    //  handle_spring();
    //}

    if(move_count < columns-1 && !stop)
    {
      right();
    }

    else if(move_count == columns-1 && !stop)
    {
      up();
    }

    else if(move_count > columns-1 && move_count < ((holes - rows)) && !stop )
    {

      if(dir_move == 0 && pattern2_move_count < columns-1)
      {
        left();
        pattern2_move_count++;
      }   
      else if(dir_move == 1 && pattern2_move_count < columns-1)
      {
        right();
        pattern2_move_count++;
      }
      else if(dir_move == 0 && pattern2_move_count == columns-1 && move_count != (holes - rows))
      {
        up();      
        dir_move = 1;
        pattern2_move_count = 1;
      }
      else if(dir_move == 1 && pattern2_move_count == columns-1 && move_count != (holes - rows))
      {
        up();
        dir_move = 0;
        pattern2_move_count = 1;
      }
    }
    else if(move_count == ((holes - rows)) && !stop)
    {
      left();    
    }
    else if(move_count > (holes - rows) && move_count <= holes && !stop) 
    {
      down();
    }
    SDL_Delay(300);
  }
  
  if(!stop)
  {
    command(22);
    command(11);
    command(23);
    command(12);
    usleep(1000000);
    writeVariableValue("O_4", 1);
  }
  stop = 0;
  writeVariableValue("O_3", 0);
  
}


int page_main()   /* setting up main page */
{
  while(!start && pageNumber == 1)
  {
   
    draw();
    eventUpdate(); 
    initVars(50, 550, 20, 20);
    holes = rows * columns;
    drawEbGrid();
    
    //admin(945, 0, 75, 50, 2);
    
    
    sprintf(buffCount, "N:%d", move_count);
    writeText(buffCount, textColor);
    render(750, 400, NULL, 0.0, NULL, SDL_FLIP_NONE);

    sprintf(buffX, "X:%d", curX);
    writeText(buffX, textColor);
    render(750, 450, NULL, 0.0, NULL, SDL_FLIP_NONE);

    sprintf(buffY, "Y:%d", curY);
    writeText(buffY, textColor);
    render(750, 500, NULL, 0.0, NULL, SDL_FLIP_NONE); 
    if(readVariableValue("I_5")==1)
    { 
      writeText("REF.STIKALO X", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
    }
    else if(readVariableValue("I_6")==1)
    {
      writeText("REF.STIKALO Y", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
    }
    else if(readVariableValue("I_5")==1 && readVariableValue("I_6")==1)
    {
      writeText("REF. STIKALA", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_5", 1);
      writeVariableValue("O_3", 0);
    }
    else if(readVariableValue("I_1") == 0)
    {
      writeText("STOP TOTAL", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_3", 0);
      writeVariableValue("O_5", 1);
    }
       
    else if(readVariableValue("I_8") == 1)
    {
      writeText("SERVO REGUL.", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_3", 0);
    }
    else if(readVariableValue("I_9") == 1)
    {
      writeText("PREKORACITEV HODA", textColor);
      render(750, 100, NULL, 0.0, NULL, SDL_FLIP_NONE);
      writeVariableValue("O_3", 0);
    }
    else
    {
      writeVariableValue("O_5", 0);
    }
    SDL_RenderPresent(renderer);
    cycleCounter++;
    oldtimestamp = timestamp;
    
    if(readVariableValue("I_2") == 1 && readVariableValue("I_1") != 0)      /* read START button value */
    {
      start = 1;
    }

    if(readVariableValue("I_4")==1 && readVariableValue("I_1") != 0)     /* read HOME button value */
    {
      home();
    }
  }
  
  if(pageNumber == 1)                   /* check grid setup */
  {
    holes = rows * columns;
    ss_grid();
    start = 0;
    return 1;
  }
  SDL_RenderClear(renderer);
  start = 0;
  return 1;
}

void page_pass() /* setting up password page */
{
  draw();
  eventUpdate(); 
  keypad(400, 200, 100, 100);
  admin(945, 0, 75, 50, 3);
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
  cycleCounter++;
  oldtimestamp = timestamp;
}

void page_select() /* setting up selection page */
{
  draw();
  eventUpdate();
  admin(945, 0, 75, 50, 1);
  button(400, 100, 300, 100, "SETTINGS", 4); 
  button(400, 300, 300, 100, "MANUAL MODE", 5); 
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
  cycleCounter++;
  oldtimestamp = timestamp;
}


void page_settings() //setting up settings page//
{
  draw();
  eventUpdate();
  admin(945, 0, 75, 50, 3);

  int x;
  int y;
  int w;
  int h;
  
  int x2;
  int y2;
  int w2;
  int h2;

  x=50;
  y=50;
  w=100;
  h=100;
  
  x2=50;
  y2=200;
  w2=100;
  h2=100;
  
  admin(945, 0, 75, 50, 2);
   
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x, y, (x+w), y);
  SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
  SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
  SDL_RenderDrawLine(renderer, x, (y+h), x, y);
  writeText("VENTIL ON", textColor);
  render(x+((w/2)-(textureWidth/2)), y + ((h/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
  
  if(touchLocation.x > x && touchLocation.x < x+w && touchLocation.y > y && touchLocation.y < y + h && timestamp > oldtimestamp)
  {
    writeVariableValue("O_1", 1); 
  }
   
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLine(renderer, x2, y2, (x2+w2), y2);
  SDL_RenderDrawLine(renderer, (x2+w2), y2, (x2+w2), (y2+h2)); 
  SDL_RenderDrawLine(renderer, (x2+w2), (y2+h2), x2, (y2+h2));
  SDL_RenderDrawLine(renderer, x2, (y2+h2), x2, y2);
  writeText("VENTIL OFF", textColor);
  render(x2+((w2/2)-(textureWidth/2)), y2 + ((h2/2)-(textureHeight/2)), NULL, 0.0, NULL, SDL_FLIP_NONE); 
  
  if(touchLocation.x > x2 && touchLocation.x < x2+w2 && touchLocation.y > y2 && touchLocation.y < y2 + h2 && timestamp > oldtimestamp)
  {
    writeVariableValue("O_1", 0); 
  }
  
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
  cycleCounter++;
  oldtimestamp = timestamp;
}

void page_manual() /*setting up manual page*/
{
  if(readVariableValue("I_5") == 0)
  {
    command(13);
  }
  if(readVariableValue("I_6") == 0)
  {
    command(14);
  }
  
  draw();
  eventUpdate();
  admin(945, 0, 75, 50, 6);
  up_button(500, 100);
  down_button(500, 250);
  left_button(300, 250);
  right_button(700, 250);
  
  if(readVariableValue("I_5") == 1)
  {
    command(15);
    writeVariableValue("O_3", 1);
  }
  
  if(readVariableValue("I_6") == 1)
  {
    command(16);
    writeVariableValue("O_3", 1);
  }

  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
  cycleCounter++;
  oldtimestamp = timestamp;
}

void page_redirect()
{
  curX=0;
  curY=0;
  pageNumber=1; 
}

void load_page(int pageNumber) /*handling loading pages */
{
  switch(pageNumber)
  {
    case 1:
      page_main();
      break;

    case 2:
      page_pass();
      break;
  
    case 3:
      page_select();
      break;

    case 4:
      page_settings();
      break;
 
    case 5:
      page_manual();
      break;
    
    case 6:
      page_redirect();
      break;
  }
}

int main()
{
  FILE *fp;  
  char *line;
  size_t len;
  int i;

  i = 0;
  line = NULL;
  len = 0;
  /* TO BE CHANGED */
  /* fp = fopen("/home/pi/move_program/param.txt", "r");
  
  for(i = 0; i < 2; ++i)
  {
    getline(&line, &len, fp);
    printf("%s", line);
    if(i == 0)
    {
      timer1 = atoi(line); 
    }
    else
    {
      timer2 = atoi(line);   
    }
  }
  */
  pageNumber = 1;
  passText[0] = '\0';
  init();
  comm_init();
  program = 1;
  command(19);
  command(15);
  command(16);
  //command(21); 
  home();
 
  while(program == 1)
  {
    load_page(pageNumber);
  }
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_DestroyTexture(texture);
  TTF_Quit();
  SDL_Quit(); 
  return 1;
}
