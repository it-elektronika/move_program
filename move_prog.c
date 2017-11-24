#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080



SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
SDL_Texture *texture = NULL;
SDL_Color textColor = {255, 255, 255, 255};

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

int rows = 13;
int columns = 13;
struct editbox eb[169];

int editBox_id_counter;

int curX = 0;
int curY = 0;

int textureWidth = 0;
int textureHeight = 0;

int i = 0;
int move_count = 1;
int pattern_move_count = 1;
int pattern2_move_count = 1;

int dir_move;

int holes;

int init()
{  
  if((SDL_Init(SDL_INIT_VIDEO||SDL_INIT_AUDIO||SDL_INIT_TIMER)) != 0)
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
    printf("RENDERER IS NULL");
  }
  
  return 15;
}

void freeTexture(void)
{
  if(texture != NULL)
  {
    SDL_DestroyTexture(texture);	  
    texture = NULL;
    textureWidth = 0;
    textureHeight = 0;
  }
}


void draw(void)
{
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
}

void close()
{
  freeTexture();	
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}

void render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
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

void drawTextBox(int i, int x, int y, int w, int h)
{
  if(eb[i].hasFocus == 0)
  {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, x, y, (x+w), y);
    SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
    SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
    SDL_RenderDrawLine(renderer, x, (y+h), x, y);
    render(eb[i].x, eb[i].y, NULL, 0.0, NULL, SDL_FLIP_NONE);
  }
  if(eb[i].posX == curX && eb[i].posY == curY)
  {
    SDL_Rect fillRect = {x, y, w, h};	  
    SDL_SetRenderDrawColor(renderer, 0x110, 0x70, 0x200, 0x70);
    SDL_RenderFillRect(renderer, &fillRect);
    SDL_RenderDrawLine(renderer, x, y, (x+w), y);
    SDL_RenderDrawLine(renderer, (x+w), y, (x+w), (y+h)); 
    SDL_RenderDrawLine(renderer, (x+w), (y+h), x, (y+h));
    SDL_RenderDrawLine(renderer, x, (y+h), x, y);
    render(eb[i].x, eb[i].y, NULL, 0.0, NULL, SDL_FLIP_NONE);
  }  
}




void drawEbGrid(void)
{
  int i;
  
  
  for(i = 1; i < 170; ++i)
  {
    drawTextBox(i, eb[i].x, eb[i].y, eb[i].w, eb[i].h);
  }
}

void initVars()
{
  int x = 400;
  int y = 700;
  int w = 50;
  int h = 50;
  editBox_id_counter = 1;
  int posX = 0;
  int posy = 0;

  int drawRows = 0;
  int drawColumns = 0;

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
      strcpy(eb[editBox_id_counter].value, "0\0");
      /*printf("ebval[%d]: %s\n", editBox_id_counter, ebVal[editBox_id_counter]); */
      
      editBox_id_counter++;
      x = x + w;
    }
    x = 400;
    y = y - h;  
  }
  

}
//////////////////////////////////////////////////////////////////////////////////////////////
void up()
{
  curY++;	
  printf("%d:UP\n", move_count);
}
void down()
{
  curY--;	
  printf("%d:DOWN\n", move_count);
}
void left()
{
  curX--;	
  printf("%d:LEFT\n", move_count);
}
void right()
{
  curX++;
  printf("%d:RIGHT\n", move_count);
}
void diagonal()
{
  curX++;
  curY++;
  printf("%d:DIAGONAL\n", move_count);
}

int main()
{
  init();
  draw();
 
  

  initVars();
  


  holes = rows * columns;
  
  dir_move = 1;
  for(i=0;i<holes;++i)
  {
    drawEbGrid();
    SDL_RenderPresent(renderer);	  
    if(move_count == 1)
    {
      diagonal();
    }
    else if(move_count < ((columns*2)-2) && move_count != 1)
    {
      switch(pattern_move_count)
      {
        case 1:
	  down();
	  break;
	case 2:
	  right();
	  break;
	case 3:
          up();
	  break;
        case 4:
	  right();
	  break;
      }
      if(pattern_move_count < 4)
      {
        pattern_move_count++;
      }
      else
      {
        pattern_move_count = 1;
      }
    }
    else if(move_count >= ((columns*2)-2) && move_count < (columns*2))
    {
      up();
    }
    else if(move_count >= columns*2 && move_count < ((holes - columns)+1))
    {
      if(dir_move == 1 && pattern2_move_count < (rows-1))
      {
        left();
	pattern2_move_count++;
      }   
      else if(dir_move == 0 && pattern2_move_count < (rows-1))
      {
        right();
	pattern2_move_count++;
      }
      else if(dir_move == 0 && pattern2_move_count == (rows-1))
      {
	up();      
        dir_move = 1;
	pattern2_move_count = 1;
      }
      else if(dir_move == 1 && pattern2_move_count == (rows-1))
      {
	up();
        dir_move = 0;
	pattern2_move_count = 1;
      }
    }
    else if(move_count == ((holes - columns)+1))
    {
      left();    
    }
    else if(move_count > (holes - columns) && move_count <= holes)
    {
      down();
    }
    move_count++;
    SDL_Delay(100);
  }
}
