//#include <stdio.h>
#include <gb/gb.h>
#include <gb/drawing.h>

//Sprite/Tile data
#include "tileset.c"
#include "l11.c"
#include "qblock.c"
#include "mario.c"
#include "font.c"

//Constants
#define GRAVITY 2
#define MAX_VEL 2
#define ZERO 128
#define SCROLL_POS 70
#define EVENT_DELAY 20
#define WIDTH 144
#define HEIGHT 160

//Level scrolling data
BOOLEAN scroll = 0;
BOOLEAN end = 0;
BOOLEAN grounded = 1;
UBYTE scrollX = 0;
BYTE tileCounter = 0;
UBYTE tmpA = 0;
UBYTE tmpB = 0;
UWORD count = 0;
UBYTE i = 0;

//Timers
UBYTE time;
UBYTE animationTimer = time;
UBYTE eventTimer = time;

//QBlock animation frame counter
UBYTE qBlockFrame = 0;

//Velocity and direction
UINT32 pX = 25;
UINT32 pY = ZERO;
BYTE velX = 0;
BYTE velY = 0;
BOOLEAN dir = 0;

//Function prototypes
void init();
void drawMario();
void drawBlock();
void drawLevel();
void collision();
void drawHUD();
void eventHandler();

int main(){
	//Initialize sprite and level data
	init();
	//Game loop
	while(1){
		//Wait for previous drawing cycle to finish
		wait_vbl_done();
		//Increment timer variable
		time++;
		//Event handler
		eventHandler();
		//Draw level
		drawLevel();
		//Draw Mario sprite
		drawMario();
		//Draw HUD
		drawHUD();
		//Show background layer
		SHOW_BKG;
	}
}

void init(){
	wait_vbl_done();
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
	HIDE_BKG;
	SWITCH_ROM_MBC1(2);

	//Initialize level tile data
	set_bkg_data(0,66,&tileset);
	count = 0;
	for(tmpA=0; tmpA!=18; ++tmpA){
		set_bkg_tiles(0, tmpA, 22, 1, (l11+count));
		count+=l11Width;
	}

/*	SPRITES_8x8;
	set_sprite_data(0,8,qblock);
	set_sprite_tile(0,0);
	move_sprite(0,75,75);
*/
	//Initialize Mario sprite data
	SPRITES_8x16;
	set_sprite_data(0,40,mario);
	set_sprite_tile(0,0);
	set_sprite_tile(1,2);
	move_sprite(0,pX,ZERO);
	move_sprite(1,pX+8,ZERO);

	//Initialize HUD
	//set_win_data(0,40,font);
	//set_win_tiles(0,0,8,8,font);
	//move_win(0,0);

	SHOW_BKG;
	SHOW_SPRITES;
	//SHOW_WIN;
	DISPLAY_ON;
	enable_interrupts();
}

void drawBlock(){
	//Question block animation
	if((time-animationTimer)>32){
		qBlockFrame = (qBlockFrame+1)%3;
		set_sprite_tile(0, qBlockFrame);
		animationTimer=time;
	}
}

void drawMario(){
	//Set correct sprite
	if(!grounded){ //Jumping
		set_sprite_tile(0, 32+dir*4);
		set_sprite_tile(1, 34+dir*4);
	} else if(velX!=0){ //Running
		set_sprite_tile(0, 8+((pX|scrollX)%3)*4+dir*4);
		set_sprite_tile(1, 10+((pX|scrollX)%3)*4+dir*4);
	} else { //Static
		set_sprite_tile(0, dir*4);
		set_sprite_tile(1, 2+dir*4);
	}

	//Move sprite if map is not scrolling
	if(!scroll){
		move_sprite(0,pX,pY);
		move_sprite(1,pX+8,pY);
	}
}

void drawLevel(){
	//Check if level end has been reached
	if(!end) if(scrollX>=l11Width-20) end = 1;

	//Scroll background
	if(scroll&&!end&&velX>0){
		scroll_bkg(velX,0);
		tileCounter+=velX;
	}

	//Draw only visible tiles
	if(tileCounter>=8){
		scrollX++;
		tileCounter=0;
		count=scrollX+21;
		tmpB=count%32;
		for(tmpA=0; tmpA!=18; ++tmpA){
			set_bkg_tiles(tmpB, tmpA, 1, 1, l11+count);
			count+=l11Width;
		}
	}
}

void collision(){
	//Check for ground collision
	if(!grounded && pY+velY>=ZERO){
	   	velY=0;
		grounded=1;
	}
}

void drawHUD(){
	
}

void eventHandler(){
	//Check if enough frame time has passed
	if(time-eventTimer>EVENT_DELAY){
		//Get key states
		i = joypad();

		//Accelerate/decelerate
		if(i&J_RIGHT){ velX+=1+(i&J_B)*2; dir=0; }
		else if(velX>0) velX--;
		if(i&J_LEFT){ velX-=1+(i&J_B)*2; dir=1; }
		else if(velX<0) velX++;
		
		//Jumping conditions
		if(grounded && i&J_A){ velY-=MAX_VEL; grounded=0; }

		//Apply gravity
		if(!grounded) velY--;
		
		//Limit velocity
		if(velX>MAX_VEL) velX = MAX_VEL;
		else if(velX<-MAX_VEL) velX = -MAX_VEL;
		if(velY>MAX_VEL) velY = MAX_VEL;
		else if(velY<-MAX_VEL) velY = -MAX_VEL;

		//Limit position
		//if(pX<0) pX=0;
		//if(pX>WIDTH-16) pX=WIDTH-16;

		//Check for colisions
		collision();
		
		//Increment positon
		pX+=velX;
		pY+=velY;

		//Raise level scroll flag if needed
		//if(!end&&velX>0&&pX>=SCROLL_POS){
		//   	pX = SCROLL_POS;
			scroll = 1;
		//} else scroll = 0;
		
		//Reset timer
		eventTimer = time;
	}
}
