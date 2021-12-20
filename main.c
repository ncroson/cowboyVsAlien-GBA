
#include <stdio.h>
#include <string.h>
#include <tonc.h>
#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"
#include "cowboy.h"
#include "alien.h"
#include "desert.h"
#include "bullet.h"


//function declarations
void init();
void detect_hit(int);
void update_screen();
void jump();
void shootBullet();
void killAlien();
void move_cowboy();
void move_alien();


OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

OBJ_ATTR *cowboy = &obj_buffer[0];
struct PlayerS {
	/*
	u32 x= 96;
	u32 y= 80;
	u32 tid= 0 ;
	u32 pb= 0;
	int jumpSpeed = -12;
	int gravity = 1;
	bool isJumping = false;
	bool facingRight = true;
	*/
	u32 x;
	u32 y;
	u32 tid;
	u32 pb;
	int jumpSpeed;
	int gravity;
	bool isJumping;
	bool facingRight;
};

struct PlayerS player;


OBJ_ATTR *alien = &obj_buffer[1];
struct EnemyS{
	/*
	u32 ax = 96;
	u32 ay = 80;
	u32 atid = 80;
	u32 apb = 0;
	u32 adx = 1;
	bool alienExplode = false;
	bool alienShot = false;
	*/
	u32 ax;
	u32 ay;
	u32 atid;
	u32 apb;
	u32 adx;
	bool alienExplode;
	bool alienShot;
};

struct EnemyS enemy;


OBJ_ATTR *bullet[11];
struct BulletSprS {
	/*
	int bx;
	int by;
	u32 btid = 176;
	bpb = 0;
	bool shotFired;
	
	u32 bulletSpeed = 4;
	*/
	int bx;
	int by;
	u32 btid;
	u32 bpb;
	u32 dir;
	bool shotFired;
	
	u32 bulletSpeed;
};

struct BulletSprS bulletSpr[10];


u32 frame= 0; 
u32 shotCount = 0;


mm_sound_effect shoot = {
		{ SFX_GUNSHOT } ,
		(int) (1.0f * (1<<10)), //rate
		0, //handle
		128, //volume
		0,  //panning
	};
	
	mm_sound_effect alienDeath = {
		{ SFX_ALIENDIE } ,
		(int) (1.0f * (1<<10)), //rate
		1, //handle
		255, //volume
		0, //panning
	};

mm_sound_effect playerJump = {
		{ SFX_JUMP } ,
		(int) (1.0f * (1<<10)), //rate
		0, //handle
		255, //volume
		0,  //panning
	};
	




int main()
{
	int selectedsong = 0;
	
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, mmVBlank);
	mmInitDefault( (mm_addr)soundbank_bin, 8 );
	
	mmStart( selectedsong, MM_PLAY_LOOP );

	REG_BG0HOFS = 16;
	REG_BG0VOFS = 55;
	
	init();
	
	while(1)
	{
		//vid_vsync();
		VBlankIntrWait();
		
		move_cowboy();
		move_alien();
		shootBullet();
		
		update_screen();
		
		mmFrame();
		frame++;
		if(frame >= 255){
			frame = 0;
		}
	}
	
	return 0;
}


