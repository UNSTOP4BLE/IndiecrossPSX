/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sans.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//sans character structure
enum
{
	sans_ArcMain_Idle0,
	sans_ArcMain_Idle1,
	sans_ArcMain_Idle2,
	sans_ArcMain_Idle3,
	sans_ArcMain_Left,
	sans_ArcMain_Down,
	sans_ArcMain_Up0,
	sans_ArcMain_Up1,
	sans_ArcMain_Right0,
	sans_ArcMain_Right1,

	sans_ArcMain_Idleb0,
	sans_ArcMain_Idleb1,
	sans_ArcMain_Leftb0,
	sans_ArcMain_Leftb1,
	sans_ArcMain_Downb,
	sans_ArcMain_Upb0,
	sans_ArcMain_Upb1,
	sans_ArcMain_Rightb0,
	sans_ArcMain_Rightb1,
	
	sans_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[sans_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_sans;

//sans character definitions
static const CharFrame char_sans_frame[] = {
	{sans_ArcMain_Idle0, {  0,   0, 122, 130}, { 100, 130}}, //0 idle 1
	{sans_ArcMain_Idle0, {126,   0, 122, 130}, {  99, 129}}, //1 idle 2
	{sans_ArcMain_Idle1, {  0,   0, 123, 130}, { 100, 130}}, //2 idle 3
	{sans_ArcMain_Idle1, {126,   0, 122, 130}, { 101, 130}}, //3 idle 4
	{sans_ArcMain_Idle2, {  0,   1, 122, 129}, { 102, 129}}, //4 idle 1
	{sans_ArcMain_Idle2, {125,   1, 123, 129}, { 104, 129}}, //5 idle 2
	{sans_ArcMain_Idle3, {  0,   1, 122, 129}, { 104, 129}}, //6 idle 3
	{sans_ArcMain_Idle3, {125,   1, 123, 129}, { 103, 129}}, //7 idle 4
	
	{sans_ArcMain_Left, {  0,   0, 134, 126}, { 121, 125}}, //8 left 1
	{sans_ArcMain_Left, {  0, 126, 127, 130}, { 112, 129}}, //9 left 2
	{sans_ArcMain_Left, {127, 126, 121, 130}, { 108, 130}}, //10 left 1
	
	{sans_ArcMain_Down, {  0,   0, 122, 120}, { 105, 119}}, //11 down 1
	{sans_ArcMain_Down, {122,   0, 118, 124}, { 102, 124}}, //12 down 2
	{sans_ArcMain_Down, {  0, 120, 117, 126}, { 101, 126}}, //13 down 1
	{sans_ArcMain_Down, {117, 124, 117, 127}, { 100, 126}}, //14 down 2
	
	{sans_ArcMain_Up0, {  0,   0, 107, 137}, { 98, 136}}, //15 up 1
	{sans_ArcMain_Up0, {107,   0, 112, 131}, { 102, 130}}, //16 up 2
	{sans_ArcMain_Up1, {  0,   0, 113, 130}, { 103, 130}}, //17 up 1
	{sans_ArcMain_Up1, {113,   0, 113, 131}, { 102, 130}}, //18 up 2
	
	{sans_ArcMain_Right0, {  0,   0, 118, 130}, { 101, 129}}, //19 right 1
	{sans_ArcMain_Right0, {118,   0, 114, 130}, { 99, 129}}, //20 right 2
	{sans_ArcMain_Right1, {  0,   0, 113, 130}, { 100, 130}}, //21 right 1
	{sans_ArcMain_Right1, {113,   0, 115, 130}, { 102, 130}}, //22 right 2





	{sans_ArcMain_Idleb0, {  0,   0, 113, 125}, { 104 - 128, 124 + 10}}, //23 idle 1
	{sans_ArcMain_Idleb0, {113,   0, 106, 129}, { 100 - 128, 128 + 10}}, //24 idle 2
	{sans_ArcMain_Idleb0, {  0, 127, 107, 129}, { 101 - 128, 128 + 10}}, //25 idle 3
	{sans_ArcMain_Idleb1, {  0,   0, 106, 128}, { 101 - 128, 128 + 10}}, //26 idle 4
	{sans_ArcMain_Idleb1, {106,   0, 107, 127}, { 103 - 128, 127 + 10}}, //27 idle 1
	{sans_ArcMain_Idleb1, {  0, 128, 107, 128}, { 100 - 128, 128 + 10}}, //28 idle 2
	{sans_ArcMain_Idleb1, {107, 127, 106, 129}, { 99 - 128, 129 + 10}}, //29 idle 2
	
	{sans_ArcMain_Leftb0, {  0,   0, 118, 130}, { 117 - 128, 130 + 10}}, //30 left 1
	{sans_ArcMain_Leftb0, {118,   0, 108, 130}, { 105 - 128, 130 + 10}}, //31 left 2
	{sans_ArcMain_Leftb1, {  0,   0, 105, 131}, { 102 - 128, 130 + 10}}, //32 left 1
	{sans_ArcMain_Leftb1, {105,   0, 104, 129}, { 101 - 128, 129 + 10}}, //33 left 1
	
	{sans_ArcMain_Downb, {  0,   0, 114, 121}, { 105 - 128, 121 + 10}}, //34 down 1
	{sans_ArcMain_Downb, {114,   0, 110, 127}, { 102 - 128, 127 + 10}}, //35 down 2
	{sans_ArcMain_Downb, {  0, 127, 109, 129}, { 101 - 128, 129 + 10}}, //36 down 1
	{sans_ArcMain_Downb, {109, 127, 107, 129}, { 101 - 128, 129 + 10}}, //37 down 2

	{sans_ArcMain_Upb0, {  0,   0,  97, 137}, { 92 - 128,  137 + 10}}, //38 up 1
	{sans_ArcMain_Upb0, {100,   0, 106, 131}, { 100 - 128, 130 + 10}}, //39 up 2
	{sans_ArcMain_Upb1, {  0,   0, 105, 131}, { 100 - 128, 130 + 10}}, //40 up 1
	{sans_ArcMain_Upb1, {108,   0, 105, 131}, { 101 - 128, 131 + 10}}, //41 up 2
	
	{sans_ArcMain_Rightb0, {  0,   0, 120, 129}, { 97 - 128, 128 + 10}}, //42 right 1
	{sans_ArcMain_Rightb0, {120,   0, 110, 129}, { 91 - 128, 128 + 10}}, //43 right 2
	{sans_ArcMain_Rightb1, {  0,   0, 110, 129}, { 96 - 128, 128 + 10}}, //44 right 1
	{sans_ArcMain_Rightb1, {113,   0, 107, 129}, { 97 - 128, 128 + 10}}, //45 right 2
};

static const Animation char_sans_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{3, (const u8[]){ 23, 24, 25, 26, 27, 28, 29, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_IdleAlt
	{2, (const u8[]){ 8, 9, 10, ASCR_BACK, 0}},         //CharAnim_Left
	{2, (const u8[]){ 30, 31, 32, 33, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 11, 12, 13, 14, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ 34, 35, 36, 37, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_DownAlt
	{2, (const u8[]){ 15, 16, 17, 18, ASCR_BACK, 0}},         //CharAnim_Up
	{2, (const u8[]){ 38, 39, 40, 41, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_UpAlt
	{2, (const u8[]){ 19, 20, 21, 22, ASCR_BACK, 0}},         //CharAnim_Right
	{2, (const u8[]){ 42, 43, 44, 45, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_RightAlt
};

//sans character functions
void Char_sans_SetFrame(void *user, u8 frame)
{
	Char_sans *this = (Char_sans*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sans_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_sans_Tick(Character *character)
{
	Char_sans *this = (Char_sans*)character;
	
	if (stage.stage_id == StageId_1_2 && stage.song_step == 800) {
		character->set_anim(character, CharAnim_IdleAlt);
	}


	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_sans_SetFrame);
	Character_Draw(character, &this->tex, &char_sans_frame[this->frame]);
}

void Char_sans_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_sans_Free(Character *character)
{
	Char_sans *this = (Char_sans*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_sans_New(fixed_t x, fixed_t y)
{
	//Allocate sans object
	Char_sans *this = Mem_Alloc(sizeof(Char_sans));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_sans_New] Failed to allocate sans object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_sans_Tick;
	this->character.set_anim = Char_sans_SetAnim;
	this->character.free = Char_sans_Free;
	
	Animatable_Init(&this->character.animatable, char_sans_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(0,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SANS.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //sans_ArcMain_Idle0
		"idle1.tim", //sans_ArcMain_Idle0
		"idle2.tim", //sans_ArcMain_Idle0
		"idle3.tim", //sans_ArcMain_Idle0
		"left.tim",  //sans_ArcMain_Left
		"down.tim",  //sans_ArcMain_Down
		"up0.tim",    //sans_ArcMain_Up
		"up1.tim",    //sans_ArcMain_Up
		"right0.tim", //sans_ArcMain_Right
		"right1.tim", //sans_ArcMain_Right

		"idleb0.tim", //sans_ArcMain_Idle0
		"idleb1.tim", //sans_ArcMain_Idle0
		"leftb0.tim",  //sans_ArcMain_Left
		"leftb1.tim",  //sans_ArcMain_Left
		"downb.tim",  //sans_ArcMain_Down
		"upb0.tim",    //sans_ArcMain_Up
		"upb1.tim",    //sans_ArcMain_Up
		"rightb0.tim", //sans_ArcMain_Right
		"rightb1.tim", //sans_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
