/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfsans.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bfsans_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	bfsans_ArcMain_bf0,
	bfsans_ArcMain_bf1,
	bfsans_ArcMain_bf2,
	bfsans_ArcMain_bf3,
	bfsans_ArcMain_bf4,
	bfsans_ArcMain_bf5,
	bfsans_ArcMain_bf6,
	bfsans_ArcMain_Dead0, //BREAK
	bfsans_ArcMain_bfb0,
	bfsans_ArcMain_bfb1,
	bfsans_ArcMain_bfb2,
	bfsans_ArcMain_bfb3,

	bfsans_ArcMain_Max,
};

enum
{
	bfsans_ArcDead_Dead1, //Mic Drop
	bfsans_ArcDead_Dead2, //Twitch
	bfsans_ArcDead_Retry, //Retry prompt
	
	bfsans_ArcDead_Max,
};

#define bfsans_Arc_Max bfsans_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[bfsans_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfsans_skull)];
	u8 skull_scale;
} Char_bfsans;

//Boyfriend player definitions
static const CharFrame char_bfsans_frame[] = {
	{bfsans_ArcMain_bf0, {  0,   0, 106,  95}, { 99,  95}}, //0 idle 1
	{bfsans_ArcMain_bf0, {106,   0, 102,  99}, { 94,  99}}, //1 idle 2
	{bfsans_ArcMain_bf0, {  0, 104, 102,  101}, { 93,  101}}, //2 idle 3
	{bfsans_ArcMain_bf0, {106, 104, 102,  101}, { 93,  101}}, //3 idle 4
	{bfsans_ArcMain_bf1, {  0,   1, 101,  102}, { 93,  102}}, //4 idle 5
	
	{bfsans_ArcMain_bf1, {108,   2, 94,  101}, { 93,  101}}, //5 left 1
	{bfsans_ArcMain_bf1, {  1, 107, 93,  101}, { 92,  101}}, //6 left 2
	
	{bfsans_ArcMain_bf1, { 97, 104, 98,  87}, { 98,  87}}, //7 down 1
	{bfsans_ArcMain_bf2, {  0,   0, 94,  91}, { 93,  91}}, //8 down 2
	
	{bfsans_ArcMain_bf2, { 95,   1, 93,  110}, { 82,  110}}, //9 up 1
	{bfsans_ArcMain_bf2, {  0,  94, 94,  108}, { 82,  108}}, //10 up 2
	
	{bfsans_ArcMain_bf2, { 95, 113, 102,  102}, { 83,  102}}, //11 right 1
	{bfsans_ArcMain_bf3, {  1,   0, 100,  103}, { 85,  103}}, //12 right 2
	
	{bfsans_ArcMain_bf3, {101,  17, 111,  89}, { 105,  89}}, //13 peace 1
	{bfsans_ArcMain_bf3, {  0, 107,  97,  104}, { 94,  104}}, //14 peace 2
	{bfsans_ArcMain_bf3, {106, 108, 102,  101}, { 99,  101}}, //15 peace 3
	
	{bfsans_ArcMain_bf4, {  0,   0, 128, 128}, { 53,  92}}, //16 sweat 1
	{bfsans_ArcMain_bf4, {128,   0, 128, 128}, { 53,  93}}, //17 sweat 2
	{bfsans_ArcMain_bf4, {  0, 128, 128, 128}, { 53,  98}}, //18 sweat 3
	{bfsans_ArcMain_bf4, {128, 128, 128, 128}, { 53,  98}}, //19 sweat 4
	
	{bfsans_ArcMain_bf5, {  0,   0,  113, 107}, { 101, 107}}, //20 left miss 1
	{bfsans_ArcMain_bf5, {113,   0,   93, 108}, { 90, 108}}, //21 left miss 2
	
	{bfsans_ArcMain_bf5, {  0, 110,  94, 105}, { 94, 95}}, //22 down miss 1
	{bfsans_ArcMain_bf5, {100, 118,  94, 97}, { 94, 95}}, //23 down miss 2
	
	{bfsans_ArcMain_bf6, {  0,   0, 88, 127}, { 85, 127}}, //24 up miss 1
	{bfsans_ArcMain_bf6, { 92,   9, 89, 111}, { 85, 111}}, //25 up miss 2
	
	{bfsans_ArcMain_bf6, {  0, 127,125, 107}, { 93, 107}}, //26 right miss 1
	{bfsans_ArcMain_bf6, {125, 127, 99, 107}, { 85, 107}}, //27 right miss 2

	{bfsans_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //23 dead0 0
	{bfsans_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //24 dead0 1
	{bfsans_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //25 dead0 2
	{bfsans_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //26 dead0 3
	
	{bfsans_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //27 dead1 0
	{bfsans_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //28 dead1 1
	{bfsans_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //29 dead1 2
	{bfsans_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //30 dead1 3
	
	{bfsans_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //31 dead2 body twitch 0
	{bfsans_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //32 dead2 body twitch 1
	{bfsans_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //33 dead2 balls twitch 0
	{bfsans_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //34 dead2 balls twitch 1

	{bfsans_ArcMain_bfb0, {  0,   0, 105,  97}, { 84 + 90,  97 - 120}}, //40 idle 1
	{bfsans_ArcMain_bfb0, {108,   0, 100,  103}, { 81 + 90,  103 - 120}}, //41 idle 2
	{bfsans_ArcMain_bfb0, {  0, 103, 101,  102}, { 81 + 90,  102 - 120}}, //42 idle 3
	{bfsans_ArcMain_bfb0, {106, 105, 102,  100}, { 82 + 90,  100 - 120}}, //43 idle 4
	{bfsans_ArcMain_bfb1, {  0,   0, 102,  100}, { 82 + 90,  100 - 120}}, //44 idle 5
	
	{bfsans_ArcMain_bfb1, {112,   0, 100,  99}, { 76 + 90,  99 - 120}}, //45 left 1
	{bfsans_ArcMain_bfb1, {  0, 113, 103,  99}, { 75 + 90,  99 - 120}}, //46 left 2
	
	{bfsans_ArcMain_bfb1, {110, 117, 102,  95}, { 81 + 90,  95 - 120}}, //47 down 1
	{bfsans_ArcMain_bfb2, {  0,   0,  85,  113}, { 68 + 90,  113 - 120}}, //48 down 2
	
	{bfsans_ArcMain_bfb2, {107,   0, 93,  105}, { 89 + 90,  105 - 120}}, //49 up 1
	{bfsans_ArcMain_bfb2, {  0, 113, 85,  116}, { 82 + 90,  116 - 120}}, //50 up 2
	
	{bfsans_ArcMain_bfb2, {103, 105, 97,  102}, { 93 + 90,  102 - 120}}, //51 right 1
	{bfsans_ArcMain_bfb3, {  0,   0, 100,  102}, { 100 + 90,  102 - 120}}, //52 right 2
};

static const Animation char_bfsans_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_BACK, 1}}, //CharAnim_Idle
	{3, (const u8[]){ 44,  43,  42,  41,  40, ASCR_BACK, 1}}, //CharAnim_IdleAlt
	{2, (const u8[]){ 5,  6, ASCR_BACK, 1}},             //CharAnim_Left
	{2, (const u8[]){ 52, 52, ASCR_CHGANI,ASCR_BACK, 1}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{2, (const u8[]){ 48, 47, ASCR_CHGANI, ASCR_BACK, 1}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{2, (const u8[]){ 50, 49, ASCR_CHGANI, ASCR_BACK, 1}},       //CharAnim_UpAlt
	{2, (const u8[]){ 11, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{2, (const u8[]){ 46, 45, ASCR_CHGANI, ASCR_BACK, 1}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 5, 20, 20, 21, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 7, 22, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 9, 24, 24, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){11, 26, 26, 27, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{3, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{5, (const u8[]){28, 29, 30, 31, 31, 31, 31, 31, 31, 31, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){32, 33, 34, 35, 35, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){35, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){36, 37, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){38, 39, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){35, 35, 35, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){38, 39, 35, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_bfsans_SetFrame(void *user, u8 frame)
{
	Char_bfsans *this = (Char_bfsans*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfsans_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_bfsans_Tick(Character *character)
{
	Char_bfsans *this = (Char_bfsans*)character;
		
	if (stage.stage_id == StageId_1_2 && stage.song_step == 800)
		this->character.spec = 0;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, (stage.song_step >= 800) ? CharAnim_IdleAlt : CharAnim_Idle);
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_bfsans, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_bfsans_SetFrame);
	Character_Draw(character, &this->tex, &char_bfsans_frame[this->frame]);
}

void Char_bfsans_SetAnim(Character *character, u8 anim)
{
	Char_bfsans *this = (Char_bfsans*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //bfsans_ArcDead_Dead1
				"dead2.tim", //bfsans_ArcDead_Dead2
				"retry.tim", //bfsans_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[bfsans_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_bfsans_Free(Character *character)
{
	Char_bfsans *this = (Char_bfsans*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_bfsans_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_bfsans *this = Mem_Alloc(sizeof(Char_bfsans));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_bfsans_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_bfsans_Tick;
	this->character.set_anim = Char_bfsans_SetAnim;
	this->character.free = Char_bfsans_Free;
	
	Animatable_Init(&this->character.animatable, char_bfsans_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-125,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFSANS.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFSDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",   //bfsans_ArcMain_bfsans0
		"bf1.tim",   //bfsans_ArcMain_bfsans1
		"bf2.tim",   //bfsans_ArcMain_bfsans2
		"bf3.tim",   //bfsans_ArcMain_bfsans3
		"bf4.tim",   //bfsans_ArcMain_bfsans4
		"bf5.tim",   //bfsans_ArcMain_bfsans5
		"bf6.tim",   //bfsans_ArcMain_bfsans6
		"dead0.tim", //bfsans_ArcMain_Dead0
		"bfb0.tim",   //bfsans_ArcMain_bfsans0
		"bfb1.tim",   //bfsans_ArcMain_bfsans1
		"bfb2.tim",   //bfsans_ArcMain_bfsans2
		"bfb3.tim",   //bfsans_ArcMain_bfsans3
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bfsans_skull, sizeof(char_bfsans_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_bfsans, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