void init()
{
	player.x= 30;
	player.y= 80;
	player.tid= 0;
	player.pb= 0;
	player.jumpSpeed = -12;
	player.gravity = 1;
	player.isJumping = false;
	player.facingRight = true;
	
	enemy.ax = 200;
	enemy.ay = 80;
	enemy.atid = 80;
	enemy.apb = 0;
	enemy.adx = 0;
	enemy.alienExplode = false;
	enemy.alienShot = false;
	
	for(int i = 0; i < 10; i++){
		bulletSpr[i].bx = player.x;
		bulletSpr[i].by = player.y + 13;
		bulletSpr[i].btid = 176;
		bulletSpr[i].bpb = 0;
		bulletSpr[i].shotFired = false;
		bulletSpr[i].bulletSpeed = 4;
		bulletSpr[i].dir = 1;
	}
	
	
	//load palette
	memcpy(pal_bg_mem, desertPal, desertPalLen);
	//load tiles into CBB 0
	memcpy(&tile_mem[0][0], desertTiles, desertTilesLen);
	//load map into SBB 30
	memcpy(&se_mem[30][0], desertMap, desertMapLen);
	
	
	//place the tiles of 4bpp mario sprite
	//into LOW obj memory (cbb == 4)
	memcpy(&tile_mem[4][0], cowboySmokeTiles, cowboySmokeTilesLen);
    memcpy(pal_obj_mem, cowboySmokePal, cowboySmokePalLen);
	memcpy(&tile_mem[4][16], cowboyStandTiles, cowboyStandTilesLen);
    memcpy(pal_obj_mem, cowboyStandPal, cowboyStandPalLen);
	memcpy(&tile_mem[4][32], cowboyWalkTiles, cowboyWalkTilesLen);
    memcpy(pal_obj_mem, cowboyWalkPal, cowboyWalkPalLen);
	memcpy(&tile_mem[4][48], cowboyShootTiles, cowboyShootTilesLen);
    memcpy(pal_obj_mem, cowboyShootPal, cowboyShootPalLen);
	memcpy(&tile_mem[4][64], cowboyJumpTiles, cowboyJumpTilesLen);
    memcpy(pal_obj_mem, cowboyJumpPal, cowboyJumpPalLen);
	memcpy(&tile_mem[4][80], alienWalk1Tiles, alienWalk1TilesLen);
    memcpy(pal_obj_mem, alienWalk1Pal, alienWalk1PalLen);
	memcpy(&tile_mem[4][96], alienWalk2Tiles, alienWalk2TilesLen);
    memcpy(pal_obj_mem, alienWalk2Pal, alienWalk2PalLen);
	memcpy(&tile_mem[4][112], alienStandTiles, alienStandTilesLen);
    memcpy(pal_obj_mem, alienStandPal, alienStandPalLen);
	memcpy(&tile_mem[4][128], alienShot1Tiles, alienShot1TilesLen);
    memcpy(pal_obj_mem, alienShot1Pal, alienShot1PalLen);
	memcpy(&tile_mem[4][144], alienShot2Tiles, alienShot2TilesLen);
    memcpy(pal_obj_mem, alienShot2Pal, alienShot2PalLen);
	memcpy(&tile_mem[4][160], alienShot3Tiles, alienShot3TilesLen);
    memcpy(pal_obj_mem, alienShot3Pal, alienShot3PalLen);
	memcpy(&tile_mem[4][176], bulletTiles, bulletTilesLen);
    memcpy(pal_obj_mem, bulletPal, bulletPalLen);
	
	for(int i = 0; i < 10; i++){
		bullet[i] = &obj_buffer[i+2];
	}
	
	

	//initialize all sprites
	oam_init(obj_buffer, 128);
	
	REG_BG0CNT= BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_REG_32x32;
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
	
	obj_set_attr(cowboy,
		ATTR0_8BPP | ATTR0_TALL,
		
		ATTR1_SIZE_32,
		ATTR2_PALBANK(player.pb) | player.tid);
	
	obj_set_attr(alien,
		ATTR0_8BPP | ATTR0_TALL,
		
		ATTR1_SIZE_32,
		ATTR2_PALBANK(enemy.apb) | enemy.atid);
	
	//initalize bullets
	for(int i = 0; i < 10; i++){
		obj_set_attr(bullet[i],
			ATTR0_8BPP | ATTR0_SQUARE,
			ATTR1_SIZE_8,
			ATTR2_PALBANK(bulletSpr[i].bpb) | bulletSpr[i].btid);
	}
	
	for(int i = 0; i < 10; i++)
	{	
		obj_hide(bullet[i]);
		
		bulletSpr[i].bx = 25;
		bulletSpr[i].bx = 92;
		
		bulletSpr[i].shotFired = false;
	
		enemy.alienShot = false;
	}
	
	
}

void update_screen()
{
	
	
	cowboy->attr2= ATTR2_BUILD(player.tid, player.pb, 0);
	//oam_copy(oam_mem, obj_buffer, 1);
	alien->attr2= ATTR2_BUILD(enemy.atid, enemy.apb, 0);
	
	for(int i = 0; i < 10; i++){
		bullet[i]->attr2= ATTR2_BUILD(bulletSpr[i].btid, bulletSpr[i].bpb, 0);
	}
	
	oam_copy(oam_mem, obj_buffer, 13);
}

void jump()
{
	
	player.y += player.jumpSpeed;
	player.jumpSpeed += player.gravity;
	player.tid = 64;
		
	if(player.jumpSpeed > 12){
		player.jumpSpeed = -12;
		player.isJumping = false;
		player.tid = 16;
	}
		
	
}



