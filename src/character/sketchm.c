/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sketchm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//sketchm character structure
enum
{
	sketchm_ArcMain_Idle0,
	sketchm_ArcMain_Idle1,
	sketchm_ArcMain_Idle2,
	
	sketchm_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[sketchm_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_sketchm;

//sketchm character definitions
static const CharFrame char_sketchm_frame[] = {
	{sketchm_ArcMain_Idle0, {  0,   0, 228, 187}, { 0, 0}}, //0 idle 1
	{sketchm_ArcMain_Idle1, {  0,   0, 228, 187}, { 1, 0}}, //1 idle 2
	{sketchm_ArcMain_Idle2, {  0,   0, 228, 188}, { 0, 1}}, //2 idle 3
};

static const Animation char_sketchm_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 1, 2, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
};

//sketchm character functions
void Char_sketchm_SetFrame(void *user, u8 frame)
{
	Char_sketchm *this = (Char_sketchm*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sketchm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_sketchm_Tick(Character *character)
{
	Char_sketchm *this = (Char_sketchm*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_sketchm_SetFrame);
	Character_Draw(character, &this->tex, &char_sketchm_frame[this->frame]);
}

void Char_sketchm_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_sketchm_Free(Character *character)
{
	Char_sketchm *this = (Char_sketchm*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_sketchm_New(fixed_t x, fixed_t y)
{
	//Allocate sketchm object
	Char_sketchm *this = Mem_Alloc(sizeof(Char_sketchm));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_sketchm_New] Failed to allocate sketchm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_sketchm_Tick;
	this->character.set_anim = Char_sketchm_SetAnim;
	this->character.free = Char_sketchm_Free;
	
	Animatable_Init(&this->character.animatable, char_sketchm_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(0,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
	//Load art
	this->arc_main = IO_Read("\\MENU\\MENU.ARC;1");
	
	const char **pathp = (const char *[]){
		"sketch0.tim", //sketchm_ArcMain_Idle0
		"sketch1.tim", //sketchm_ArcMain_Idle0
		"sketch2.tim", //sketchm_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
