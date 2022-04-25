/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "cuphead.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//cuphead character structure
enum
{
	cuphead_ArcMain_Idle,
	cuphead_ArcMain_Left,
	cuphead_ArcMain_Down,
	cuphead_ArcMain_Up,
	cuphead_ArcMain_Right,
	
	cuphead_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[cuphead_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_cuphead;

//cuphead character definitions
static const CharFrame char_cuphead_frame[] = {
	{cuphead_ArcMain_Idle, {  0,   0, 90, 103}, { 83, 102}}, //0 idle 1
	{cuphead_ArcMain_Idle, { 90,   0, 90, 104}, { 83, 103}}, //1 idle 2
	{cuphead_ArcMain_Idle, {  0, 103, 89, 105}, { 82, 104}}, //2 idle 3
	{cuphead_ArcMain_Idle, { 89, 104, 89, 107}, { 83, 105}}, //3 idle 4
	
	{cuphead_ArcMain_Left, {  0,   0, 124, 106}, { 124, 106}}, //4 left 1
	{cuphead_ArcMain_Left, {124,   0, 124, 108}, { 121, 108}}, //5 left 2
	{cuphead_ArcMain_Left, {  0, 106, 124, 108}, { 121, 108}}, //6 left 1
	{cuphead_ArcMain_Left, {124, 108, 124, 108}, { 120, 108}}, //7 left 2
	
	{cuphead_ArcMain_Down, {  0,   0, 109, 102}, { 86, 95}}, //8 down 1
	{cuphead_ArcMain_Down, {109,   0, 101, 103}, { 76, 103}}, //9 down 2
	{cuphead_ArcMain_Down, {  0, 102, 101, 105}, { 78, 105}}, //10 down 1
	{cuphead_ArcMain_Down, {101, 103, 102, 105}, { 78, 105}}, //11 down 2
	
	{cuphead_ArcMain_Up, {  0,   0, 104, 130}, { 84, 130}}, //12 up 1
	{cuphead_ArcMain_Up, {104,   0, 110, 113}, { 91, 113}}, //13 up 2
	{cuphead_ArcMain_Up, {  0, 130, 113, 109}, { 92, 109}}, //14 up 1
	{cuphead_ArcMain_Up, {113, 113, 116, 106}, { 93, 106}}, //15 up 2
	
	{cuphead_ArcMain_Right, {  0,   0, 111, 103}, { 90, 103}}, //16 right 1
	{cuphead_ArcMain_Right, {111,   0, 94,  105}, { 79, 105}}, //17 right 2
	{cuphead_ArcMain_Right, {  0, 103, 92,  106}, { 79, 106}}, //18 right 1
	{cuphead_ArcMain_Right, { 92, 105, 93,  106}, { 80, 106}}, //19 right 2
};

static const Animation char_cuphead_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 0}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_IdleAlt
	{2, (const u8[]){ 4, 5, 6, 7, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//cuphead character functions
void Char_cuphead_SetFrame(void *user, u8 frame)
{
	Char_cuphead *this = (Char_cuphead*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_cuphead_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_cuphead_Tick(Character *character)
{
	Char_cuphead *this = (Char_cuphead*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_cuphead_SetFrame);
	Character_Draw(character, &this->tex, &char_cuphead_frame[this->frame]);
}

void Char_cuphead_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_cuphead_Free(Character *character)
{
	Char_cuphead *this = (Char_cuphead*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_cuphead_New(fixed_t x, fixed_t y)
{
	//Allocate cuphead object
	Char_cuphead *this = Mem_Alloc(sizeof(Char_cuphead));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_cuphead_New] Failed to allocate cuphead object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_cuphead_Tick;
	this->character.set_anim = Char_cuphead_SetAnim;
	this->character.free = Char_cuphead_Free;
	
	Animatable_Init(&this->character.animatable, char_cuphead_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(0,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CUPHEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //cuphead_ArcMain_Idle0
		"left.tim",  //cuphead_ArcMain_Left
		"down.tim",  //cuphead_ArcMain_Down
		"up.tim",    //cuphead_ArcMain_Up
		"right.tim", //cuphead_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