void shootBullet()
{
	
	
	for(int i = 0; i < 10; i++)
	{
		
		if(bulletSpr[i].shotFired == true)
		{
			obj_unhide(bullet[i], 0);
			//bulletSpr[i].by = player.y;
			if(bulletSpr[i].dir == 1)
				bulletSpr[i].bx += bulletSpr[i].bulletSpeed;
			else if(bulletSpr[i].dir == -1)
				bulletSpr[i].bx -= bulletSpr[i].bulletSpeed;
			
			
				
			if(bulletSpr[i].bx < 0)
			{	
				bulletSpr[i].shotFired = false;
				obj_hide(bullet[i]);
				bulletSpr[i].bx = 250;
				
			}
			if(bulletSpr[i].bx > 232)
			{
				bulletSpr[i].shotFired = false;
				obj_hide(bullet[i]);
				bulletSpr[i].bx = 250;
			}
			if(bulletSpr[i].by < 0)
			{
				bulletSpr[i].shotFired = false;
				obj_hide(bullet[i]);
				bulletSpr[i].by = 180;
			}
			if(bulletSpr[i].by > 152)
			{
				bulletSpr[i].shotFired = false;
				obj_hide(bullet[i]);
				bulletSpr[i].by = 180;
			}
			
			detect_hit(i);
			obj_set_pos(bullet[i], bulletSpr[i].bx, bulletSpr[i].by);
		}
	}
}	


void detect_hit(int shotNum)
{
	if(bulletSpr[shotNum].bx >= enemy.ax && bulletSpr[shotNum].bx <= (enemy.ax + 16) && enemy.atid <= 128){
		mmEffectEx(&alienDeath);
		enemy.alienExplode = true;
		enemy.atid = 112;
		enemy.adx = 0;
		
	}
}

void killAlien(){
	
	
	if(enemy.alienExplode == true){
		if(!(frame & 7)){
			enemy.atid+= 16;
		}
		if(enemy.atid >= 160){
			enemy.atid = 160;
		}
	}
}
void move_cowboy(){
	key_poll();
	
	
	if(key_hit(KEY_RIGHT) && player.facingRight == false){
		cowboy->attr1 ^= ATTR1_HFLIP;
		player.facingRight = true;
	}
	
	if(key_hit(KEY_LEFT) && player.facingRight == true){
		cowboy->attr1 ^= ATTR1_HFLIP;
		player.facingRight = false;
	}

	if(key_is_down(KEY_RIGHT)){
		player.x++;
		if(!(frame & 7)){
			player.tid += 16;
			if(player.tid > 32)
				player.tid = 16;
		}
	}
	
	else if(key_is_down(KEY_LEFT)){
		player.x--;
		if(!(frame & 7)){
			player.tid+= 16;
			if(player.tid > 32)
				player.tid = 16;
		}
	}
	
	else{
		player.tid = 16;
		
		if(!(frame & 32)){
			player.tid+= 16;
			if(player.tid > 16)
				player.tid = 0;
		}
	}
	
	if(key_hit(KEY_A)){
		player.isJumping = true;
		mmEffectEx(&playerJump);
	}
	if(player.isJumping == true){
		jump();
	}
	if(key_hit(KEY_B)){
		player.tid = 48;
		
		if(enemy.alienExplode == false  || enemy.atid == 160){
			mmEffectEx(&shoot);
		}
		
		obj_hide(bullet[0]);
		obj_hide(bullet[1]);
		
		
		
		if(player.facingRight == true)
			bulletSpr[shotCount].dir = 1;
		else if(player.facingRight == false)
			bulletSpr[shotCount].dir = -1;
		
		bulletSpr[shotCount].by = player.y + 13;
		bulletSpr[shotCount].bx = player.x;
		bulletSpr[shotCount].shotFired = true;
		
		shotCount++;
		if(shotCount >= 10)
			shotCount = 0;
			
	}
	
	if(key_is_down(KEY_B))
		player.tid = 48;
	
    obj_set_pos(cowboy, player.x, player.y);
}


void move_alien(){
	
	
	
	enemy.ax = enemy.ax + enemy.adx;
	if(enemy.alienExplode == false){		
		if(enemy.ax >= 224){
			enemy.ax = 224;
			enemy.adx = -enemy.adx;
		}
		else if(enemy.ax <= 0){
			enemy.ax = 0;
			enemy.adx = -enemy.adx;		
		}
		if(!(frame & 7)){
			enemy.atid+= 16;
			if(enemy.atid > 96)
				enemy.atid = 80;
		}
	}
	
	if(key_hit(KEY_START)){
		enemy.adx = -1;
	}
	
	if(key_hit(KEY_R)){
		enemy.adx = 0;
		enemy.alienExplode = true;
		enemy.atid = 112;
		mmEffectEx(&alienDeath);
	}
	if(enemy.alienExplode == true){
		if(!(frame & 7)){
			enemy.atid+= 16;
		}
		if(enemy.atid >= 160){
			enemy.atid = 160;
		}
		
		
	}
	
	if(key_hit(KEY_L)){
		enemy.adx = 1;
		enemy.alienExplode = false;
	}
	
	
    obj_set_pos(alien, enemy.ax, enemy.ay);
}





