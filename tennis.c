//_____________________________________________________________________________
//  ▄▄▄▄ ▄            ▄▄     ▄▄▄                ▄▄  
//  ██▄  ▄  ██▀▄ ▄▀██ ██    ▀█▄  ▄█▄█ ▄▀██  ██▀ ██▄ 
//  ██   ██ ██ █ ▀▄██ ▀█▄   ▄▄█▀ ██ █ ▀▄██ ▄██  ██ █
//_____________________________________________________________________________

// MSXgl
#include "core.h"
#include "vdp.h"
#include "color.h"
#include "bios_hook.h"
#include "system_port.h"
#include "input.h"
#include "math.h"
#include "string.h"
#include "game.h"
#include "game_menu.h"
#include "print.h"
#include "pt3/pt3_player.h"
#include "ayfx/ayfx_player.h"

//=============================================================================
//
//   D E F I N E S
//
//=============================================================================

//-----------------------------------------------------------------------------
// CONFIG
#define GAME_VERSION				"V0.3.0"
#define DEBUG						1
#define MSX2_ENHANCE				1
#define MUSIC_ENABLE				1
#define SFX_ENABLE					1
#define DISPLAY_CREDITS				1
#define COMPRESS_NAMES				0

// VRAM Tables Address
#define VRAM_LAYOUT_TABLE			0x3800
#define VRAM_COLOR_TABLE			0x2000
#define VRAM_PATTERN_TABLE			0x0000
#define VRAM_SPRITE_ATTRIBUTE		0x3E00
#define VRAM_SPRITE_PATTERN			0x1800

// Unit conversion (Pixel <> Q10.6)
#define UNIT_TO_PX(a)				(u8)((a) / 64)
#define PX_TO_UNIT(a)				(i16)((a) * 64)

#define GRAVITY						PX_TO_UNIT(0.2)

#define SPEED_BASE					1.6
#define MOVE_DIAG					PX_TO_UNIT(SPEED_BASE * 0.71)
#define MOVE_UNIT					PX_TO_UNIT(SPEED_BASE * 1.00)

#define PLY_MIN_X					PX_TO_UNIT(8)
#define PLY_MAX_X					PX_TO_UNIT(255 - 8)
#define PLY_UP_MIN_X				PX_TO_UNIT(8)
#define PLY_UP_MAX_X				PX_TO_UNIT(88)
#define PLY_DOWN_MIN_X				PX_TO_UNIT(97 + 8)
#define PLY_DOWN_MAX_X				PX_TO_UNIT(191 + 8)

#define SHOT_DETECT_X				20
#define SHOT_DETECT_Y				4
#define SHOT_DETECT_SMASH			10

// Menu layout
#define MENU_ITEMS_X				8
#define MENU_ITEMS_X_GOTO			12
#define MENU_ITEMS_Y				14
#define MENU_ITEMS_H				(24-MENU_ITEMS_Y)

// Index offset
#define OFFSET_TITLE_LOGO			0
#define OFFSET_TITLE_FONT_DEF		112
#define OFFSET_TITLE_FONT_ALT		176
#define OFFSET_TITLE_MISC			240
#define OFFSET_GAME_COURT			0
#define OFFSET_GAME_SCRFONT			80
#define OFFSET_GAME_FONT			96
#define OFFSET_GAME_SCORE			160
#define OFFSET_GAME_REFEREE			208

#if (DEBUG)
	#define DEBUG_CODE(code) code
#else
	#define DEBUG_CODE(code)
#endif

// Input check macros
#define KEY_PRESS(key)				(((g_KeyRow[KEY_ROW(key)] & KEY_FLAG(key)) == 0) && ((g_PrevRow[KEY_ROW(key)] & KEY_FLAG(key)) != 0))
#define KEY_RELEASE(key)			(((g_KeyRow[KEY_ROW(key)] & KEY_FLAG(key)) != 0) && ((g_PrevRow[KEY_ROW(key)] & KEY_FLAG(key)) == 0))
#define KEY_ON(key)					((g_KeyRow[KEY_ROW(key)] & KEY_FLAG(key)) == 0)
#define KEY_OFF(key)				((g_KeyRow[KEY_ROW(key)] & KEY_FLAG(key)) != 0)

#define SCROLL_BUF_SIZE				(22)
#define SCORE_COLOR 				COLOR_CYAN
#define SCORE_SHADE					COLOR_DARK_BLUE
#define SCORE_TOP_X					(22)
#define SCORE_TOP_Y					(86-16)
#define SCORE_BOT_X					(250)
#define SCORE_BOT_Y					(90-16)

#define EVENT_TOP_X					(24)
#define EVENT_TOP_Y					(70)
#define EVENT_BOT_X					(240)
#define EVENT_BOT_Y					(EVENT_TOP_Y+16)

#define SCORE_BOARD_Y				1
#define SCORE_TXT_Y					(SCORE_BOARD_Y+5)

#define EVENT_DELAY					(50)

#if (COMPRESS_NAMES)
	#define GM2_LAYOUT(src, x0, y0, x1, y1) { UnpackRLEpToRAM(src, g_UnpackBuffer); VDP_WriteLayout_GM2(g_UnpackBuffer, x0, y0, x1, y1); }
#else
	#define GM2_LAYOUT(src, x0, y0, x1, y1) VDP_WriteLayout_GM2(src, x0, y0, x1, y1);
#endif

/// Level types
enum LEVEL
{
	LEVEL_EASY = 0,
	LEVEL_MEDIUM,
	LEVEL_HARD,
	LEVEL_MAX,
};

/// Ball spin types
enum SPIN
{
	SPIN_TOP = 0,
	SPIN_FLAT,
	SPIN_BACK,
	SPIN_MAX,
};

/// Ball shot types
enum SHOT
{
	SHOT_FLAT = 0,	// Flat shot
	SHOT_SLICE,		// Drop shot / lob
	SHOT_ATTACK,	// Top spin shot
	SHOT_MAX,
};

/// Training shot options
enum TRAIN_SHOT
{
	TRAIN_BOTH = 0,
	TRAIN_FLAT,
	TRAIN_LOB,
	TRAIN_MAX,
};

/// Player input direction
enum TAIN_DIR
{
	DIR_RANDOM = 0,
	DIR_CENTER,
	DIR_RIGHT,
	DIR_LEFT,
	DIR_MAX,
};

/// Player actions
enum ACTION
{
	ACTION_IDLE = 0,
	ACTION_WALK_F,
	ACTION_WALK_R,
	ACTION_WALK_L,
	ACTION_SHOOT_R,
	ACTION_SHOOT_L,
	ACTION_SMASH,
	ACTION_THROW,
	ACTION_SERVE,
	ACTION_WIN,
	ACTION_LOOSE,
	ACTION_MAX,
};

/// Player actions special functions
enum FUNCT
{
	FUNCT_NONE = 0,
	FUNCT_PREPARE,
	FUNCT_SHOOT_R,
	FUNCT_SHOOT_L,
	FUNCT_SMASH,
	FUNCT_THROW,
	FUNCT_SERVE,
	FUNCT_CUP,
	FUNCT_MAX,
};

/// Frequence
enum FREQ
{
	FREQ_60HZ,							// Force 60 Hz
	FREQ_50HZ,							// Force 50 Hz
	FREQ_AUTO,							// Use auto-detection
	FREQ_MAX,
};

/// Sprite list
enum SPRITE_ID
{
	SPRITE_PLY_BOT_BLACK_H = 0,			// 0
	SPRITE_PLY_BOT_BLACK_L,				// 1
	SPRITE_PLY_BOT_RACKET,				// 2

	SPRITE_BALL_OUTLINE,				// 3

	SPRITE_PLY_BOT_CLOTH,				// 4
	SPRITE_PLY_BOT_WHITE_H,				// 5
	SPRITE_PLY_BOT_WHITE_L,				// 6
	SPRITE_PLY_BOT_SKIN_H,				// 7
	SPRITE_PLY_BOT_SKIN_L,				// 8

	SPRITE_NET_LEFT,					// 9
	SPRITE_NET_RIGHT, 					// 10
	SPRITE_NET_GRAY,					// 11

	SPRITE_PLY_TOP_BLACK_H,				// 12
	SPRITE_PLY_TOP_BLACK_L,				// 13
	SPRITE_PLY_TOP_RACKET,				// 14

	SPRITE_PLY_TOP_CLOTH,				// 15
	SPRITE_PLY_TOP_WHITE_H,				// 16
	SPRITE_PLY_TOP_WHITE_L,				// 17
	SPRITE_PLY_TOP_SKIN_H,				// 18
	SPRITE_PLY_TOP_SKIN_L,				// 19

	SPRITE_BALL_BODY,					// 20

	SPRITE_SCORE_1,						// 21
	SPRITE_SCORE_2,						// 22
	SPRITE_SCORE_3,						// 23
	SPRITE_SCORE_4,						// 24
	SPRITE_SCORE_5,						// 25
	SPRITE_SCORE_6,						// 26

	SPRITE_SCORE_BALL_1,                // 27
	SPRITE_SCORE_BALL_2,                // 28
	SPRITE_SCORE_BALL_3,                // 29
	SPRITE_SCORE_BALL_4,                // 30

	SPRITE__31,                			// 31

	SPRITE_MAX, //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	SPRITE_CURSOR_1       = SPRITE_SCORE_1, // Easy + Medium + Hard
	SPRITE_CURSOR_2       = SPRITE_SCORE_2, // Easy + Medium
	SPRITE_CURSOR_3       = SPRITE_SCORE_3, // Easy
	SPRITE_CURSOR_4       = SPRITE_SCORE_4, // Easy

	SPRITE_BALL_SHADOW    = SPRITE_SCORE_5,
	SPRITE_BALL_MARK      = SPRITE_SCORE_6,

	SPRITE_LAUNCHER_BOT_1 = SPRITE_PLY_BOT_BLACK_H,	
	SPRITE_LAUNCHER_BOT_2 = SPRITE_PLY_BOT_BLACK_L,		
	SPRITE_LAUNCHER_BOT_3 = SPRITE_PLY_BOT_RACKET,		
	SPRITE_LAUNCHER_BOT_4 = SPRITE_PLY_BOT_CLOTH,
	SPRITE_LAUNCHER_BOT_5 = SPRITE_PLY_BOT_WHITE_H,
	// Not used
	SPRITE_LAUNCHER_BOT_6 = SPRITE_PLY_BOT_WHITE_L,
	SPRITE_LAUNCHER_BOT_7 = SPRITE_PLY_BOT_SKIN_H,
	SPRITE_LAUNCHER_BOT_8 = SPRITE_PLY_BOT_SKIN_L,

	SPRITE_LAUNCHER_TOP_1 = SPRITE_PLY_TOP_BLACK_H,	
	SPRITE_LAUNCHER_TOP_2 = SPRITE_PLY_TOP_BLACK_L,		
	SPRITE_LAUNCHER_TOP_3 = SPRITE_PLY_TOP_RACKET,		
	SPRITE_LAUNCHER_TOP_4 = SPRITE_PLY_TOP_CLOTH,
	SPRITE_LAUNCHER_TOP_5 = SPRITE_PLY_TOP_WHITE_H,
	// Not used
	SPRITE_LAUNCHER_TOP_6 = SPRITE_PLY_TOP_WHITE_L,
	SPRITE_LAUNCHER_TOP_7 = SPRITE_PLY_TOP_SKIN_H,
	SPRITE_LAUNCHER_TOP_8 = SPRITE_PLY_TOP_SKIN_L,

	SPRITE_CUP_OUTLINE    = SPRITE_SCORE_BALL_1,
	SPRITE_CUP_BODY       = SPRITE_SCORE_BALL_2,
};

enum SPRITE_PATTERN
{
	// Bottom player
	PATTERN_PLY_BOT_LINE1_H =   0, // #000
	PATTERN_PLY_BOT_LINE2_H =   4, // #004
	PATTERN_PLY_BOT_CLOTH   =   8, // #008
	PATTERN_PLY_BOT_WHITE_H =  12, // #012
	PATTERN_PLY_BOT_SKIN_H  =  16, // #016
	PATTERN_PLY_BOT_LINE1_L =  20, // #020
	PATTERN_PLY_BOT_LINE2_L =  24, // #024
	PATTERN_PLY_BOT_WHITE_L =  28, // #028
	PATTERN_PLY_BOT_SKIN_L  =  32, // #032
	// Top player
	PATTERN_PLY_TOP_LINE1_H =  36, // #036
	PATTERN_PLY_TOP_LINE2_H =  40, // #040
	PATTERN_PLY_TOP_WHITE_H =  44, // #044
	PATTERN_PLY_TOP_SKIN_H  =  48, // #048
	PATTERN_PLY_TOP_LINE1_L =  52, // #052
	PATTERN_PLY_TOP_LINE2_L =  56, // #056
	PATTERN_PLY_TOP_CLOTH   =  60, // #060
	PATTERN_PLY_TOP_WHITE_L =  64, // #064
	PATTERN_PLY_TOP_SKIN_L  =  68, // #068
	// Ball
	PATTERN_BALL_LINE1      =  72, // #072
	PATTERN_BALL_LINE2      =  76, // #076
	PATTERN_BALL_BODY       =  80, // #080
	PATTERN_BALL_SHADE      =  84, // #084
	PATTERN_BALL_MARK       =  88, // #088
	// Racket
	PATTERN_RACKET_R        =  92, // #092
	PATTERN_RACKET_L        =  96, // #096
	PATTERN_RACKET_T        = 100, // #100
	PATTERN_104             = 104, // #104
	// Net
	PATTERN_NET1            = 108, // #108
	PATTERN_NET2            = 112, // #112
	PATTERN_NET3            = 116, // #116
	PATTERN_NET4            = 120, // #120
	PATTERN_NET5            = 124, // #124
	// Bottom player
	PATTERN_128             = 128, // #128
	PATTERN_132             = 132, // #132
	PATTERN_136             = 136, // #136
	PATTERN_140             = 140, // #140
	PATTERN_144             = 144, // #144
	PATTERN_148             = 148, // #148
	PATTERN_152             = 152, // #152
	PATTERN_156             = 156, // #156
	PATTERN_160             = 160, // #160
	// Top player
	PATTERN_164             = 164, // #164
	PATTERN_168             = 168, // #168
	PATTERN_172             = 172, // #172
	PATTERN_176             = 176, // #176
	PATTERN_180             = 180, // #180
	PATTERN_184             = 184, // #184
	PATTERN_188             = 188, // #188
	PATTERN_192             = 192, // #192
	PATTERN_196             = 196, // #196
	// 
	PATTERN_200             = 200, // #200
	PATTERN_204             = 204, // #204
	PATTERN_CURSOR1         = 208, // #208
	PATTERN_CURSOR2         = 212, // #212
	// Cup sprites
	PATTERN_CUP_OUTLINE1    = 216, // #216
	PATTERN_CUP_OUTLINE2    = 220, // #220
	PATTERN_CUP_BODY        = 224, // #224
	// Server icon
	PATTERN_SERVER_BASE     = 228, // #228
	PATTERN_SERVER_SHADE    = 232, // #232
	PATTERN_SERVER_STRIP    = 236, // #236
	PATTERN_SERVER_OUTLINE  = 240, // #240
	// Score
	PATTERN_SCORE_A         = 244, // #244
	PATTERN_SCORE_B         = 248, // #248
	PATTERN_SCORE_PLY       = 252, // #252
};

/// Menu pages enumeration
enum
{
	MENU_MAIN = 0,
	MENU_MATCH1P,
	MENU_MATCH2P,
	MENU_TRAINING,
	MENU_OPTIONS,
	MENU_CREDIT,
	MENU_MAX,
};

/// Point validation type
enum POINT_VALIDATION
{
	POINT_PENDING = 0,	// Before normal shot's first bounce
	POINT_SERVING,		// Before serve first bounce
	POINT_VALIDATED,	// First bounce was valid
	POINT_FINISHED,		// Point already checked
	POINT_MAX,
};

// Tennis score types
enum SCORE
{
	SCORE_0 = 0,
	SCORE_15,
	SCORE_30,
	SCORE_40,
	SCORE_ADV,
	SCORE_WIN,
};

/// Rule event
enum EVENT
{
	EVENT_GAME = 0,		// 0
	EVENT_SET,			// 1
	EVENT_MATCH,		// 2
	EVENT_ACE,			// 3
	EVENT_OUT,			// 4
	EVENT_NET,			// 5
	EVENT_LET,			// 6
	EVENT_FAULT,		// 7
	EVENT_DEUCE,		// 8
	EVENT_DBLFAULT,
	EVENT_POINT,
	EVENT_NONE,
	EVENT_INOUT,
	EVENT_IN,
};

enum SIDE
{
	SIDE_BOTTOM = 0,
	SIDE_TOP,
};

enum SERVE_ZONE
{
	SERVE_BOT_R = 0,
	SERVE_BOT_L,
	SERVE_TOP_L,
	SERVE_TOP_R,
};

enum BIND
{
	BIND_KB1A = 0,	// Up, Left, Down, Right + Space, N
	BIND_KB1B,  	// Up, Left, Down, Right + Shift, Ctrl
	BIND_KB1C,  	// Up, Left, Down, Right + Return, BS
	BIND_KB2,   	// E, S, D, F + Shift, Ctrl
	BIND_JOY1,  	// Joystick 1 stick + triggers
	BIND_JOY2,  	// Joystick 2 stick + triggers
	BIND_MAX,
};

enum CONTROL
{
	CONTROL_P1 = 0,
	CONTROL_P2,
	CONTROL_AI
};

enum MODE
{
	MODE_1P = 0,
	MODE_2P,
	MODE_TRAIN
};


//-----------------------------------------------------------------------------
// TYPES

// Functions
typedef bool (*InputCB)(void);	///< Input check function signature
typedef void (*ScoreCB)(u8);	///< Score callback

/// 8-bits vector structure
typedef struct
{
	u8			x;	
	u8			y;	
} Vector8;

/// 16-bits vector structure
typedef struct
{
	i16			x;	
	i16			y;	
} Vector16;

/// Binding
typedef struct
{
	const c8*   name;
	InputCB		inHoldUp;
	InputCB		inHoldDown;
	InputCB		inHoldLeft;
	InputCB		inHoldRight;
	InputCB		inPressUp;
	InputCB		inPressDown;
	InputCB		inPressLeft;
	InputCB		inPressRight;
	InputCB		inButton1;
	InputCB		inButton2;
} Binding;

/// Player structure
typedef struct
{
	Vector16	pos;
	Vector8		srcPos;	// Ball screen position (Q8.0 format)
	u8			side;
	u8			control;
	u8			action;
	u8			counter;
	u8			shot;
	u8			shotDir;
	u8			shotCnt;
	u8			step;
	u8			anim;
	u8			prevAnim; // buffered animation frame
	const Binding* binding;
	InputCB		inLong;
	InputCB		inShort;
} Player;

/// Player structure
typedef struct
{
	Vector16	pos;	// Ball position (Q10.6 format)
	Vector8		srcPos;	// Ball screen position (Q8.0 format)
	i16			height; // Ball height (Q10.6 format)
	u8			dir;	// Ball direction (0-63)
	i16			velXY;  // Ball horizontal velocity (Q10.6 format)
	i16			velZ;   // Ball vertical velocity (Q10.6 format)
	u8          coolDown;
	u8			lastPly;
	u8			bounce;
	u8			point;
	u8			spin;
	bool		physics;
} Ball;

/// Animation strcture
typedef struct
{
	u8			frame;
	u8			len;
	u8			event;
} Anim;

/// Action strcture
typedef struct
{
	const Anim* animFrames;
	u8			animLen;
	u8			loop;      // looping action?
	u8			interrupt; // can be interrupted?
} Action;

///
typedef struct
{
	Vector8		min;
	Vector8		max;
} Field;

///
typedef struct
{
	i16			velXY;  // Ball horizontal velocity (Q10.6 format)
	i16			velZ;   // Ball vertical velocity (Q10.6 format)
	u8			spin;
} Shot;

///
typedef struct
{
	Vector8		pos;
	u8			dir;
	u8			y;
	u8			pattern;
} TrainSide;

/// Serve information (start position, X-coordinate limit and valid serving zone)
typedef struct
{
	Vector8		startPos;			///< Player start position (screen coordinate)
	i16			minX;				///< Minimum X coordinate (Q10.6 format)
	i16			maxX;				///< Maximum X coordinate (Q10.6 format)
	Vector8		zoneMin;			///< Serving zone minimum coordinate (screen coordinate)
	Vector8		zoneMax;			///< Serving zone maximum coordinate (screen coordinate)
} ServeInfo;

//-----------------------------------------------------------------------------
// FUNCTIONS PROTOTYPE

// States callback
bool State_Init();
bool State_TitleStart();
bool State_TitleUpdate();
bool State_MenuStart();
bool State_MenuUpdate();
bool State_ScoreStart();
bool State_ScoreUpdate();
bool State_CourtRestore();
bool State_MatchStart();
bool State_MatchUpdate();
bool State_WinMatch();
bool State_TrainingStart();
bool State_TrainingUpdate();
bool State_VictoryStart();
bool State_VictoryUpdate();

// Menu callback
const char* Menu_StartMatch(u8 op, i8 value);
const char* Menu_StartTrain(u8 op, i8 value);
const char* Menu_SetLevel(u8 op, i8 value);
const char* Menu_SetSets(u8 op, i8 value);
const char* Menu_SetShot(u8 op, i8 value);
const char* Menu_SetDir(u8 op, i8 value);
const char* Menu_SetSide(u8 op, i8 value);
const char* Menu_CreditScroll(u8 op, i8 value);
const char* Menu_SetInput(u8 op, i8 value);
const char* Menu_SetMusic(u8 op, i8 value);
const char* Menu_SetSFX(u8 op, i8 value);
const char* Menu_SetFreq(u8 op, i8 value);

// Input callback
bool KB1_Hold_Up();
bool KB1_Hold_Down();
bool KB1_Hold_Left();
bool KB1_Hold_Right();
bool KB1_Press_Up();
bool KB1_Press_Down();
bool KB1_Press_Left();
bool KB1_Press_Right();
bool KB1A_Button1();
bool KB1A_Button2();
bool KB1B_Button1();
bool KB1B_Button2();
bool KB1C_Button1();
bool KB1C_Button2();
bool KB2_Hold_Up();
bool KB2_Hold_Down();
bool KB2_Hold_Left();
bool KB2_Hold_Right();
bool KB2_Press_Up();
bool KB2_Press_Down();
bool KB2_Press_Left();
bool KB2_Press_Right();
bool KB2_Button1();
bool KB2_Button2();
bool Joy1_Hold_Up();
bool Joy1_Hold_Down();
bool Joy1_Hold_Left();
bool Joy1_Hold_Right();
bool Joy1_Press_Up();
bool Joy1_Press_Down();
bool Joy1_Press_Left();
bool Joy1_Press_Right();
bool Joy1_Button1();
bool Joy1_Button2();
bool Joy2_Hold_Up();
bool Joy2_Hold_Down();
bool Joy2_Hold_Left();
bool Joy2_Hold_Right();
bool Joy2_Press_Up();
bool Joy2_Press_Down();
bool Joy2_Press_Left();
bool Joy2_Press_Right();
bool Joy2_Button1();
bool Joy2_Button2();

// Helpers
void SetSprite(u8 index, u8 x, u8 y, u8 shape, u8 color);

//-----------------------------------------------------------------------------
// EXTERNAL VARIABLES

#if (TARGET_TYPE == TYPE_ROM)
	extern const u8 g_VersionROM;
	extern const u8 g_VersionMSX;
#endif

//_____________________________________________________________________________
//  ▄▄▄   ▄▄  ▄▄▄▄  ▄▄ 
//  ██ █ ██▄█  ██  ██▄█
//  ██▄▀ ██ █  ██  ██ █
//_____________________________________________________________________________

//=============================================================================
//
//  ROM DATA - PAGE 0-2
//
//=============================================================================

// Data
#include "data.h"

// Animation
const Anim g_FramesIdle[] = 
{
	{ 19, 10, FUNCT_NONE },
	{ 18, 10, FUNCT_NONE },
	{ 19, 10, FUNCT_NONE },
	{ 20, 10, FUNCT_NONE },
};
const Anim g_FramesWalkF[] = 
{
	{ 3, 8, FUNCT_NONE },
	{ 4, 8, FUNCT_NONE },
	{ 5, 8, FUNCT_NONE },
	{ 4, 8, FUNCT_NONE },
};
const Anim g_FramesWalkR[] = 
{
	{ 8, 8, FUNCT_NONE },
	{ 7, 8, FUNCT_NONE },
	{ 6, 8, FUNCT_NONE },
	{ 7, 8, FUNCT_NONE },
};
const Anim g_FramesWalkL[] = 
{
	{ 2, 8, FUNCT_NONE },
	{ 1, 8, FUNCT_NONE },
	{ 0, 8, FUNCT_NONE },
	{ 1, 8, FUNCT_NONE },
};
const Anim g_FramesShootR[] = 
{
	{ 15,  4, FUNCT_PREPARE },
	{ 16,  8, FUNCT_SHOOT_R }, // do shoot
	{ 17, 16, FUNCT_NONE },
};
const Anim g_FramesShootL[] = 
{
	{  9,  4, FUNCT_PREPARE },
	{ 10,  8, FUNCT_SHOOT_L }, // do shoot
	{ 11, 16, FUNCT_NONE },
};
const Anim g_FramesSmash[] = 
{
	{ 12,  4, FUNCT_PREPARE },
	{ 13,  8, FUNCT_SMASH }, // do smash
	{ 14, 16, FUNCT_NONE },
};
const Anim g_FramesThrow[] = 
{
	{ 19, 4,  FUNCT_NONE },
	{ 12, 64, FUNCT_THROW }, // do throw
};
const Anim g_FramesServe[] = 
{
	{ 13,  8, FUNCT_SERVE }, // do serve
	{ 14, 16, FUNCT_NONE },
};

const Anim g_FramesWin[] = 
{
	{ 23, 12, FUNCT_CUP },
	{ 24, 12, FUNCT_CUP },
};

const Anim g_FramesLoose[] = 
{
	{ 25, 16, FUNCT_NONE },
	{ 26, 16, FUNCT_NONE },
};

// Actions list
const Action g_Actions[] =
{ //                       frames                                 loop  interupt
	/* ACTION_IDLE    */ { g_FramesIdle,   numberof(g_FramesIdle),   1, 1 },
	/* ACTION_WALK_F  */ { g_FramesWalkF,  numberof(g_FramesWalkF),  1, 1 },
	/* ACTION_WALK_R  */ { g_FramesWalkR,  numberof(g_FramesWalkR),  1, 1 },
	/* ACTION_WALK_L  */ { g_FramesWalkL,  numberof(g_FramesWalkL),  1, 1 },
	/* ACTION_SHOOT_R */ { g_FramesShootR, numberof(g_FramesShootR), 0, 0 },
	/* ACTION_SHOOT_L */ { g_FramesShootL, numberof(g_FramesShootL), 0, 0 },
	/* ACTION_SMASH   */ { g_FramesSmash,  numberof(g_FramesSmash),  0, 0 },
	/* ACTION_THROW   */ { g_FramesThrow,  numberof(g_FramesThrow),  0, 0 },
	/* ACTION_SERVE   */ { g_FramesServe,  numberof(g_FramesServe),  0, 0 },
	/* ACTION_WIN     */ { g_FramesWin,    numberof(g_FramesWin),    1, 0 },
	/* ACTION_LOOSE   */ { g_FramesLoose,  numberof(g_FramesLoose),  1, 0 },
};

// Direction index
// +---+---+---+
// | 0 | 3 | 5 |
// +---+---+---+
// | 1 |   | 6 |
// +---+---+---+
// | 2 | 4 | 7 |
// +---+---+---+

/// Court field area definition
const Field g_Field[2] =
{
	{ { 55, 31 }, { 200, 95 } },	// Top field    (Player 1 target)
	{ { 55, 96 }, { 200, 160 } },	// Bottom field (Player 2 target)
};

/// Serve information
const ServeInfo g_ServeInfo[4] =
{
	//                   startPos     minX             maxX              zoneMin      zoneMax	
	/* SERVE_BOT_R */ { { 164, 161 }, PX_TO_UNIT(128), PX_TO_UNIT(200), {  55, 63 }, { 127,  95 } },
	/* SERVE_BOT_L */ { {  92, 161 }, PX_TO_UNIT( 55), PX_TO_UNIT(127), { 128, 63 }, { 200,  95 } },
	/* SERVE_TOP_L */ { {  92,  30 }, PX_TO_UNIT( 55), PX_TO_UNIT(127), { 128, 96 }, { 200, 128 } },
	/* SERVE_TOP_R */ { { 164,  30 }, PX_TO_UNIT(128), PX_TO_UNIT(200), {  55, 96 }, { 127, 128 } },
};

/// Movement
const Vector16 g_Move[8] =
{
	{ -MOVE_DIAG, -MOVE_DIAG }, // 🡤
	{ -MOVE_UNIT,          0 }, // 🡠
	{ -MOVE_DIAG,  MOVE_DIAG }, // 🡧
	{          0, -MOVE_UNIT }, // 🡡
	{          0,  MOVE_UNIT }, // 🡣
	{  MOVE_DIAG, -MOVE_DIAG }, // 🡥
	{  MOVE_UNIT,          0 }, // 🡢
	{  MOVE_DIAG,  MOVE_DIAG }, // 🡦
};

/// 
const Shot g_Shots[12] =
{
	// Flat shot - Long
	{ PX_TO_UNIT(3.0),	PX_TO_UNIT(3.5),	SPIN_FLAT },
	// Flat shot - Med
	{ PX_TO_UNIT(2.7),	PX_TO_UNIT(3.0),	SPIN_FLAT },
	// Flat shot - Short
	{ PX_TO_UNIT(2.0),	PX_TO_UNIT(2.5),	SPIN_FLAT },
	// Flat shot - Smash
	{ PX_TO_UNIT(4.0),	PX_TO_UNIT(-0.5),	SPIN_FLAT },
	
	// High shot - Long
	{ PX_TO_UNIT(2.3),	PX_TO_UNIT(5.0),	SPIN_FLAT },	// Lob
	// High shot - Med
	{ PX_TO_UNIT(2.2),	PX_TO_UNIT(4.0),	SPIN_FLAT },
	// High shot - Short
	{ PX_TO_UNIT(1.7),	PX_TO_UNIT(3.0),	SPIN_BACK },	// Down shot
	// High shot - Smash
	{ PX_TO_UNIT(4.0),	PX_TO_UNIT(0.0),	SPIN_FLAT },
	
	// Attack shot - Long
	{ PX_TO_UNIT(3.5),	PX_TO_UNIT(2.0),	SPIN_TOP },
	// Attack shot - Med
	{ PX_TO_UNIT(3.0),	PX_TO_UNIT(2.0),	SPIN_TOP },
	// Attack shot - Short
	{ PX_TO_UNIT(2.0),	PX_TO_UNIT(2.5),	SPIN_FLAT },
	// Attack shot - Smash
	{ PX_TO_UNIT(5.0),	PX_TO_UNIT(-0.5),	SPIN_FLAT },
};

///
const u8 g_ColorShadeDefault[8] =
{ 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xF, 0x1), 
	COLOR_MERGE(0xE, 0x1), 
	COLOR_MERGE(0xE, 0x1)
};

///
const u8 g_ColorShadeSelect[8] =
{ 
	COLOR_MERGE(9, 0x1), 
	COLOR_MERGE(9, 0x1), 
	COLOR_MERGE(8, 0x1), 
	COLOR_MERGE(8, 0x1), 
	COLOR_MERGE(8, 0x1), 
	COLOR_MERGE(6, 0x1), 
	COLOR_MERGE(6, 0x1), 
	COLOR_MERGE(6, 0x1)
};

// Menu 0 - Main
const MenuItem g_MenuMain[] =
{
	{ "TRAINING",	MENU_ITEM_GOTO, NULL, MENU_TRAINING },
	// { "1P MATCH",	MENU_ITEM_GOTO|MENU_MATCH1P, 0, 0 },
	{ "2P MATCH",	MENU_ITEM_GOTO, NULL, MENU_MATCH2P },
	{ "OPTIONS", 	MENU_ITEM_GOTO, NULL, MENU_OPTIONS },
	{ "CREDITS", 	MENU_ITEM_GOTO, NULL, MENU_CREDIT },
	{ "",        	MENU_ITEM_DISABLE, NULL, 0 },
	{ "",        	MENU_ITEM_DISABLE, NULL, 0 },
	{ "",        	MENU_ITEM_DISABLE, NULL, 0 },
	{ "",        	MENU_ITEM_DISABLE, NULL, 0 },
	{ "",        	MENU_ITEM_DISABLE, NULL, 0 },
	{ GAME_VERSION, MENU_ITEM_TEXT, NULL, 4 },
};

// Menu 1 - Match 1P
const MenuItem g_MenuMatch1P[] =
{
	{ "START>",		MENU_ITEM_ACTION, Menu_StartMatch, MODE_1P },
	{ "AI",			MENU_ITEM_ACTION, Menu_SetLevel, 0 },
	{ "SETS",		MENU_ITEM_ACTION, Menu_SetSets, 0 },
	{ "INPUT",		MENU_ITEM_ACTION, Menu_SetInput, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "<BACK",		MENU_ITEM_GOTO, NULL, MENU_MAIN },
};

// Menu 2 - Match 2P
const MenuItem g_MenuMatch2P[] =
{
	{ "START>",		MENU_ITEM_ACTION, Menu_StartMatch, MODE_2P },
	{ "SETS",		MENU_ITEM_ACTION, Menu_SetSets, 0 },
	{ "P1 INPUT",	MENU_ITEM_ACTION, Menu_SetInput, 0 },
	{ "P2 INPUT",	MENU_ITEM_ACTION, Menu_SetInput, 1 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "<BACK",		MENU_ITEM_GOTO, NULL, MENU_MAIN },
};

// Menu 3 - Training
const MenuItem g_MenuTraining[] =
{
	{ "START>",		MENU_ITEM_ACTION, Menu_StartTrain, MODE_TRAIN },
	{ "LEVEL",		MENU_ITEM_ACTION, Menu_SetLevel, 0 },
	{ "SIDE",		MENU_ITEM_ACTION, Menu_SetSide, 0 },
	{ "SHOT",		MENU_ITEM_ACTION, Menu_SetShot, 0 },
	{ "DIR",		MENU_ITEM_ACTION, Menu_SetDir, 0 },
	{ "INPUT",		MENU_ITEM_ACTION, Menu_SetInput, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "<BACK",		MENU_ITEM_GOTO, NULL, MENU_MAIN },
};

// Menu 4 - Option
const MenuItem g_MenuOption[] =
{
	{ "MUSIC",		MENU_ITEM_ACTION, &Menu_SetMusic, 0 },
	{ "SFX",		MENU_ITEM_ACTION, &Menu_SetSFX, 0 },
	{ "SHADE",		MENU_ITEM_BOOL, &g_FlickerShadow, 0 },
	{ "FREQ",		MENU_ITEM_ACTION, Menu_SetFreq, 0 },
	{ "P1 INPUT",	MENU_ITEM_ACTION, Menu_SetInput, 0 },
	{ "P2 INPUT",	MENU_ITEM_ACTION, Menu_SetInput, 1 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "",			MENU_ITEM_DISABLE, NULL, 0 },
	{ "<BACK",		MENU_ITEM_GOTO, NULL, MENU_MAIN },
};

// Menu 5 - Credits
const MenuItem g_MenuCredits[] =
{
	{ "PIXEL PHENIX # 2023",	MENU_ITEM_TEXT, NULL, (i8)-1 },
	{ "",						MENU_ITEM_DISABLE, NULL, 0 },
	{ "CODE:  AOINEKO",			MENU_ITEM_TEXT, NULL, (i8)-2 },
	{ "GRAPH: AOINEKO & GFX",	MENU_ITEM_TEXT, NULL, (i8)-2 },
	// { "MUSIC: ???",				MENU_ITEM_TEXT, NULL, (i8)-2 },
	{ "SFX:   AYFX SAMPLE",		MENU_ITEM_TEXT, NULL, (i8)-2 },
	{ "",						MENU_ITEM_DISABLE, NULL, 0 },
	{ "",						MENU_ITEM_DISABLE, NULL, 0 },
	{ "",						MENU_ITEM_UPDATE, Menu_CreditScroll, (i8)-2 },
	{ "",						MENU_ITEM_DISABLE, NULL, 0 },
	{ "<BACK",					MENU_ITEM_GOTO, NULL, MENU_MAIN },
};

///
const Menu g_Menus[MENU_MAX] =
{
	{ "",		g_MenuMain,			numberof(g_MenuMain) },
	{ "",		g_MenuMatch1P,		numberof(g_MenuMatch1P) },
	{ "",		g_MenuMatch2P,		numberof(g_MenuMatch2P) },
	{ "",  		g_MenuTraining,		numberof(g_MenuTraining) },
	{ "",		g_MenuOption,		numberof(g_MenuOption) },
	{ "",		g_MenuCredits,		numberof(g_MenuCredits) },
};

///
const TrainSide g_TrainSideData[] = 
{
	{ { 128, 24     }, 44, 0,   PATTERN_PLY_TOP_LINE1_H },
	{ { 128, 192-24 }, 12, 144, PATTERN_PLY_BOT_LINE1_H },
};

/// Input bindings
const Binding g_Binding[BIND_MAX] =
{
	// BIND_KB1A - Up, Left, Down, Right + Space, N
	{ "[]+SPACE",	KB1_Hold_Up,	KB1_Hold_Down,	KB1_Hold_Left,	KB1_Hold_Right,		KB1_Press_Up,	KB1_Press_Down,		KB1_Press_Left,		KB1_Press_Right,	KB1A_Button1,	KB1A_Button2 },
	// BIND_KB1B - Up, Left, Down, Right + Shift, Ctrl			
	{ "[]+SHFT",	KB1_Hold_Up,	KB1_Hold_Down,	KB1_Hold_Left,	KB1_Hold_Right,		KB1_Press_Up,	KB1_Press_Down,		KB1_Press_Left,		KB1_Press_Right,	KB1B_Button1,	KB1B_Button2 },
	// BIND_KB1C - Up, Left, Down, Right + Return, BS			
	{ "[]+RET",		KB1_Hold_Up,	KB1_Hold_Down,	KB1_Hold_Left,	KB1_Hold_Right,		KB1_Press_Up,	KB1_Press_Down,		KB1_Press_Left,		KB1_Press_Right,	KB1C_Button1,	KB1C_Button2 },
	// BIND_KB2  - E, S, D, F + Shift, Ctrl			
	{ "ESDF+SHFT",	KB2_Hold_Up,	KB2_Hold_Down,	KB2_Hold_Left,	KB2_Hold_Right,		KB2_Press_Up,	KB2_Press_Down,		KB2_Press_Left,		KB2_Press_Right,	KB1B_Button1,	KB1B_Button2 },
	// BIND_JOY1 - Joystick 1 stick + triggers
	{ "^1",			Joy1_Hold_Up,	Joy1_Hold_Down,	Joy1_Hold_Left,	Joy1_Hold_Right,	Joy1_Press_Up,	Joy1_Press_Down,	Joy1_Press_Left,	Joy1_Press_Right,	Joy1_Button1,	Joy1_Button2 },
	// BIND_JOY2 - Joystick 2 stick + triggers
	{ "^2",			Joy2_Hold_Up,	Joy2_Hold_Down,	Joy2_Hold_Left,	Joy2_Hold_Right,	Joy2_Press_Up,	Joy2_Press_Down,	Joy2_Press_Left,	Joy2_Press_Right,	Joy2_Button1,	Joy2_Button2 },
};

///
const u8 g_CourtNet[32] = { 0xFF, 0xFF, 0xFF, 1, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0x80, 0xFF, 0xFF, 0xFF };

const Vector8 g_TrainSize[LEVEL_MAX] =
{
	{ 32, 32 }, // LEVEL_EASY
	{ 16, 32 }, // LEVEL_MEDIUM
	{ 16, 16 }, // LEVEL_HARD
};

/// Score bard's score text:                           Adv
const c8* g_ScoreString[] = { "00", "15", "30", "40", ":;" };
/// Score board's player text:  P1    P2    AI
const c8* g_PlayerString[] = { ">1", ">2", "<=" };

const c8* g_CreditScroll = "______________________DEDICATED_TO_MY_WONDERFUL_WIFE_AND_SON_$$____THANKS_TO_ALL_MSX-VILLAGE_AND_MRC_MEMBERS_FOR_SUPPORT!____MSX'LL_NEVER_DIE.____PRAISE_THE_HOLY-BRIOCHE!";

//=============================================================================
//
//  RAM DATA - PAGE 3
//
//=============================================================================

// Gameplay
Player 		g_Player[2];
Ball		g_Ball;
ScoreCB		g_ScoreFct = NULL;

// Configuration
u8			g_OptionSets = 1;			///< Sets count (0=1 set, 1=3 sets, 2=5 sets)
u8			g_FlickerShadow = TRUE;
u8			g_PlayMusic = FALSE;
u8			g_PlaySFX = FALSE;
u8			g_InputBinding[2];
u8			g_GameMode;
u8			g_Freq;
u8			g_FreqDetected;
u8			g_FreqOpt = FREQ_AUTO;

// System
#if (MSX2_ENHANCE)
	u8		g_VersionVDP;
#endif
#if (DEBUG)
	u8		g_Debug = 0;
#endif

// Input
u8			g_PrevRow[9];
u8			g_KeyRow[9] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
u8			g_PrevJoy1;
u8			g_PrevJoy2;
u8			g_Joy1 = 0xFF;
u8			g_Joy2 = 0xFF;

// Menu
const Menu*	g_CurrentMenu; 
u8			g_MenuID = 0; 
u8			g_MenuItem = 0; 

// Match
u8			g_Level = LEVEL_EASY;	///< AI level (0=Easy, 1=Medium, 2=Hard)
u8			g_EventID;				///< Match event (@see EVENT enum)
u8			g_EventSub;				///< Match event (@see EVENT enum)
u8			g_EventTimer;			///< Match event timer (event start when timer is equal to 0)
u8			g_EventFrame;
u8			g_Winner;
u8			g_Points[2];
u8			g_Games[5][2];
u8			g_Sets[2];
u8			g_CurSet;
u8			g_NumSet;

// Serving
u8			g_Server;
bool		g_ServeState;
u8			g_ServeZone;
i8			g_ServeDX;
i8			g_ServeDY;

// Stats
bool		g_OddPoint;
u8			g_TotalPoints[2];
u8			g_TotalFault[2];

// Training
u8			g_TrainScore;
u8			g_TrainBest[LEVEL_MAX] = { 0, 0, 0 };
u8			g_TrainSide = SIDE_BOTTOM;
u8			g_TrainShot = TRAIN_BOTH;
u8			g_TrainSpin = SPIN_FLAT;
u8			g_TrainDir = DIR_RANDOM;
Vector8		g_TrainZone;

// Misc
c8			g_CreditScrollBuf[SCROLL_BUF_SIZE];
u8			g_CreditScrollCnt = 0;
u8			g_DisplayNet;
u8			g_UnpackBuffer[512];

//_____________________________________________________________________________
//   ▄▄   ▄▄  ▄▄▄  ▄▄▄▄
//  ██ ▀ ██ █ ██ █ ██▄ 
//  ▀█▄▀ ▀█▄▀ ██▄▀ ██▄▄
//_____________________________________________________________________________

//=============================================================================
//
//   M E N U
//
//=============================================================================

//-----------------------------------------------------------------------------
///
const c8* Menu_StartMatch(u8 op, i8 value)
{
	if(op == MENU_ACTION_SET)
	{
		g_GameMode = value;
		Game_SetState(State_MatchStart);
		ayFX_PlayBank(17, 0);
	}
	return NULL; 
}

//-----------------------------------------------------------------------------
///
const c8* Menu_StartTrain(u8 op, i8 value)
{
	if(op == MENU_ACTION_SET)
	{
		g_GameMode = value;
		Game_SetState(State_TrainingStart);
		ayFX_PlayBank(17, 0);
	}
	return NULL;
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetLevel(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_Level = (g_Level + 1) % 3; break;
	case MENU_ACTION_DEC: g_Level = (g_Level + 2) % 3; break;	
	}

	if(g_Level == LEVEL_EASY)
		return "EASY";
	else if(g_Level == LEVEL_MEDIUM)
		return "MED";

	return "HARD";
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetSets(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_OptionSets = (g_OptionSets + 1) % 3; break;
	case MENU_ACTION_DEC: g_OptionSets = (g_OptionSets + 2) % 3; break;	
	}

	if(g_OptionSets == 0)
		return "1";
	else if(g_OptionSets == 1)
		return "3";

	return "5";
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetShot(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_TrainShot = (g_TrainShot + 1) % TRAIN_MAX; break;
	case MENU_ACTION_DEC: g_TrainShot = (g_TrainShot + (TRAIN_MAX-1)) % TRAIN_MAX; break;	
	}
	
	switch(g_TrainShot)
	{
	case TRAIN_BOTH: return "BOTH";
	case TRAIN_FLAT: return "FLAT";
	case TRAIN_LOB:  return "LOB";
	};
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetDir(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_TrainDir = (g_TrainDir + 1) % DIR_MAX; break;
	case MENU_ACTION_DEC: g_TrainDir = (g_TrainDir + (DIR_MAX-1)) % DIR_MAX; break;	
	}
	
	switch(g_TrainDir)
	{
	case DIR_RANDOM: return "RANDOM";
	case DIR_CENTER: return "CENTER";
	case DIR_RIGHT: return "RIGHT";
	case DIR_LEFT: return "LEFT";
	};
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetMusic(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
	case MENU_ACTION_DEC:
		g_PlayMusic = 1 - g_PlayMusic;
		if(g_PlayMusic)
		{
			PT3_InitSong(g_DataMusic);
			PT3_Resume();
		}
		else
			PT3_Pause();
		break;	
	}
	
	return g_PlayMusic ? "/" : "\\";
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetSFX(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
	case MENU_ACTION_DEC:
		g_PlaySFX = 1 - g_PlaySFX;
		if(!g_PlaySFX)
		{
			ayFX_Mute();
			PT3_UpdatePSG();
		}
		break;	
	}
	
	return g_PlaySFX ? "/" : "\\";
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetInput(u8 op, i8 value)
{
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_InputBinding[value] = (g_InputBinding[value] + 1) % BIND_MAX; break;
	case MENU_ACTION_DEC: g_InputBinding[value] = (g_InputBinding[value] + (BIND_MAX-1)) % BIND_MAX; break;	
	}
	
	return g_Binding[g_InputBinding[value]].name;
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetSide(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
	case MENU_ACTION_DEC:
		 g_TrainSide = 1 - g_TrainSide; break;	
	}
	
	if(g_TrainSide == SIDE_BOTTOM)
		return "BOTTOM";
	else
		return "TOP";
}

//-----------------------------------------------------------------------------
///
const char* Menu_SetFreq(u8 op, i8 value)
{
	value;
	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC: g_FreqOpt = (g_FreqOpt + 1) % FREQ_MAX; break;
	case MENU_ACTION_DEC: g_FreqOpt = (g_FreqOpt + (FREQ_MAX - 1)) % FREQ_MAX; break;
	}
	
	if(g_FreqOpt == FREQ_60HZ) 
	{
		g_Freq = FREQ_60HZ;
		return "60 HZ";
	}
	else if(g_FreqOpt == FREQ_50HZ)
	{
		g_Freq = FREQ_50HZ;
		return "50 HZ";
	}
	else
	{
		g_Freq = g_FreqDetected;
		if(g_Freq == FREQ_50HZ)
			return "AUTO (50 HZ)";
		else
			return "AUTO (60 HZ)";
	}
}

//-----------------------------------------------------------------------------
///
const char* Menu_CreditScroll(u8 op, i8 value)
{
	value;
	if(op == MENU_ACTION_GET)
	{	
		u8 len = String_Length(g_CreditScroll);
		for(u8 i=0; i < SCROLL_BUF_SIZE-1; ++i)
		{
			u8 j = (i + g_CreditScrollCnt) % len;
			g_CreditScrollBuf[i] = g_CreditScroll[j];
		}
		g_CreditScrollBuf[SCROLL_BUF_SIZE-1] = 0;
		g_CreditScrollCnt++;
		g_CreditScrollCnt %= len;
	}
#if (DISPLAY_CREDITS)
	return g_CreditScrollBuf;
#else
	return "";
#endif
}


#pragma save
#pragma disable_warning	244		///< remove "pointer types incompatible" warning

//-----------------------------------------------------------------------------
///
void Menu_DisplayItem(u8 item) __FASTCALL
{
	// Clean buttom-third
	u16 dst = g_ScreenLayoutLow + ((MENU_ITEMS_Y + item) * 32);
	VDP_FillVRAM_16K(0, dst, 32);
	
	// Draw item
	if(item < g_CurrentMenu->ItemNum)
	{
		MenuItem* pCurEntry = &g_CurrentMenu->Items[item];
		if(g_MenuItem == item)
			Print_SelectTextFont(g_DataFont, OFFSET_TITLE_FONT_ALT);
		else
			Print_SelectTextFont(g_DataFont, OFFSET_TITLE_FONT_DEF);
			
		u8 x;
		if((pCurEntry->Type == MENU_ITEM_GOTO) != 0)
			x = MENU_ITEMS_X_GOTO;
		else
			x = MENU_ITEMS_X;
		 
		u8 y = MENU_ITEMS_Y;
		
		if(pCurEntry->Type == MENU_ITEM_TEXT)
			x += pCurEntry->Value;
			
		Print_SetPosition(x, y + item);
		Print_DrawText(pCurEntry->Text);

		Print_SetPosition(x + 9, y + item);

		if(pCurEntry->Type == MENU_ITEM_ACTION)
		{
			Menu_ActionCB cb = (Menu_ActionCB)pCurEntry->Action;
			const c8* str = cb(MENU_ACTION_GET, pCurEntry->Value);
			if(str)
			{
				if(g_MenuItem == item)
					Print_DrawChar('<');
				else
					Print_Space();
				Print_DrawText(str);
				if(g_MenuItem == item)
					Print_DrawChar('>');
			}
		}
		else if(pCurEntry->Type == MENU_ITEM_INT)
		{
			i8* data = (u8*)pCurEntry->Action;
			if(g_MenuItem == item)
				Print_DrawChar('<');
			else
				Print_Space();
			Print_DrawInt(*data);
			if(g_MenuItem == item)
				Print_DrawChar('>');
		}
		else if(pCurEntry->Type == MENU_ITEM_BOOL)
		{
			u8* data = (u8*)pCurEntry->Action;
			if(g_MenuItem == item)
				Print_DrawChar('<');
			else
				Print_Space();
			Print_DrawChar(*data ? '/' : '\\');
			if(g_MenuItem == item)
				Print_DrawChar('>');
		}
	}
}

//-----------------------------------------------------------------------------
///
void Menu_Initi(const Menu* menu) __FASTCALL
{
	// Initialize menu
	g_CurrentMenu = menu;
	g_MenuItem = 0;
	while(g_MenuItem < g_CurrentMenu->ItemNum)
	{
		if(g_CurrentMenu->Items[g_MenuItem].Type < MENU_ITEM_EMPTY)
			break;
		g_MenuItem++;
	}
	
	// Display menu items
	for(u8 item = 0; item < MENU_ITEMS_H; ++item)
	{
		Menu_DisplayItem(item);
	}
}

//-----------------------------------------------------------------------------
///
bool Menu_PressRight()
{
	for(u8 i = 0; i < BIND_MAX; ++i)
		if(g_Binding[i].inPressRight())
			return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool Menu_PressLeft()
{
	for(u8 i = 0; i < BIND_MAX; ++i)
		if(g_Binding[i].inPressLeft())
			return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool Menu_PressUp()
{
	for(u8 i = 0; i < BIND_MAX; ++i)
		if(g_Binding[i].inPressUp())
			return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool Menu_PressDown()
{
	for(u8 i = 0; i < BIND_MAX; ++i)
		if(g_Binding[i].inPressDown())
			return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool Menu_PressTrigger()
{
	for(u8 i = 0; i < BIND_MAX; ++i)
		if(g_Binding[i].inButton1() || g_Binding[i].inButton2())
			return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
///
void Menu_Update()
{
	// Handle activation
	MenuItem* pCurEntry = &g_CurrentMenu->Items[g_MenuItem];
	if(Menu_PressTrigger())
	{
		ayFX_PlayBank(14, 0);
		if(pCurEntry->Type == MENU_ITEM_ACTION)
		{
			Menu_ActionCB cb = (Menu_ActionCB)pCurEntry->Action;
			cb(MENU_ACTION_SET, pCurEntry->Value);
		}
		else if(pCurEntry->Type == MENU_ITEM_INT)
		{
			i8* data = (u8*)pCurEntry->Action;
			*data++;
		}
		else if(pCurEntry->Type == MENU_ITEM_BOOL)
		{
			u8* data = (u8*)pCurEntry->Action;
			*data = 1 - *data;
		}
		Menu_DisplayItem(g_MenuItem);
		if(pCurEntry->Type == MENU_ITEM_GOTO)
		{
			Menu_Initi(&g_Menus[pCurEntry->Value]);
		}
		return;
	}

	// Handle change value
	if(Menu_PressRight())
	{
		if(pCurEntry->Type == MENU_ITEM_ACTION)
		{
			Menu_ActionCB cb = (Menu_ActionCB)pCurEntry->Action;
			cb(MENU_ACTION_INC, pCurEntry->Value);
		}
		else if(pCurEntry->Type == MENU_ITEM_INT)
		{
			i8* data = (u8*)pCurEntry->Action;
			*data++;
		}
		else if(pCurEntry->Type == MENU_ITEM_BOOL)
		{
			u8* data = (u8*)pCurEntry->Action;
			*data = 1 - *data;
		}
		Menu_DisplayItem(g_MenuItem);
		ayFX_PlayBank(14, 0);
	}
	else if(Menu_PressLeft())
	{
		if(pCurEntry->Type == MENU_ITEM_ACTION)
		{
			Menu_ActionCB cb = (Menu_ActionCB)pCurEntry->Action;
			cb(MENU_ACTION_DEC, pCurEntry->Value);
		}
		else if(pCurEntry->Type == MENU_ITEM_INT)
		{
			i8* data = (u8*)pCurEntry->Action;
			*data--;
		}
		else if(pCurEntry->Type == MENU_ITEM_BOOL)
		{
			u8* data = (u8*)pCurEntry->Action;
			*data = 1 - *data;
		}
		Menu_DisplayItem(g_MenuItem);
		ayFX_PlayBank(14, 0);
	}
	
	// Handle navigation
	if(Menu_PressUp())
	{
		u8 i = g_MenuItem;
		while(i > 0)
		{
			i--;
			if(g_CurrentMenu->Items[i].Type < MENU_ITEM_EMPTY)
			{
				u8 prev = g_MenuItem;
				g_MenuItem = i;
				Menu_DisplayItem(prev);
				Menu_DisplayItem(g_MenuItem);
				break;
			}
		}			
	}
	else if(Menu_PressDown())
	{
		u8 i = g_MenuItem;
		while(i < g_CurrentMenu->ItemNum-1)
		{
			i++;
			if(g_CurrentMenu->Items[i].Type < MENU_ITEM_EMPTY)
			{
				u8 prev = g_MenuItem;
				g_MenuItem = i;
				Menu_DisplayItem(prev);
				Menu_DisplayItem(g_MenuItem);
				break;
			}
		}
	}
	
	// Update menu items
	if((g_GameFrame & 0x07) == 0) // 8th frame
	{
		Print_SelectTextFont(g_DataFont, OFFSET_TITLE_FONT_DEF);
		for(u8 item = 0; item < g_CurrentMenu->ItemNum; ++item)
		{
			MenuItem* pCurEntry = &g_CurrentMenu->Items[item];
			if(pCurEntry->Type == MENU_ITEM_UPDATE)
			{
				Menu_ActionCB cb = (Menu_ActionCB)pCurEntry->Action;
				const c8* str = cb(MENU_ACTION_GET, pCurEntry->Value);
				u8 x = MENU_ITEMS_X;
				x += pCurEntry->Value;			
				Print_SetPosition(x, MENU_ITEMS_Y + item);
				Print_DrawText(str);
			}
		}
	}
}

#pragma restore		///< restore "pointer types incompatible" warning


//=============================================================================
//
//   S Y S T E M
//
//=============================================================================

//-----------------------------------------------------------------------------
///
void LoadMatchData()
{
	 // Clear sprite attribute table
	 VDP_FillVRAM_16K(0x00, g_SpriteAtributeLow, 32*4);
	 // Clear sprite patterns table
	 VDP_FillVRAM_16K(0x00, g_SpritePatternLow, 256*8);

	// Load ball sprites
	{
		const u8* src = g_DataBall;
		u16 dst = g_SpritePatternLow + (PATTERN_BALL_LINE1 * 8);
		for(i8 i = 0; i < 5; ++i)
		{
			VDP_WriteVRAM_16K(src, dst, 8);
			src += (1 * 8);
			dst += (4 * 8);
		}
	}
	
	// Load racket sprites
	{
		const u8* src = g_DataRacket;
		u16 dst	= g_SpritePatternLow + (PATTERN_RACKET_R * 8) + 8;
		for(u8 i = 0; i < 3; ++i)
		{
			VDP_WriteVRAM_16K(src, dst, 8);
			src += 8;
			dst += 8 * 4;
		}
	}
	
	// Generate cursor sprites
	{
		u16 dst = g_SpritePatternLow + (PATTERN_CURSOR1 * 8);
		for(u8 i = 0; i < 8 * 2; ++i)
		{
			VDP_FillVRAM_16K(0x55, dst + 4 * 8, 1);
			VDP_FillVRAM_16K(0xAA, dst++, 1);
			VDP_FillVRAM_16K(0xAA, dst + 4 * 8, 1);
			VDP_FillVRAM_16K(0x55, dst++, 1);
		}
	}

	// Load net sprites
	{
		u16 dst = g_SpritePatternLow + (PATTERN_NET1 * 8);
		VDP_WriteVRAM_16K(g_DataNet, dst, 8 * 4 * 5);
	}
	
	// Load score ball sprites
	{
		const u8* src = g_DataBall + 6 * 8;
		u16 dst = g_SpritePatternLow + (PATTERN_SERVER_BASE * 8);
		for(u8 i = 0; i < 4; ++i)
		{
			VDP_WriteVRAM_16K(src, dst, 8);
			src += 8;
			dst += 8 * 4;
		}
	}

	// Load cup sprites
	{
		const u8* src = g_DataCup;
		u16 dst = g_SpritePatternLow + (PATTERN_CUP_OUTLINE1 * 8);
		for(u8 i = 0; i < 6; ++i)
		{
			VDP_WriteVRAM_16K(src, dst, 8);
			src += 8;
			dst += 8 * 2;
		}
	}
}


//=============================================================================
//
//   B A L L
//
//=============================================================================

//-----------------------------------------------------------------------------
///
void SetNetSprite(u8 x) __FASTCALL
{
	x /= 8;
	for(u8 i = 0; i < 2; ++i)
	{
		if(g_CourtNet[x] == 0x80)
			x--;
		if(g_CourtNet[x] != 0xFF)
		{
			if(g_CourtNet[x] == 0)
				VDP_SetSprite(SPRITE_NET_LEFT+i, x * 8, 79, PATTERN_NET1 + g_CourtNet[x] * 4);
			else
			{
				VDP_SetSprite(SPRITE_NET_LEFT+i, x * 8, 79, PATTERN_NET1 + g_CourtNet[x] * 4);
				VDP_SetSprite(SPRITE_NET_GRAY, x * 8, 79, PATTERN_NET1 + 4 + g_CourtNet[x] * 4);
			}
			x += 2;
		}
		else
			x++;
	}
}


//-----------------------------------------------------------------------------
///
void SetTrainingCursor()
{
	u16 rnd = Math_GetRandom16();

	if(g_Level == LEVEL_EASY)
		g_TrainZone.x = (rnd % (128-16)) + 56;
	else
		g_TrainZone.x = (rnd % 128) + 56;

	if(g_Level == LEVEL_HARD)
		g_TrainZone.y = ((rnd >> 8) % 40) + 32;
	else if(g_Level == LEVEL_MEDIUM)
		g_TrainZone.y = ((rnd >> 8) % (40-16)) + 32;
	else
		g_TrainZone.y = ((rnd >> 8) % (40-32)) + 40;

	if(g_TrainSide == SIDE_TOP)
		g_TrainZone.y += 72;

	VDP_SetSpritePositionY(SPRITE_CURSOR_1, 213);
	VDP_SetSpritePositionY(SPRITE_CURSOR_2, 213);
	VDP_SetSpritePositionY(SPRITE_CURSOR_3, 213);
	VDP_SetSpritePositionY(SPRITE_CURSOR_4, 213);

	switch(g_Level)
	{
	case LEVEL_EASY:
		SetSprite(SPRITE_CURSOR_2, g_TrainZone.x + 16, g_TrainZone.y, PATTERN_CURSOR1, COLOR_DARK_BLUE);
		SetSprite(SPRITE_CURSOR_4, g_TrainZone.x + 16, g_TrainZone.y + 16, PATTERN_CURSOR1, COLOR_DARK_BLUE);

	case LEVEL_MEDIUM:
		SetSprite(SPRITE_CURSOR_3, g_TrainZone.x, g_TrainZone.y + 16, PATTERN_CURSOR1, COLOR_DARK_BLUE);

	case LEVEL_HARD:
		SetSprite(SPRITE_CURSOR_1, g_TrainZone.x, g_TrainZone.y, PATTERN_CURSOR1, COLOR_DARK_BLUE);
	}
	
	if(g_TrainSide == SIDE_BOTTOM)
	{
		VDP_SetSpritePositionY(SPRITE_NET_LEFT, 213);
		VDP_SetSpritePositionY(SPRITE_NET_RIGHT, 213);
		VDP_SetSpritePositionY(SPRITE_NET_GRAY, 213);
		SetNetSprite(g_TrainZone.x);
	}
}

void UpdateTrainingCursor()
{
	u8 pat = g_FlickerShadow ? (PATTERN_CURSOR1 + (g_GameFrame & 0x1) * 4) : PATTERN_CURSOR1;
	VDP_SetSpritePattern(SPRITE_CURSOR_1, pat);
	VDP_SetSpritePattern(SPRITE_CURSOR_2, pat);
	VDP_SetSpritePattern(SPRITE_CURSOR_3, pat);
	VDP_SetSpritePattern(SPRITE_CURSOR_4, pat);
}

//-----------------------------------------------------------------------------
///
void Ball_ShootRandom()
{
	// Position
	g_Ball.height = PX_TO_UNIT(10);
	g_Ball.pos.x = PX_TO_UNIT(g_TrainSideData[g_TrainSide].pos.x);
	g_Ball.pos.y = PX_TO_UNIT(g_TrainSideData[g_TrainSide].pos.y);
	g_Ball.srcPos.x = UNIT_TO_PX(g_Ball.pos.x);
	g_Ball.srcPos.y = UNIT_TO_PX(g_Ball.pos.y);

	u16 rnd = Math_GetRandom16();

	// Direction
	g_Ball.dir = g_TrainSide == SIDE_BOTTOM ? 48 : 16;
	switch(g_TrainDir)
	{
	case DIR_RANDOM:
		g_Ball.dir = rnd;
		g_Ball.dir %= 9;
		g_Ball.dir += g_TrainSideData[g_TrainSide].dir;
		break;
	case DIR_CENTER: break;
	case DIR_RIGHT:  g_Ball.dir += 4; break;
	case DIR_LEFT:   g_Ball.dir -= 4; break;
	}
		
	// Velocity
	if(((g_TrainShot == TRAIN_FLAT)) || ((g_TrainShot == TRAIN_BOTH) && (rnd & BIT_8))) // top spine
	{
		g_Ball.velXY = PX_TO_UNIT(3);
		g_Ball.velZ = PX_TO_UNIT(3);
		g_Ball.spin = SPIN_FLAT;
	}
	else // lob
	{
		g_Ball.velXY = PX_TO_UNIT(2);
		g_Ball.velZ = PX_TO_UNIT(5);
		g_Ball.spin = SPIN_FLAT;
	}

	// Misc
	g_Ball.lastPly = 1 - g_TrainSide;
	g_Ball.coolDown = 0;
	g_Ball.bounce = 0;
	g_Ball.point = POINT_PENDING;
	g_Ball.physics = TRUE;
	
	ayFX_PlayBank(1, 0);
}

//-----------------------------------------------------------------------------
///
void Ball_SwitchSprites()
{
	// Flicker Shadows
	u8 pat = PATTERN_BALL_LINE1;
	if(g_FlickerShadow && (g_GameFrame & 0x1))
	{
		pat += 4;
	}
	VDP_SetSpritePattern(SPRITE_BALL_OUTLINE, pat);
}

//-----------------------------------------------------------------------------
///
bool Ball_CheckField()
{
	const Field* field;
	if(g_Ball.lastPly == SIDE_BOTTOM)
		field = &g_Field[SIDE_BOTTOM];
	else
		field = &g_Field[SIDE_TOP];

	if(g_Ball.srcPos.x < field->min.x)
		return FALSE;
	if(g_Ball.srcPos.x > field->max.x)
		return FALSE;
	if(g_Ball.srcPos.y < field->min.y)
		return FALSE;
	if(g_Ball.srcPos.y > field->max.y)
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------------
///
void Ball_Hide()
{	
	VDP_SetSpritePositionY(SPRITE_BALL_SHADOW,  213);
	VDP_SetSpritePositionY(SPRITE_BALL_OUTLINE, 213);
	VDP_SetSpritePositionY(SPRITE_BALL_BODY,    213);
	VDP_SetSpritePositionY(SPRITE_BALL_MARK,    213);

	g_Ball.pos.y = PX_TO_UNIT(246);
	g_Ball.srcPos.y = UNIT_TO_PX(g_Ball.pos.y);
	g_Ball.velXY = 0;
	g_Ball.velZ = 0;
	g_Ball.physics = FALSE;
}

//-----------------------------------------------------------------------------
///
void Ball_Update()
{
	//-------------------------------------------------------------------------
	// Simulate ball physics
	if(g_Ball.physics)
	{
		// Horizontal force
		i16 dx = g_Ball.velXY * (i16)g_Cosinus64[g_Ball.dir];
		dx /= 64;
		g_Ball.pos.x += dx;
		
		i16 prevY = g_Ball.pos.y;
		i16 dy = g_Ball.velXY * (i16)g_Sinus64[g_Ball.dir];
		dy /= 64;
		g_Ball.pos.y -= dy;
		g_Ball.srcPos.x = UNIT_TO_PX(g_Ball.pos.x);
		g_Ball.srcPos.y = UNIT_TO_PX(g_Ball.pos.y);
		
		// Vertical force - Gravity
		g_Ball.velZ -= GRAVITY;	
		g_Ball.height += g_Ball.velZ;

		// Bounce
		if(g_Ball.height < 0)
		{
			g_Ball.height = 0;
			g_Ball.bounce++;
			g_Ball.velZ = -g_Ball.velZ * 2 / 3;

			/*if(g_Ball.spin == SPIN_TOP)
			{
			}
			else*/ if(g_Ball.spin == SPIN_BACK)
			{
				g_Ball.velXY = g_Ball.velXY / 2;
			}
			else if(g_Ball.spin == SPIN_FLAT)
			{
				g_Ball.velXY = g_Ball.velXY * 2 / 3;
			}

			if(g_Ball.bounce == 1)
			{
				if(Ball_CheckField() && (g_Ball.point == POINT_PENDING))
				{
					g_Ball.point = POINT_VALIDATED;
					g_ScoreFct(EVENT_IN);
					ayFX_PlayBank(12, 0);
				}
				else
				{
					g_ScoreFct(EVENT_OUT);
					ayFX_PlayBank(2, 0);
				}
			}
			else if(g_Ball.bounce == 2)
			{
				g_ScoreFct(EVENT_INOUT);
				ayFX_PlayBank(12, 0);
			}
		}

		if(g_Ball.coolDown > 0)
			g_Ball.coolDown--;

		// Check out of screen
		if((g_Ball.pos.y < 0) || (g_Ball.pos.y > PX_TO_UNIT(192)) || (g_Ball.pos.x < 0) || (g_Ball.pos.x > PX_TO_UNIT(256)))
		{
			Ball_Hide();
			g_ScoreFct((g_Ball.point == POINT_PENDING) ? EVENT_OUT : EVENT_INOUT);
		}
		// Check net collision
		else if(g_Ball.height < PX_TO_UNIT(14))
		{
			if(((prevY < PX_TO_UNIT(96)) && (g_Ball.pos.y >= PX_TO_UNIT(96))) || (prevY > PX_TO_UNIT(96)) && (g_Ball.pos.y <= PX_TO_UNIT(96)))
			{
				g_Ball.velXY = 0;
				g_Ball.velZ = 0;
				g_ScoreFct(EVENT_NET);
			}
		}
	}
	
	//-------------------------------------------------------------------------
	// Display ball

	// Update sprites position
	u8  x = g_Ball.srcPos.x - 3;
	i16 y = g_Ball.srcPos.y - 3;

	// Ball shadow
	if((y >= 82) && (y < 96))
		VDP_SetSpritePositionY(SPRITE_BALL_SHADOW, 213);
	else
		VDP_SetSpritePosition(SPRITE_BALL_SHADOW,  x, y);

	// Ball body
	y -= UNIT_TO_PX(g_Ball.height);
	if(y < 0)
	{
		VDP_SetSpritePositionY(SPRITE_BALL_OUTLINE, 213);
		VDP_SetSpritePositionY(SPRITE_BALL_BODY,    213);
	}
	else
	{
		VDP_SetSpritePosition(SPRITE_BALL_OUTLINE, x, y);
		VDP_SetSpritePosition(SPRITE_BALL_BODY,    x, y);
	}
}

//=============================================================================
//
//   P L A Y E R S
//
//=============================================================================

//-----------------------------------------------------------------------------
///
void ThrowService(Player* ply)
{
	// Position
	g_Ball.height = PX_TO_UNIT(28);
	g_Ball.pos = ply->pos;
	g_Ball.srcPos.x = UNIT_TO_PX(g_Ball.pos.x);
	g_Ball.srcPos.y = UNIT_TO_PX(g_Ball.pos.y);

	g_ServeDX /= 16;
	g_ServeDY /= 16;

// Print_SetPosition(0, 1);
// Print_DrawInt(g_ServeDX);
// Print_SetPosition(0, 2);
// Print_DrawInt(g_ServeDY);

	// Setup direction
	switch(g_ServeZone)
	{
	case SERVE_BOT_R: g_Ball.dir = 16+7; break;
	case SERVE_BOT_L: g_Ball.dir = 16-7; break;
	case SERVE_TOP_L: g_Ball.dir = 48+7; break;
	case SERVE_TOP_R: g_Ball.dir = 48-7; break;
	};
	g_Ball.dir -= g_ServeDX;

	// Setup velocity	
	g_Ball.velXY = PX_TO_UNIT(3.3);
	g_Ball.velZ = PX_TO_UNIT(2.5);
	g_Ball.spin = SPIN_FLAT;

	// g_Ball.velZ -= g_ServeDX + PX_TO_UNIT(0.1);

	// g_Ball.velXY += g_ServeDY + PX_TO_UNIT(0.1);
	// g_Ball.velZ -= g_ServeDY + PX_TO_UNIT(0.1);

	// if(g_ServeDY < 0)
	// {
		// g_Ball.velXY = PX_TO_UNIT(3.3);
		// g_Ball.velZ = PX_TO_UNIT(2.5);
	// }
	// else if(g_ServeDY > 0)
	// {
		// g_Ball.velXY = PX_TO_UNIT(3.3);
		// g_Ball.velZ = PX_TO_UNIT(2.5);
	// }


	// if(g_ServeDY < -32)
		// g_Ball.velXY = PX_TO_UNIT(2.8);
	// else if(g_ServeDY < -16)
		// g_Ball.velXY = PX_TO_UNIT(3.0);
	// else if(g_ServeDY < -8)
		// g_Ball.velXY = PX_TO_UNIT(3.1);
	// else if(g_ServeDY > 8)
		// g_Ball.velXY = PX_TO_UNIT(3.8);
	// else if(g_ServeDY > 16)
		// g_Ball.velXY = PX_TO_UNIT(4.6);
	// else if(g_ServeDY > 32)
		// g_Ball.velXY = PX_TO_UNIT(5.5);

	// Misc
	g_Ball.lastPly = ply->side;
	g_Ball.coolDown = 0;
	g_Ball.bounce = 0;
	g_Ball.point = POINT_PENDING;
	g_Ball.physics = TRUE;
		
	ayFX_PlayBank(1, 0);
}

//-----------------------------------------------------------------------------
///
void Player_CheckShoot(Player* ply)
{
	if(g_Ball.coolDown > 0)
		return;
		
	u8 event = g_Actions[ply->action].animFrames[ply->step].event;

	// Check Z coordinate
	if(event == FUNCT_SMASH)
	{
		if(g_Ball.height > PX_TO_UNIT(40))
			return;
	}	
	
	// Check X coordinate
	u8 minX, maxX;
	switch(event)
	{
	case FUNCT_SHOOT_R:
		minX = UNIT_TO_PX(ply->pos.x);
		maxX = minX + SHOT_DETECT_X;
		break;
	case FUNCT_SHOOT_L:
		maxX = UNIT_TO_PX(ply->pos.x);
		minX = maxX - SHOT_DETECT_X;
		break;
	case FUNCT_SMASH:
		minX = UNIT_TO_PX(ply->pos.x) - SHOT_DETECT_SMASH;
		maxX = minX + (2 * SHOT_DETECT_SMASH);
		break;
	default:
		return;
	};
	
	u8 ballX = g_Ball.srcPos.x;
	if((ballX < minX) || (ballX > maxX))
		return;

	// Check Y coordinate
	u8 minY = ply->srcPos.y - SHOT_DETECT_Y;
	u8 maxY = ply->srcPos.y + SHOT_DETECT_Y;
	
	u8 ballY = g_Ball.srcPos.y;
	if((ballY < minY) || (ballY > maxY))
		return;
		
	// Shoot Succed !!!
	
	ayFX_PlayBank(1, 0);

	// Handle shot direction
	ply->shotCnt = ply->counter;
	//ply->shotCnt -= 4;
	// if(ply->shotCnt > 6)
		// ply->shotCnt = 6;

	const Binding* bind = ply->binding;
	u8 dir = 16;

	// Set shot direction
	if(ply->side == SIDE_BOTTOM)
	{
		if(event == FUNCT_SHOOT_R)
			dir += 2;
		else if(event == FUNCT_SHOOT_L)
			dir -= 2;

		if(bind->inHoldLeft())
			dir += ply->shotCnt;
		else if(bind->inHoldRight())
			dir -= ply->shotCnt;
	}
	else // if(ply->side == SIDE_TOP)
	{
		dir += 32;
	
		if(event == FUNCT_SHOOT_R)
			dir -= 2;
		else if(event == FUNCT_SHOOT_L)
			dir += 2;

		if(bind->inHoldLeft())
			dir -= ply->shotCnt;
		else if(bind->inHoldRight())
			dir += ply->shotCnt;			
	}
	g_Ball.dir = dir;
	
	// Set shot strength
	u8 shotId = ply->shot * 4; // Long shot
	if(event == FUNCT_SMASH)
	{
		shotId += 3;
	}
	else if(ply->inShort()) // Short shot
	{
		shotId += 2;
	}
	else if(!ply->inLong()) // Med shot
	{
		shotId++;
	}
	
	const Shot* shot = &g_Shots[shotId];

	g_Ball.velXY = shot->velXY;
	g_Ball.velZ  = shot->velZ;
	g_Ball.spin  = shot->spin;

	// Misc
	g_Ball.lastPly = ply->side;
	g_Ball.coolDown = 10;
	g_Ball.bounce = 0;
	g_Ball.point = POINT_PENDING;
	//g_Ball.physics = TRUE;
}




//-----------------------------------------------------------------------------
// GENERIC

/// Set sprite attribute for Sprite Mode 2 and fill color table with unique color
void SetSprite(u8 index, u8 x, u8 y, u8 shape, u8 color)
{
	// Initialize sprite attributes
	#if (MSX2_ENHANCE)
		if(g_VersionVDP == VDP_VERSION_TMS9918A)
			VDP_SetSpriteSM1(index, x, y, shape, color);
		else
			VDP_SetSpriteExUniColor(index, x, y, shape, color);
	#else
		VDP_SetSpriteSM1(index, x, y, shape, color);
	#endif
}


//-----------------------------------------------------------------------------
///
void SetAction(Player* ply, u8 id)
{
	if(ply->action != id)
	{
		ply->action = id;
		ply->counter = 0;
		ply->step = 0;
	}
}

//-----------------------------------------------------------------------------
///
void Player_UpdateAction(Player* ply) __FASTCALL
{
	const Action* act = &g_Actions[ply->action];

	// Finished current animation step
	if(ply->counter >= act->animFrames[ply->step].len)
	{
		ply->counter = 0;
		ply->step++;
	}

	// Finished last animation step
	if(ply->step >= act->animLen)
	{
		if(act->loop) // restart action
		{
			ply->counter = 0;
			ply->step = 0;
		}
		else // stop action and transit to IDLE
		{
			SetAction(ply, ACTION_IDLE);
			Player_UpdateAction(ply);
			return;
		}
	}
	
	// Update animation
	ply->anim = act->animFrames[ply->step].frame;
	ply->counter++;

	// Handle current action's event
	const Binding* bind = ply->binding;
	switch(act->animFrames[ply->step].event)
	{
	case FUNCT_THROW:
		if(bind->inHoldRight())
			g_ServeDX++;
		else if(bind->inHoldLeft())
			g_ServeDX--;			
		if(ply->inLong())
			g_ServeDY++;
		else if(ply->inShort())
			g_ServeDY--;
		if(bind->inButton1() || bind->inButton2())
		{
			SetAction(ply, ACTION_SERVE);
			ThrowService(ply);
			g_ServeState = FALSE;
		}
		break;
	
	case FUNCT_PREPARE:
	case FUNCT_SHOOT_R:
	case FUNCT_SHOOT_L:
	case FUNCT_SMASH:
		if(bind->inButton1() && (ply->shot == SHOT_SLICE))
			ply->shot = SHOT_ATTACK;
		if(bind->inButton2() && (ply->shot == SHOT_FLAT))
			ply->shot = SHOT_ATTACK;
		break;

	case FUNCT_CUP:
		VDP_SetSpritePositionY(SPRITE_CUP_OUTLINE, 164-33 - 22 + ply->anim);
		VDP_SetSpritePositionY(SPRITE_CUP_BODY,    164-33 - 22 + ply->anim);

	default:
		break;
	};
}

//-----------------------------------------------------------------------------
///
void Player_HandleService(Player* ply) __FASTCALL
{
	const Binding* bind = ply->binding;
	if(g_Actions[ply->action].interrupt)
	{
		if(bind->inButton1() || bind->inButton2())
		{
			g_ServeDX = 0;
			g_ServeDY = 0;
			SetAction(ply, ACTION_THROW);
		}
		else
		{
			u8 dir = 4;
			if(bind->inHoldLeft())
				dir -= 3;
			else if(bind->inHoldRight())
				dir += 3;	
				
			if(dir == 4) // No move
			{
				SetAction(ply, ACTION_IDLE);
			}
			else
			{
				if(dir > 3)
					dir--;

				// Update player position
				ply->pos.x += g_Move[dir].x;

				const ServeInfo* info = &g_ServeInfo[g_ServeZone];

				// Validate new X position
				if(ply->pos.x < info->minX)
					ply->pos.x = info->minX;
				else if(ply->pos.x > info->maxX)
					ply->pos.x = info->maxX;
				
				// Precompute screen position
				ply->srcPos.x = UNIT_TO_PX(ply->pos.x);
				
				// Set movement action
				if(dir < 3) // Left move
					SetAction(ply, ACTION_WALK_L);
				else // Right move
					SetAction(ply, ACTION_WALK_R);				
			}
		}
	}
}


//-----------------------------------------------------------------------------
///
void Player_HandleInput(Player* ply) __FASTCALL
{
	const Binding* bind = ply->binding;
	if(g_Actions[ply->action].interrupt)
	{
		if(bind->inButton1() || bind->inButton2()) // Button triggered
		{
			if(g_Ball.height > PX_TO_UNIT(32))
				SetAction(ply, ACTION_SMASH);
			else
			{
				// Project the movement vector of the ball on the horizontal line of the player (to know if it will arrive on his right or on his left) 
				i16 dY;
				if(ply->side == SIDE_BOTTOM)
					dY = ply->srcPos.y - g_Ball.srcPos.y;
				else // if(ply->side == SIDE_TOP)
					dY = g_Ball.srcPos.y - ply->srcPos.y;
				i16 dX = dY * (i16)g_Cosinus64[g_Ball.dir]; // Q16.0 x Q2.6 => Q10.6
				dX /= 64; // Q10.6  => Q16.0
				if((g_Ball.srcPos.x + dX) > ply->srcPos.x)
					SetAction(ply, ACTION_SHOOT_R);
				else
					SetAction(ply, ACTION_SHOOT_L);
			}
			if(bind->inButton1())
				ply->shot = SHOT_FLAT;
			else // if(bind->inButton2())
				ply->shot = SHOT_SLICE;
		}
		else
		{
			u8 dir = 4;
			if(bind->inHoldUp())
				dir--;
			else if(bind->inHoldDown())
				dir++;
			if(bind->inHoldLeft())
				dir -= 3;
			else if(bind->inHoldRight())
				dir += 3;	
				
			if(dir == 4) // No move
			{
				SetAction(ply, ACTION_IDLE);
			}
			else
			{
				if(dir > 3)
					dir--;

				// Update player position
				ply->pos.x += g_Move[dir].x;
				ply->pos.y += g_Move[dir].y;
				
				// Validate new X position
				if(ply->pos.x < PLY_MIN_X)
					ply->pos.x = PLY_MIN_X;
				else if(ply->pos.x > PLY_MAX_X)
					ply->pos.x = PLY_MAX_X;
				
				// Validate new Y position
				if(ply->side == SIDE_BOTTOM)
				{
					if(ply->pos.y < PLY_DOWN_MIN_X)
						ply->pos.y = PLY_DOWN_MIN_X;
					else if(ply->pos.y > PLY_DOWN_MAX_X)
						ply->pos.y = PLY_DOWN_MAX_X;
				}
				else
				{
					if(ply->pos.y < PLY_UP_MIN_X)
						ply->pos.y = PLY_UP_MIN_X;
					else if(ply->pos.y > PLY_UP_MAX_X)
						ply->pos.y = PLY_UP_MAX_X;
				}

				// Precompute screen position
				ply->srcPos.x = UNIT_TO_PX(ply->pos.x);
				ply->srcPos.y = UNIT_TO_PX(ply->pos.y);
				
				// Set movement action
				if((dir == 3) || (dir == 4)) // Front move
					SetAction(ply, ACTION_WALK_F);
				else if(dir < 3) // Left move
					SetAction(ply, ACTION_WALK_L);
				else // Right move
					SetAction(ply, ACTION_WALK_R);				
			}
		}
	}
}

//-----------------------------------------------------------------------------
///
void UpdatePlayerBottom()
{	
	// Update sprite position
	u8 x = g_Player[SIDE_BOTTOM].srcPos.x - 8;
	u8 y = g_Player[SIDE_BOTTOM].srcPos.y - 24;
	VDP_SetSpritePosition(SPRITE_PLY_BOT_BLACK_H, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_BOT_WHITE_H, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_BOT_SKIN_H, x, y);
	y += 8;
	VDP_SetSpritePosition(SPRITE_PLY_BOT_CLOTH, x, y);
	y += 8;
	VDP_SetSpritePosition(SPRITE_PLY_BOT_BLACK_L, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_BOT_WHITE_L, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_BOT_SKIN_L, x, y);

	// Handle special event
	u8 event = g_Actions[g_Player[SIDE_BOTTOM].action].animFrames[g_Player[SIDE_BOTTOM].step].event;
	switch(event)
	{
	case FUNCT_SHOOT_R:
		Player_CheckShoot(&g_Player[SIDE_BOTTOM]);
		VDP_SetSprite(SPRITE_PLY_BOT_RACKET, x+16, y-16, PATTERN_RACKET_R);
		break;
	case FUNCT_SHOOT_L:
		Player_CheckShoot(&g_Player[SIDE_BOTTOM]);
		VDP_SetSprite(SPRITE_PLY_BOT_RACKET, x-8, y-16, PATTERN_RACKET_L);
		break;
	case FUNCT_SMASH:
		Player_CheckShoot(&g_Player[SIDE_BOTTOM]);
	case FUNCT_SERVE:
		VDP_SetSprite(SPRITE_PLY_BOT_RACKET, x+8, y-32, PATTERN_RACKET_T);
		break;
	default:
		VDP_SetSpritePositionY(SPRITE_PLY_BOT_RACKET, 213);
		break;
	};
}

//-----------------------------------------------------------------------------
///
void DrawPlayerBottom()
{
	u8 frame = g_Player[SIDE_BOTTOM].anim;
	if(g_Player[SIDE_BOTTOM].prevAnim == frame)
		return;

	g_Player[SIDE_BOTTOM].prevAnim = frame;

	const u8* src;
	u16 dst;

	src = g_DataPlayer1 + (frame * 224);
	dst = g_SpritePatternLow + (PATTERN_PLY_BOT_LINE1_H * 8); // Pattern #0 - 20
	VDP_WriteVRAM_16K(src, dst, 21 * 8);
	
	src += 21 * 8;
	dst += (22 * 8); // Pattern #22
	VDP_WriteVRAM_16K(src, dst, 8);
	
	src += 8;
	dst += (2 * 8); // Pattern #24
	VDP_WriteVRAM_16K(src, dst, 8);
	
	src += 8;
	dst += (2 * 8); // Pattern #26
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #28
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #30
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #32
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #34
	VDP_WriteVRAM_16K(src, dst, 8);
}

//-----------------------------------------------------------------------------
///
void Player_SwitchSprites(Player* ply) __FASTCALL
{
	// Flicker Shadows
	u8 pat = 0;
	if(g_FlickerShadow && (g_GameFrame & 0x1))
	{
		pat += 4;
	}
	if(ply->side == SIDE_BOTTOM)
	{
		VDP_SetSpritePattern(SPRITE_PLY_BOT_BLACK_H, pat + PATTERN_PLY_BOT_LINE1_H);      // Pattern #0  / #4
		VDP_SetSpritePattern(SPRITE_PLY_BOT_BLACK_L, pat + PATTERN_PLY_BOT_LINE1_H + 20); // Pattern #20 / #24
	}
	else
	{
		VDP_SetSpritePattern(SPRITE_PLY_TOP_BLACK_H, pat + PATTERN_PLY_TOP_LINE1_H);      // Pattern #40 / #44
		VDP_SetSpritePattern(SPRITE_PLY_TOP_BLACK_L, pat + PATTERN_PLY_TOP_LINE1_H + 16); // Pattern #56 / #60
	}
}

//-----------------------------------------------------------------------------
///
void UpdatePlayerTop()
{	
	// Update sprite position
	u8 x = g_Player[SIDE_TOP].srcPos.x - 8;
	u8 y = g_Player[SIDE_TOP].srcPos.y - 32;
	VDP_SetSpritePosition(SPRITE_PLY_TOP_BLACK_H, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_TOP_WHITE_H, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_TOP_SKIN_H, x, y);
	y += 8;
	VDP_SetSpritePosition(SPRITE_PLY_TOP_CLOTH, x, y);
	y += 8;
	VDP_SetSpritePosition(SPRITE_PLY_TOP_BLACK_L, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_TOP_WHITE_L, x, y);
	VDP_SetSpritePosition(SPRITE_PLY_TOP_SKIN_L, x, y);

	// Handle special event
	u8 event = g_Actions[g_Player[SIDE_TOP].action].animFrames[g_Player[SIDE_TOP].step].event;
	switch(event)
	{
	case FUNCT_SHOOT_R:
		Player_CheckShoot(&g_Player[SIDE_TOP]);
		VDP_SetSprite(SPRITE_PLY_TOP_RACKET, x+16, y-8, PATTERN_RACKET_R);
		break;                                    
	case FUNCT_SHOOT_L:                           
		Player_CheckShoot(&g_Player[SIDE_TOP]);
		VDP_SetSprite(SPRITE_PLY_TOP_RACKET, x-8, y-8, PATTERN_RACKET_L);
		break;                                    
	case FUNCT_SMASH:
		Player_CheckShoot(&g_Player[SIDE_TOP]);
	case FUNCT_SERVE:
		VDP_SetSprite(SPRITE_PLY_TOP_RACKET, x+0, y-24, PATTERN_RACKET_T);
		break;                                    
	default:                                      
		VDP_SetSpritePositionY(SPRITE_PLY_TOP_RACKET, 213);
		break;
	};
	
	// Handle net
	if(g_DisplayNet)
	{
		VDP_SetSpritePositionY(SPRITE_NET_LEFT, 213);
		VDP_SetSpritePositionY(SPRITE_NET_RIGHT, 213);
		VDP_SetSpritePositionY(SPRITE_NET_GRAY, 213);
		if(g_Player[SIDE_TOP].srcPos.y >= 80)
			SetNetSprite(x);
	}
}

//-----------------------------------------------------------------------------
///
void DrawPlayerTop() __FASTCALL
{
	u8 frame = g_Player[SIDE_TOP].anim;
	if(g_Player[SIDE_TOP].prevAnim == frame)
		return;

	g_Player[SIDE_TOP].prevAnim = frame;

	const u8* src;
	u16 dst;
		
	src = g_DataPlayer2 + (frame * 224);
	dst = g_SpritePatternLow + ((PATTERN_PLY_TOP_LINE1_H+1) * 8); // Pattern #41
	VDP_WriteVRAM_16K(src, dst, 8);
	
	src += 8;
	dst += (2 * 8); // Pattern #43
	VDP_WriteVRAM_16K(src, dst, 8);
	
	src += 8;
	dst += (2 * 8); // Pattern #45
	VDP_WriteVRAM_16K(src, dst, 8);
	
	src += 8;
	dst += (2 * 8); // Pattern #47
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #49
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #51
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #53
	VDP_WriteVRAM_16K(src, dst, 8);

	src += 8;
	dst += (2 * 8);  // Pattern #55 - #75
	VDP_WriteVRAM_16K(src, dst, 21 * 8);
}

//-----------------------------------------------------------------------------
///
void Launcher_SwitchSprites()
{
	// Flicker Shadows
	u8 pat = g_TrainSideData[g_TrainSide].pattern;
	if(g_FlickerShadow && (g_GameFrame & 0x1))
	{
		pat += 8;
	}
	if(g_TrainSide == SIDE_BOTTOM)
	{
		VDP_SetSpritePattern(SPRITE_LAUNCHER_TOP_1, pat);     // Pattern #40 / #48
		VDP_SetSpritePattern(SPRITE_LAUNCHER_TOP_2, pat + 4); // Pattern #44 / #52
	}
	else
	{
		VDP_SetSpritePattern(SPRITE_LAUNCHER_BOT_1, pat);     // Pattern #40 / #48
		VDP_SetSpritePattern(SPRITE_LAUNCHER_BOT_2, pat + 4); // Pattern #44 / #52
	}
}

//-----------------------------------------------------------------------------
///
void UnpackGM2ToVRAM(const u8* src, u16 dst)
{
	while(*src != 0)
	{
		u8 type = *src >> 6;
		u8 len = *src & 0x3F;
		src++;
		if(type == 0) // Chunk of zeros
		{
			VDP_FillVRAM_16K(0x00, dst + 0x0000, len);
			VDP_FillVRAM_16K(0x00, dst + 0x0800, len);
			VDP_FillVRAM_16K(0x00, dst + 0x1000, len);
		}
		else if(type == 1) // Chunk of same byte
		{
			VDP_FillVRAM_16K(*src, dst + 0x0000, len);
			VDP_FillVRAM_16K(*src, dst + 0x0800, len);
			VDP_FillVRAM_16K(*src, dst + 0x1000, len);
			src++;
		}
		else // type == 3 // Chcunk of uncompressed data
		{
			VDP_WriteVRAM_16K(src, dst + 0x0000, len);
			VDP_WriteVRAM_16K(src, dst + 0x0800, len);
			VDP_WriteVRAM_16K(src, dst + 0x1000, len);
			src += len;
		}
		dst += len;
	}
}

//-----------------------------------------------------------------------------
///
void UnpackSpriteToVRAM(const u8* src, u16 dst)
{
	while(*src != 0)
	{
		u8 type = *src >> 6;
		u8 len = *src & 0x3F;
		src++;
		if(type == 0) // Chunk of zeros
		{
			VDP_FillVRAM_16K(0x00, dst, len);
		}
		else if(type == 1) // Chunk of same byte
		{
			VDP_FillVRAM_16K(*src, dst, len);
			src++;
		}
		else // type == 3 // Chcunk of uncompressed data
		{
			VDP_WriteVRAM_16K(src, dst, len);
			src += len;
		}
		dst += len;
	}
}

//-----------------------------------------------------------------------------
///
#if (COMPRESS_NAMES)
void UnpackRLEpToRAM(const u8* src, u8* dst)
{
	while(*src != 0)
	{
		u8 type = *src >> 6;
		u8 len = *src & 0x3F;
		src++;
		if(type == 0) // Chunk of zeros
		{
			Mem_Set(0x00, dst + 0x0000, len);
			Mem_Set(0x00, dst + 0x0800, len);
			Mem_Set(0x00, dst + 0x1000, len);
		}
		else if(type == 1) // Chunk of same byte
		{
			Mem_Set(*src, dst + 0x0000, len);
			Mem_Set(*src, dst + 0x0800, len);
			Mem_Set(*src, dst + 0x1000, len);
			src++;
		}
		else // type == 3 // Chcunk of uncompressed data
		{
			Mem_Copy(src, dst + 0x0000, len);
			Mem_Copy(src, dst + 0x0800, len);
			Mem_Copy(src, dst + 0x1000, len);
			src += len;
		}
		dst += len;
	}
}
#endif

//-----------------------------------------------------------------------------
///
void InitializeCourt()
{
	// Load court data to VRAM
	VDP_FillScreen_GM2(0);
	GM2_LAYOUT(g_DataCourt_Names, 3, 3, 27, 18);
	UnpackGM2ToVRAM(g_DataCourt_Patterns, g_ScreenPatternLow + (OFFSET_GAME_COURT * 8));
	UnpackGM2ToVRAM(g_DataCourt_Colors,   g_ScreenColorLow   + (OFFSET_GAME_COURT * 8));

	// Load Scrore Board data to VRAM
	UnpackGM2ToVRAM(g_DataScore_Patterns, g_ScreenPatternLow + (OFFSET_GAME_SCORE * 8));
	UnpackGM2ToVRAM(g_DataScore_Colors,   g_ScreenColorLow   + (OFFSET_GAME_SCORE * 8));

	// Load Referee data to VRAM
	UnpackGM2ToVRAM(g_DataReferee_Patterns, g_ScreenPatternLow + (OFFSET_GAME_REFEREE * 8));
	UnpackGM2ToVRAM(g_DataReferee_Colors,   g_ScreenColorLow   + (OFFSET_GAME_REFEREE * 8));

	// Initialize font
	Print_SetTextFont(g_DataFont, OFFSET_GAME_FONT);
	Print_SetColor(COLOR_WHITE, 0x9);
	
	Print_SetTextFont(g_DataSrcFont, OFFSET_GAME_SCRFONT);
	Print_SetColor(COLOR_WHITE, COLOR_BLACK);

	// Load static data to VRAM
	LoadMatchData();

	// Setup ball sprites
	SetSprite(SPRITE_BALL_OUTLINE, 0, 213, PATTERN_BALL_LINE1, COLOR_BLACK);		// Outline
	SetSprite(SPRITE_BALL_BODY,    0, 213, PATTERN_BALL_BODY,  COLOR_LIGHT_YELLOW);	// Body
	SetSprite(SPRITE_BALL_SHADOW,  0, 213, PATTERN_BALL_SHADE, COLOR_BLACK);		// Shadow
	SetSprite(SPRITE_BALL_MARK,    0, 213, PATTERN_BALL_MARK,  COLOR_MEDIUM_RED);	// Mark

	// Setup net sprites	
	SetSprite(SPRITE_NET_LEFT,  0, 213, PATTERN_NET1, COLOR_WHITE); 	// Net part 1
	SetSprite(SPRITE_NET_RIGHT, 0, 213, PATTERN_NET1, COLOR_WHITE); 	// Net part 2
	SetSprite(SPRITE_NET_GRAY,  0, 213, PATTERN_NET5, COLOR_GRAY);  	// Net gray part

	VDP_HideSpriteFrom(SPRITE_SCORE_BALL_1);

	g_DisplayNet = TRUE;

	PT3_Pause();

	VDP_SetColor(COLOR_DARK_RED);
}

//-----------------------------------------------------------------------------
///
void Player_Initialize(u8 side, u8 controller)
{
	// Setup player sprites
	if(side == SIDE_BOTTOM)
	{
		SetSprite(SPRITE_PLY_BOT_BLACK_H, 0, 213, PATTERN_PLY_BOT_LINE1_H, COLOR_BLACK);		// Outline
		SetSprite(SPRITE_PLY_BOT_BLACK_L, 0, 213, PATTERN_PLY_BOT_LINE1_L, COLOR_BLACK);		// Outline
		SetSprite(SPRITE_PLY_BOT_CLOTH,   0, 213, PATTERN_PLY_BOT_CLOTH,   COLOR_LIGHT_BLUE);	// Cloth
		SetSprite(SPRITE_PLY_BOT_WHITE_H, 0, 213, PATTERN_PLY_BOT_WHITE_H, COLOR_WHITE);		// White
		SetSprite(SPRITE_PLY_BOT_WHITE_L, 0, 213, PATTERN_PLY_BOT_WHITE_L, COLOR_WHITE);		// White
		SetSprite(SPRITE_PLY_BOT_SKIN_H,  0, 213, PATTERN_PLY_BOT_SKIN_H,  COLOR_LIGHT_RED);	// Skin
		SetSprite(SPRITE_PLY_BOT_SKIN_L,  0, 213, PATTERN_PLY_BOT_SKIN_L,  COLOR_LIGHT_RED);	// Skin
		SetSprite(SPRITE_PLY_BOT_RACKET,  0, 213, PATTERN_RACKET_R,        COLOR_BLACK);		// Racket
	}
	else // if(side == SIDE_TOP)
	{
		SetSprite(SPRITE_PLY_TOP_BLACK_H, 0, 213, PATTERN_PLY_TOP_LINE1_H, COLOR_BLACK);			// Outline
		SetSprite(SPRITE_PLY_TOP_BLACK_L, 0, 213, PATTERN_PLY_TOP_LINE1_L, COLOR_BLACK);			// Outline
		SetSprite(SPRITE_PLY_TOP_CLOTH,   0, 213, PATTERN_PLY_TOP_CLOTH, COLOR_MEDIUM_GREEN);	// Cloth
		SetSprite(SPRITE_PLY_TOP_WHITE_H, 0, 213, PATTERN_PLY_TOP_WHITE_H, COLOR_WHITE);			// White
		SetSprite(SPRITE_PLY_TOP_WHITE_L, 0, 213, PATTERN_PLY_TOP_WHITE_L, COLOR_WHITE);			// White
		SetSprite(SPRITE_PLY_TOP_SKIN_H,  0, 213, PATTERN_PLY_TOP_SKIN_H, COLOR_LIGHT_RED);		// Skin
		SetSprite(SPRITE_PLY_TOP_SKIN_L,  0, 213, PATTERN_PLY_TOP_SKIN_L, COLOR_LIGHT_RED);		// Skin
		SetSprite(SPRITE_PLY_TOP_RACKET,  0, 213, PATTERN_RACKET_R, COLOR_BLACK);			// Racket
	}
	
	// Initialize player
	Player* ply = &g_Player[side];
	ply->side = side;
	ply->control = controller;
	ply->action = ACTION_IDLE;
	ply->counter = 0;
	ply->anim = 0;
	ply->prevAnim = 0xFF;
	ply->step = 0;
	ply->binding = &g_Binding[g_InputBinding[controller]];
	const Binding* bind = ply->binding;
	if(side == SIDE_BOTTOM)
	{
		ply->inLong  = bind->inHoldUp;
		ply->inShort = bind->inHoldDown;
	}
	else // if(side == SIDE_TOP)
	{
		ply->inLong  = bind->inHoldDown;
		ply->inShort = bind->inHoldUp;
	}
	
	u8 zone = g_ServeZone;
	if(side != g_Server)
	{
		zone += 2;
		zone %= 4;
	}
	
	ply->srcPos.x = g_ServeInfo[zone].startPos.x;
	ply->srcPos.y = g_ServeInfo[zone].startPos.y;
	ply->pos.x = PX_TO_UNIT(ply->srcPos.x);
	ply->pos.y = PX_TO_UNIT(ply->srcPos.y);
}

//-----------------------------------------------------------------------------
///
void Launcher_Initialize()
{
	if(g_TrainSide == SIDE_BOTTOM)
	{
		// Load launcher pattern
		u16 dst;
		dst	= g_SpritePatternLow + (PATTERN_PLY_TOP_LINE1_H * 8); // Pattern #40 - #71
		UnpackSpriteToVRAM(g_DataLauncher0, dst);

		// Setup launcher sprites
		SetSprite(SPRITE_LAUNCHER_TOP_1, 128-8, (u8)(-4 +  0), PATTERN_PLY_TOP_LINE1_H +  0, COLOR_BLACK);			// Black 1&2
		SetSprite(SPRITE_LAUNCHER_TOP_2, 128-8, (u8)(-4 + 16), PATTERN_PLY_TOP_LINE1_H +  4, COLOR_BLACK);			// Black 1&2
		SetSprite(SPRITE_LAUNCHER_TOP_3, 128-8, (u8)(-4 +  0), PATTERN_PLY_TOP_LINE1_H + 16, COLOR_LIGHT_YELLOW);	// Yellow
		SetSprite(SPRITE_LAUNCHER_TOP_4, 128-8, (u8)(-4 +  8), PATTERN_PLY_TOP_LINE1_H + 20, COLOR_MEDIUM_GREEN);	// Green
		SetSprite(SPRITE_LAUNCHER_TOP_5, 128-8, (u8)(-4 + 16), PATTERN_PLY_TOP_LINE1_H + 24, COLOR_WHITE);			// White
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_TOP_6, 213);
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_TOP_7, 213);
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_TOP_8, 213);
	}
	else // if(g_TrainSide == SIDE_TOP)
	{
		// Load launcher pattern
		u16 dst;
		dst	= g_SpritePatternLow + (PATTERN_PLY_BOT_LINE1_H * 8); // Pattern #40 - #71
		UnpackSpriteToVRAM(g_DataLauncher1, dst);

		// Setup launcher sprites
		SetSprite(SPRITE_LAUNCHER_BOT_1, 128-8, (u8)(150 +  0), PATTERN_PLY_BOT_LINE1_H +  0, COLOR_BLACK);			// Black 1&2
		SetSprite(SPRITE_LAUNCHER_BOT_2, 128-8, (u8)(150 + 16), PATTERN_PLY_BOT_LINE1_H +  4, COLOR_BLACK);			// Black 1&2
		SetSprite(SPRITE_LAUNCHER_BOT_3, 128-8, (u8)(150 -  1), PATTERN_PLY_BOT_LINE1_H + 16, COLOR_LIGHT_YELLOW);	// Yellow
		SetSprite(SPRITE_LAUNCHER_BOT_4, 128-8, (u8)(150 +  3), PATTERN_PLY_BOT_LINE1_H + 20, COLOR_MEDIUM_GREEN);	// Green
		SetSprite(SPRITE_LAUNCHER_BOT_5, 128-8, (u8)(150 + 15), PATTERN_PLY_BOT_LINE1_H + 24, COLOR_WHITE);			// White
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_BOT_6, 213);
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_BOT_7, 213);
		VDP_SetSpritePositionY(SPRITE_LAUNCHER_BOT_8, 213);
	}	
}

//-----------------------------------------------------------------------------
///
void SetEvent(u8 event) __FASTCALL
{
	g_EventID = event;
	g_EventTimer = 25;
	g_EventFrame = 0;
}

//-----------------------------------------------------------------------------
///
void DisplayTrainingScore()
{
	u8 y = (g_TrainSide == SIDE_BOTTOM) ? 0 : 23;

	Print_SelectTextFont(g_DataSrcFont, OFFSET_GAME_SCRFONT);

	Print_SetPosition(1, y);
	if(g_TrainScore < 10)
		Print_DrawChar('0');
	Print_DrawInt(g_TrainScore);

	Print_SetPosition(29, y);
	if(g_TrainBest[g_Level] < 10)
		Print_DrawChar('0');
	Print_DrawInt(g_TrainBest[g_Level]);
}

//-----------------------------------------------------------------------------
///
void TrainingScore(u8 subEvent)
{
	if(g_Ball.point == POINT_FINISHED)
		return;

	if((subEvent == EVENT_IN) && (g_Ball.lastPly != g_TrainSide))
		return;

	if((subEvent == EVENT_IN) && (g_Ball.point == POINT_VALIDATED) && (g_Ball.lastPly == g_TrainSide))
	{
		if(((u8)(g_Ball.srcPos.x - g_TrainZone.x) <= g_TrainSize[g_Level].x) && ((u8)(g_Ball.srcPos.y - g_TrainZone.y) <= g_TrainSize[g_Level].y))
		{
			if(g_TrainScore < 99)
				g_TrainScore++;
			if(g_TrainScore > g_TrainBest[g_Level])
				g_TrainBest[g_Level] = g_TrainScore;
			SetTrainingCursor();
			DisplayTrainingScore();
		}
		else
			return;
	}
	else if(((g_Ball.point == POINT_PENDING) && (g_Ball.lastPly == g_TrainSide))
		|| ((g_Ball.point == POINT_VALIDATED) && (g_Ball.lastPly != g_TrainSide)))
	{
		g_TrainScore = 0;
		SetTrainingCursor();
		DisplayTrainingScore();
	}

	g_Ball.point = POINT_FINISHED;

	SetEvent(EVENT_POINT);
}

//-----------------------------------------------------------------------------
///
void DisplayScore()
{
	Print_SelectTextFont(g_DataSrcFont, OFFSET_GAME_SCRFONT);

	//-------------------------------------------------------------------------
	// SCORE BOARD
	u8 y = SCORE_TXT_Y;
	for(i8 p = 1; p >= 0; --p)
	{
		// Current game
		Print_SetPosition(26, y);
		Print_DrawText(g_ScoreString[g_Points[p]]);

		// Current set
		Print_SetPosition(23, y);
		Print_DrawInt(g_Games[g_CurSet][p]);

		// Player name
		Print_SetPosition(20, y);
		Print_DrawText(g_PlayerString[g_Player[p].control]);

		// Current set
		Print_SetPosition(15, y);
		Print_DrawInt(g_Sets[p]);

		// Previous sets
		if(g_CurSet > 0)
		{
			for(u8 i = 0; i < g_CurSet; ++i)
			{
				Print_SetPosition(5 + (i * 2), y);
				Print_DrawInt(g_Games[i][p]);
			}
		}
		
		y += 2;
	}
	

	//-------------------------------------------------------------------------
	// STATISTICS

	u8 x = 23;
	for(u8 q = 0; q < 2; ++q)
	{
		// Player name
		Print_SetPosition(x, 12);
		Print_DrawText(g_PlayerString[g_Player[q].control]);
		
		// Points
		u8 v = g_TotalPoints[q];
		if(v > 99)
			v = 99;
		Print_SetPosition((v < 10) ? x + 1 : x, 14);
		Print_DrawInt(v);
		
		x += 3;
	}


}

//-----------------------------------------------------------------------------
///
void InitService()
{
	g_ServeState = TRUE;
	g_ServeZone = (g_Server * 2) + g_OddPoint;
	Player_Initialize(SIDE_BOTTOM, 0);
	Player_Initialize(SIDE_TOP, 1);
}

//-----------------------------------------------------------------------------
///
bool Event_WinPoint()
{
	g_EventFrame++;

	if(g_EventFrame == 1)
	{
		if(g_EventSub != EVENT_INOUT)
		{
			// Display Referee Image
			Ball_Hide();
			GM2_LAYOUT(g_DataReferee_Names, 0, 7, 12, 6);		

			// Load Referee Text
			u16 dst;
			const u8* src = g_DataEvent + (g_EventSub * 8 * 8);
			dst	= g_SpritePatternLow + (PATTERN_SCORE_A * 8); // Pattern #112
			VDP_WriteVRAM_16K(src, dst, 8 * 8);
			
			// Display Referee Sprite
			SetSprite(SPRITE_SCORE_1, 58,    60,   PATTERN_SCORE_A, COLOR_WHITE);
			SetSprite(SPRITE_SCORE_2, 58+16, 60,   PATTERN_SCORE_B, COLOR_WHITE);
			SetSprite(SPRITE_SCORE_3, 58+1,  60+1, PATTERN_SCORE_A, COLOR_BLACK);
			SetSprite(SPRITE_SCORE_4, 58+17, 60+1, PATTERN_SCORE_B, COLOR_BLACK);
			VDP_HideSpriteFrom(SPRITE_SCORE_5);
		}
		else
		{
			g_EventFrame += EVENT_DELAY / 2;
		}
	}
	if(g_EventFrame < EVENT_DELAY)
	{
		return TRUE;
	}
	
	// Display Score
	if(g_EventFrame == EVENT_DELAY)
	{
		// Draw Court
		VDP_FillScreen_GM2(0);
		GM2_LAYOUT(g_DataCourt_Names, 3, 3, 27, 18);

		// Top Player Score
		const u8* src = g_DataPoints + (g_Points[1] * 8 * 4);
		u16 dst	= g_SpritePatternLow + (PATTERN_SCORE_A * 8); // Pattern #112
		VDP_WriteVRAM_16K(src, dst, 8 * 4);		
		SetSprite(SPRITE_SCORE_1, 0, 213, PATTERN_SCORE_A, VDP_SPRITE_EC | SCORE_COLOR);
		SetSprite(SPRITE_SCORE_2, 0, 213, PATTERN_SCORE_A, VDP_SPRITE_EC | SCORE_SHADE);
		
		// Top Player Score
		src = g_DataPoints + (g_Points[0] * 8 * 4);
		dst	= g_SpritePatternLow + (PATTERN_SCORE_B * 8); // Pattern #116
		VDP_WriteVRAM_16K(src, dst, 8 * 4);		
		SetSprite(SPRITE_SCORE_3, 0, 213, PATTERN_SCORE_B, SCORE_COLOR);
		SetSprite(SPRITE_SCORE_4, 0, 213, PATTERN_SCORE_B, SCORE_SHADE);

		VDP_HideSpriteFrom(SPRITE_SCORE_5);

		InitService();
		Ball_Hide();
	}
	if(g_EventFrame < EVENT_DELAY+16)
	{
		u8 x = ((g_EventFrame - EVENT_DELAY) * 8);
		VDP_SetSpritePosition(SPRITE_SCORE_1, SCORE_TOP_X   + x, SCORE_TOP_Y);
		VDP_SetSpritePosition(SPRITE_SCORE_2, SCORE_TOP_X+1 + x, SCORE_TOP_Y+1);
		VDP_SetSpritePosition(SPRITE_SCORE_3, SCORE_BOT_X   - x, SCORE_BOT_Y);
		VDP_SetSpritePosition(SPRITE_SCORE_4, SCORE_BOT_X+1 - x, SCORE_BOT_Y+1);
	}
	if(g_EventFrame == EVENT_DELAY+16)
	{
		ayFX_PlayBank(13, 0);
	}
	if(g_EventFrame == EVENT_DELAY+50)
	{
		// Hide score sprites
		VDP_HideSpriteFrom(SPRITE_SCORE_1);
		g_EventID = EVENT_NONE;
	}
	
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool Event_Default(u8 event)
{
	if(g_EventFrame == 0)
	{
		// Text
		{
			const u8* src = g_DataEvent + (event * 8 * 8);
			u16 dst	= g_SpritePatternLow + (PATTERN_SCORE_A * 8); // Pattern #112
			VDP_WriteVRAM_16K(src, dst, 8 * 8);
			SetSprite(SPRITE_SCORE_1, 0, 213, PATTERN_SCORE_A, VDP_SPRITE_EC | SCORE_COLOR);
			SetSprite(SPRITE_SCORE_2, 0, 213, PATTERN_SCORE_B, VDP_SPRITE_EC | SCORE_COLOR);
			SetSprite(SPRITE_SCORE_3, 0, 213, PATTERN_SCORE_A, VDP_SPRITE_EC | SCORE_SHADE);
			SetSprite(SPRITE_SCORE_4, 0, 213, PATTERN_SCORE_B, VDP_SPRITE_EC | SCORE_SHADE);
		}
		
		// Player
		{
			u8 ctrl = g_Player[g_Winner].control;
			const u8* src = g_DataPoints + ((5 + ctrl) * 8 * 4); // 5: AI, 6: P1, 7: P2
			u16 dst	= g_SpritePatternLow + (PATTERN_SCORE_PLY * 8); // Pattern #116
			VDP_WriteVRAM_16K(src, dst, 8 * 4);
			SetSprite(SPRITE_SCORE_5, 0, 213, PATTERN_SCORE_PLY, SCORE_COLOR);
			SetSprite(SPRITE_SCORE_6, 0, 213, PATTERN_SCORE_PLY, SCORE_SHADE);
		}

		VDP_HideSpriteFrom(SPRITE_SCORE_BALL_1);		

		InitService();
		Ball_Hide();
	}
	if(g_EventFrame < 16)
	{
		VDP_SetSpritePosition(SPRITE_SCORE_1, EVENT_TOP_X    + (g_EventFrame * 8), EVENT_TOP_Y);
		VDP_SetSpritePosition(SPRITE_SCORE_2, EVENT_TOP_X+16 + (g_EventFrame * 8), EVENT_TOP_Y);
		VDP_SetSpritePosition(SPRITE_SCORE_3, EVENT_TOP_X+1  + (g_EventFrame * 8), EVENT_TOP_Y+1);
		VDP_SetSpritePosition(SPRITE_SCORE_4, EVENT_TOP_X+17 + (g_EventFrame * 8), EVENT_TOP_Y+1);

		VDP_SetSpritePosition(SPRITE_SCORE_5, EVENT_BOT_X   - (g_EventFrame * 8), EVENT_BOT_Y);
		VDP_SetSpritePosition(SPRITE_SCORE_6, EVENT_BOT_X+1 - (g_EventFrame * 8), EVENT_BOT_Y+1);
	}
	if(g_EventFrame == 16)
	{
		ayFX_PlayBank(13, 0);
	}
	if(g_EventFrame == 50)
	{
		// Hide score sprites
		VDP_HideSpriteFrom(SPRITE_SCORE_1);
		g_EventID = EVENT_NONE;
		if(event == EVENT_SET)
			Game_SetState(State_ScoreStart);
		else if(event == EVENT_MATCH)
			Game_SetState(State_VictoryStart);
	}
	g_EventFrame++;
	
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool HandleEvent()
{
	switch(g_EventID)
	{
	case EVENT_POINT:
		return Event_WinPoint();
	case EVENT_GAME:
	case EVENT_SET:
	case EVENT_MATCH:
		return Event_Default(g_EventID);
	default:
		break;
	};
	return TRUE;
}

//-----------------------------------------------------------------------------
///
void InitNewGame()
{
	g_Points[0] = 0;
	g_Points[1] = 0;
	g_Server = 1 - g_Server;
	g_OddPoint = FALSE;
}


//-----------------------------------------------------------------------------
///
void MatchScore(u8 subEvent)
{
	if((g_Ball.point == POINT_FINISHED) || (subEvent == EVENT_IN))
		return;

	u8 event = EVENT_POINT;
		
	// Win the point
	if(g_Ball.point == POINT_VALIDATED)
	{
		g_Winner = g_Ball.lastPly;
		g_Points[g_Ball.lastPly]++;
		g_TotalPoints[g_Ball.lastPly]++;
	}
	else
	{
		g_Winner = 1 - g_Ball.lastPly;
		g_Points[1 - g_Ball.lastPly]++;
		g_TotalPoints[1 - g_Ball.lastPly]++;
	}

	// Deuce
	if((g_Points[0] == SCORE_ADV) && (g_Points[1] == SCORE_ADV))
	{
		g_Points[0] = SCORE_40;
		g_Points[1] = SCORE_40;
	}

	g_OddPoint = (g_Points[0] + g_Points[1]) & 0x1;
		
	for(u8 i = 0; i < 2; ++i)
	{
		// Win the current game
		if((g_Points[i] == SCORE_WIN) || ((g_Points[i] == SCORE_ADV) && (g_Points[1-i] < SCORE_40)))
		{
			g_Winner = i;
			g_Games[g_CurSet][i]++;
			event = EVENT_GAME;
			InitNewGame();
		}
		
		// Win the current set
		if(((g_Games[g_CurSet][i] == 6) && (g_Games[g_CurSet][1-i] < 5))
		|| ((g_Games[g_CurSet][i] > 6) && (g_Games[g_CurSet][i] > g_Games[g_CurSet][1-i] + 1)))
		{
			g_Winner = i;
			g_Sets[i]++;
			g_CurSet++;
			event = EVENT_SET;
			InitNewGame();
		}

		// Win the match!
		if(g_Sets[i] >= g_OptionSets + 1)
		{
			g_Winner = i;
			event = EVENT_MATCH;
			// Game_SetState(State_TitleStart); // return to menu for the moment
		}
	}
	
	#if (DEBUG)
		if(g_Debug)
			DisplayScore();
	#endif
	
	SetEvent(event);
	g_EventSub = subEvent;
	
	g_Ball.point = POINT_FINISHED;
}


//=============================================================================
//
//   I N P U T
//
//=============================================================================

//-----------------------------------------------------------------------------
///
void UpdateInput()
{
	// Keyboard
	for(u8 i = 0; i < 9; ++i)
	{
		g_PrevRow[i] = g_KeyRow[i];	
		g_KeyRow[i] = Keyboard_Read(i);
	}
	// Joystick
	g_PrevJoy1 = g_Joy1;
	g_PrevJoy2 = g_Joy2;
	g_Joy1 = Joystick_Read(JOY_PORT_1);
	g_Joy2 = Joystick_Read(JOY_PORT_2);
}


//-----------------------------------------------------------------------------
// INPUT CALLBACK

bool KB1_Hold_Up()		{ return KEY_ON(KEY_UP); }
bool KB1_Hold_Down()	{ return KEY_ON(KEY_DOWN); }
bool KB1_Hold_Left()	{ return KEY_ON(KEY_LEFT); }
bool KB1_Hold_Right()	{ return KEY_ON(KEY_RIGHT); }

bool KB1_Press_Up()		{ return KEY_PRESS(KEY_UP); }
bool KB1_Press_Down()	{ return KEY_PRESS(KEY_DOWN); }
bool KB1_Press_Left()	{ return KEY_PRESS(KEY_LEFT); }
bool KB1_Press_Right()	{ return KEY_PRESS(KEY_RIGHT); }

bool KB1A_Button1()		{ return KEY_PRESS(KEY_SPACE); }
bool KB1A_Button2()		{ return KEY_PRESS(KEY_N); }
bool KB1B_Button1()		{ return KEY_PRESS(KEY_SHIFT); }
bool KB1B_Button2()		{ return KEY_PRESS(KEY_CTRL); }
bool KB1C_Button1()		{ return KEY_PRESS(KEY_RET); }
bool KB1C_Button2()		{ return KEY_PRESS(KEY_BS); }

bool KB2_Hold_Up()		{ return KEY_ON(KEY_E); }
bool KB2_Hold_Down()	{ return KEY_ON(KEY_D); }
bool KB2_Hold_Left()	{ return KEY_ON(KEY_S); }
bool KB2_Hold_Right()	{ return KEY_ON(KEY_F); }

bool KB2_Press_Up()		{ return KEY_PRESS(KEY_E); }
bool KB2_Press_Down()	{ return KEY_PRESS(KEY_D); }
bool KB2_Press_Left()	{ return KEY_PRESS(KEY_S); }
bool KB2_Press_Right()	{ return KEY_PRESS(KEY_F); }

bool Joy1_Hold_Up()		{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_UP)); }
bool Joy1_Hold_Down()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_DOWN)); }
bool Joy1_Hold_Left()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_LEFT)); }
bool Joy1_Hold_Right()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_RIGHT)); }

bool Joy1_Press_Up()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_UP)		&& !IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_DIR_UP)); }
bool Joy1_Press_Down()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_DOWN)	&& !IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_DIR_DOWN)); }
bool Joy1_Press_Left()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_LEFT)	&& !IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_DIR_LEFT)); }
bool Joy1_Press_Right()	{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_DIR_RIGHT)	&& !IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_DIR_RIGHT)); }

bool Joy1_Button1()		{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_TRIGGER_A) && !(IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_TRIGGER_A))); }	
bool Joy1_Button2()		{ return (IS_JOY_PRESSED(g_Joy1, JOY_INPUT_TRIGGER_B) && !(IS_JOY_PRESSED(g_PrevJoy1, JOY_INPUT_TRIGGER_B))); }

bool Joy2_Hold_Up()		{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_UP)); }
bool Joy2_Hold_Down()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_DOWN)); }
bool Joy2_Hold_Left()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_LEFT)); }
bool Joy2_Hold_Right()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_RIGHT)); }

bool Joy2_Press_Up()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_UP)    && !IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_DIR_UP)); }
bool Joy2_Press_Down()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_DOWN)  && !IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_DIR_DOWN)); }
bool Joy2_Press_Left()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_LEFT)  && !IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_DIR_LEFT)); }
bool Joy2_Press_Right()	{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_DIR_RIGHT) && !IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_DIR_RIGHT)); }

bool Joy2_Button1()		{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_TRIGGER_A) && !(IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_TRIGGER_A))); }	
bool Joy2_Button2()		{ return (IS_JOY_PRESSED(g_Joy2, JOY_INPUT_TRIGGER_B) && !(IS_JOY_PRESSED(g_PrevJoy2, JOY_INPUT_TRIGGER_B))); }

//=============================================================================
//
//   G A M E   S T A T E S
//
//=============================================================================

//-----------------------------------------------------------------------------
///
bool State_Init()
{
	// Setup VRAM tables
	VDP_SetLayoutTable(VRAM_LAYOUT_TABLE);
	VDP_SetColorTable(VRAM_COLOR_TABLE);
	VDP_SetPatternTable(VRAM_PATTERN_TABLE);
	VDP_SetSpritePatternTable(VRAM_SPRITE_PATTERN);
	VDP_SetSpriteAttributeTable(VRAM_SPRITE_ATTRIBUTE);
	// Sprite settings
	VDP_SetSpriteFlag(VDP_SPRITE_SIZE_16 | VDP_SPRITE_SCALE_1);
	VDP_EnableVBlank(TRUE);
	
	g_InputBinding[SIDE_BOTTOM] = BIND_KB1A;
	g_InputBinding[SIDE_TOP] = BIND_KB2;
		
	// Initialize PT3
	PT3_Init();
	PT3_SetNoteTable(PT3_NT2);
	PT3_SetLoop(TRUE);
	PT3_InitSong(g_DataMusic);
	PT3_Pause();
	g_PlayMusic = MUSIC_ENABLE;

	// Initialize ayFX
	ayFX_InitBank(g_DataSFX);
	ayFX_SetChannel(PSG_CHANNEL_A);
	ayFX_SetMode(AYFX_MODE_FIXED);
	g_PlaySFX = SFX_ENABLE;

	Game_SetState(State_TitleStart);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_TitleStart()
{
	VDP_EnableDisplay(FALSE);

	VDP_SetColor(COLOR_BLACK);

	// Load screen data
	VDP_FillScreen_GM2(0); // Don't set the Layout table yet
	UnpackGM2ToVRAM(g_DataLogo_Patterns, g_ScreenPatternLow + (OFFSET_TITLE_LOGO * 8));
	UnpackGM2ToVRAM(g_DataLogo_Colors,   g_ScreenColorLow   + (OFFSET_TITLE_LOGO * 8));

	// Load sprites data
	UnpackSpriteToVRAM(g_DataLogoBall, g_SpritePatternLow);

	// Initialize sprite attributes
	SetSprite(0, 0, 0, 0,  VDP_SPRITE_EC | COLOR_LIGHT_YELLOW);
	SetSprite(1, 0, 0, 4,  VDP_SPRITE_EC | COLOR_DARK_RED);
	SetSprite(2, 0, 0, 8,  VDP_SPRITE_EC | COLOR_LIGHT_YELLOW);
	SetSprite(3, 0, 0, 12, VDP_SPRITE_EC | COLOR_DARK_RED);
	SetSprite(4, 0, 0, 16, VDP_SPRITE_EC | COLOR_LIGHT_YELLOW);
	SetSprite(5, 0, 0, 20, VDP_SPRITE_EC | COLOR_DARK_RED);
	SetSprite(6, 0, 0, 24, VDP_SPRITE_EC | COLOR_LIGHT_YELLOW);
	SetSprite(7, 0, 0, 28, VDP_SPRITE_EC | COLOR_DARK_RED);
	VDP_HideSpriteFrom(8);

	VDP_HideSpriteFrom(SPRITE_SCORE_1);

	// Initialize font
	Print_SetTextFont(g_DataFont, OFFSET_TITLE_FONT_DEF);
	Print_SetColorShade(g_ColorShadeDefault);
	u16 dst = (u16)g_ScreenPatternLow + ((OFFSET_TITLE_FONT_DEF - '!' + '_') * 8); // clear '_' character
	VDP_FillVRAM_16K(0, dst + 0 * 256 * 8, 8);
	VDP_FillVRAM_16K(0, dst + 1 * 256 * 8, 8);
	VDP_FillVRAM_16K(0, dst + 2 * 256 * 8, 8);

	Print_SetTextFont(g_DataFont, OFFSET_TITLE_FONT_ALT);
	Print_SetColorShade(g_ColorShadeSelect);
	
	g_EventFrame = 0;

	VDP_EnableDisplay(TRUE);
	Game_SetState(State_TitleUpdate);
	return FALSE;
}


//-----------------------------------------------------------------------------
///
void MoveLogoBall(u8 x) __FASTCALL
{
	x += 4;
	VDP_SetSpritePosition(0, x, 32 +  0);
	VDP_SetSpritePosition(1, x, 32 +  0);
	VDP_SetSpritePosition(4, x, 32 + 16);
	VDP_SetSpritePosition(5, x, 32 + 16);
	x += 16;
	VDP_SetSpritePosition(2, x, 32 +  0);
	VDP_SetSpritePosition(3, x, 32 +  0);
	VDP_SetSpritePosition(6, x, 32 + 16);
	VDP_SetSpritePosition(7, x, 32 + 16);
}

//-----------------------------------------------------------------------------
///
bool State_TitleUpdate()
{
	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	UpdateInput();

	if(g_EventFrame < 25) // Ball movement
	{
		MoveLogoBall(g_EventFrame * 8);
	}
	else if(g_EventFrame == 25) // White flash
	{	
		ayFX_PlayBank(13, 0);
		VDP_SetColor(COLOR_WHITE);
		VDP_FillVRAM_16K(0x0F, g_ScreenColorLow + (0 * 0x800),  8);
		VDP_FillVRAM_16K(0x0F, g_ScreenColorLow + (1 * 0x800),  8);
		VDP_FillVRAM_16K(0x0F, g_ScreenColorLow + (2 * 0x800),  8);
	}
	else if(g_EventFrame == 29) // Title
	{	
		VDP_SetColor(COLOR_BLACK);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (0 * 0x800),  8);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (1 * 0x800),  8);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (2 * 0x800),  8);
		GM2_LAYOUT(g_DataLogo_Names, 4, 2, 19, 10);
	}
	else if(g_EventFrame == 64) // Title
	{
		PT3_InitSong(g_DataMusic);
		PT3_Resume();

		Print_SelectTextFont(g_DataFont, OFFSET_TITLE_FONT_DEF);
		Print_SetPosition(7, 16);
		Print_DrawText("PIXEL PHENIX # 2023");

		Print_SelectTextFont(g_DataFont, OFFSET_TITLE_FONT_ALT);
		Print_SetPosition(11, 22);
		Print_DrawText("PRESS A KEY");
	}
	if(g_EventFrame < 255)
		g_EventFrame++;

	bool bPressed = FALSE;
	for(u8 i = 0; i < BIND_MAX; ++i)
	{
		if(g_Binding[i].inButton1() || g_Binding[i].inButton2())
		{
			g_InputBinding[SIDE_BOTTOM] = i;
			bPressed = TRUE;
			break;
		}		
	}
	
	if(bPressed) // Skip
	{
		PT3_InitSong(g_DataMusic);
		PT3_Resume();

		VDP_SetColor(COLOR_BLACK);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (0 * 0x800),  8);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (1 * 0x800),  8);
		VDP_FillVRAM_16K(0x01, g_ScreenColorLow + (2 * 0x800),  8);
		GM2_LAYOUT(g_DataLogo_Names, 4, 2, 19, 10);

		MoveLogoBall(24 * 8);

		Game_SetState(State_MenuStart);

		u16 dst = g_ScreenLayoutLow + (12 * 32);
		VDP_FillVRAM_16K(0, dst, 12 * 32);
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
///
bool State_MenuStart()
{
	Menu_Initi(&g_Menus[g_MenuID]);
	Game_SetState(State_MenuUpdate);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_MenuUpdate()
{
	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	UpdateInput();
	Menu_Update();
	return TRUE;
}

//-----------------------------------------------------------------------------
///
bool State_ScoreStart()
{
	// Display Score board
	VDP_FillScreen_GM2(0);
	GM2_LAYOUT(g_DataScoreL0_Names, 3, SCORE_BOARD_Y, 27, 10);
	GM2_LAYOUT(g_DataScoreL1_Names, 16, SCORE_BOARD_Y+10, 14, 12);

	// Hide sprites
	for(u8 s = 0; s < SPRITE_SCORE_BALL_1; ++s)
		VDP_SetSpritePositionY(s, 213);

	SetSprite(SPRITE_SCORE_BALL_4, 132, 213, PATTERN_SERVER_BASE,    COLOR_LIGHT_YELLOW);	// Base
	SetSprite(SPRITE_SCORE_BALL_3, 132, 213, PATTERN_SERVER_SHADE,   COLOR_DARK_YELLOW);	// Shade
	SetSprite(SPRITE_SCORE_BALL_2, 132, 213, PATTERN_SERVER_STRIP,   COLOR_WHITE);			// Strip
	SetSprite(SPRITE_SCORE_BALL_1, 132, 213, PATTERN_SERVER_OUTLINE, COLOR_BLACK);			// Outline
	
	DisplayScore();
	
	Game_SetState(State_ScoreUpdate);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_ScoreUpdate()
{
	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	UpdateInput();

	i16 dy = g_Cosinus64[(g_GameFrame % 32) * 2] / 32;
	u8 y = 47 + 16 * (1 - g_Server) + dy;
	
	VDP_SetSpritePositionY(SPRITE_SCORE_BALL_1, y);
	VDP_SetSpritePositionY(SPRITE_SCORE_BALL_2, y);
	VDP_SetSpritePositionY(SPRITE_SCORE_BALL_3, y);
	VDP_SetSpritePositionY(SPRITE_SCORE_BALL_4, y);

	if(KEY_PRESS(KEY_F1) || g_Player[SIDE_BOTTOM].binding->inButton1() || g_Player[SIDE_TOP].binding->inButton1())
	{
		VDP_HideSpriteFrom(SPRITE_SCORE_BALL_1);
		Game_SetState(State_CourtRestore);
		return FALSE;
	}
	if(KEY_PRESS(KEY_ESC)) // Return to main menu
	{
		Game_SetState(State_TitleStart);
		return FALSE;
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
///
bool State_VictoryStart()
{
	State_ScoreStart();
	Game_SetState(State_VictoryUpdate);

	Player* ply = &g_Player[g_Winner];
	SetAction(ply, ACTION_WIN);
	ply->srcPos.x = 50;
	ply->srcPos.y = 164;
	
	ply = &g_Player[1-g_Winner];
	SetAction(ply, ACTION_LOOSE);
	ply->srcPos.x = 80;
	ply->srcPos.y = 164-40;

	g_DisplayNet = FALSE;

	SetSprite(SPRITE_CUP_OUTLINE, 50-8, (u8)(164-33), PATTERN_CUP_OUTLINE1, COLOR_BLACK);
	SetSprite(SPRITE_CUP_BODY,    50-8, (u8)(164-33), PATTERN_CUP_BODY,     COLOR_LIGHT_YELLOW);
	
	if(g_PlayMusic)
	{
		PT3_InitSong(g_DataMusic);
		PT3_Resume();
	}
	
	return FALSE;
}


//-----------------------------------------------------------------------------
///
bool State_VictoryUpdate()
{
	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	u8 pat = PATTERN_CUP_OUTLINE1;
	if(g_FlickerShadow && (g_GameFrame & 0x1))
		pat += 4;
	VDP_SetSpritePattern(SPRITE_CUP_OUTLINE, pat);

	Player_SwitchSprites(&g_Player[SIDE_BOTTOM]);
	Player_SwitchSprites(&g_Player[SIDE_TOP]);

	UpdatePlayerBottom();
	UpdatePlayerTop();

	Player_UpdateAction(&g_Player[SIDE_BOTTOM]);
	Player_UpdateAction(&g_Player[SIDE_TOP]);

	DrawPlayerBottom();
	DrawPlayerTop();
	
	UpdateInput();
	if(g_Player[SIDE_BOTTOM].binding->inButton1() || g_Player[SIDE_TOP].binding->inButton1())
	{
		Game_SetState(State_TitleStart);
		return FALSE;
	}

	return TRUE;
}



//-----------------------------------------------------------------------------
///
bool State_CourtRestore()
{
	// Restore Court
	VDP_FillScreen_GM2(0);
	GM2_LAYOUT(g_DataCourt_Names, 3, 3, 27, 18);
	
	Game_SetState(State_MatchUpdate);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_MatchStart()
{
	VDP_EnableDisplay(FALSE);

	InitializeCourt();
	
	// Initialize score
	g_ScoreFct = MatchScore;
	g_NumSet = (g_OptionSets * 2) + 1;
	g_CurSet = 0;
	g_TotalPoints[0] = 0;
	g_TotalPoints[1] = 0;
	g_Points[0] = 0;
	g_Points[1] = 0;
	g_OddPoint = FALSE;
	g_Sets[0] = 0;
	g_Sets[1] = 0;
	for(u8 i = 0; i < g_NumSet; ++i)
	{
		g_Games[i][0] = 0;
		g_Games[i][1] = 0;
	}
	g_Server = SIDE_BOTTOM;

	// Initialize timer
	g_EventID = EVENT_NONE;
	g_EventTimer = 0;

	// Initialize ball
	Ball_Hide();
	InitService();

	VDP_EnableDisplay(TRUE);
	Game_SetState(State_MatchUpdate);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_MatchUpdate()
{
// VDP_SetColor(COLOR_BLACK);

	//---------------------------------------------------------------------
	// Switch shaded sprites during V-Blank
	Player_SwitchSprites(&g_Player[SIDE_BOTTOM]);
	Player_SwitchSprites(&g_Player[SIDE_TOP]);
	Ball_SwitchSprites();

// VDP_SetColor(COLOR_LIGHT_GREEN);

	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	//---------------------------------------------------------------------
	// Handle match event
	
	bool bAllowInput = HandleEvent();

	//---------------------------------------------------------------------
	// Handle input
	
	UpdateInput();

	#if (DEBUG)
		if(KEY_PRESS(KEY_HOME))
			g_Debug = 1 - g_Debug;
		// if(KEY_PRESS(KEY_F2))
			// Ball_ShootRandom();
	#endif
	
	if(KEY_PRESS(KEY_ESC)) // Return to main menu
	{
		// VDP_HideSpriteFrom(0);
		Game_SetState(State_TitleStart);
		return FALSE;
	}
	if(KEY_PRESS(KEY_F1))
	{
		Game_SetState(State_ScoreStart);
		return FALSE;
	}
	if(KEY_PRESS(KEY_F3)) // Activate/deactivate shadows
		g_FlickerShadow = 1 - g_FlickerShadow;
	
	if(bAllowInput)
	{
		if(g_ServeState)
		{
			Player_HandleService(&g_Player[g_Server]);
			Player_HandleInput(&g_Player[1-g_Server]);
			
		}
		else
		{
			Player_HandleInput(&g_Player[SIDE_BOTTOM]);
			Player_HandleInput(&g_Player[SIDE_TOP]);
		}
	}

// VDP_SetColor(COLOR_LIGHT_BLUE);

	//---------------------------------------------------------------------
	// Update sprites position
	
	UpdatePlayerBottom();
	UpdatePlayerTop();
	Ball_Update();

	Player_UpdateAction(&g_Player[SIDE_BOTTOM]);
	Player_UpdateAction(&g_Player[SIDE_TOP]);

// VDP_SetColor(COLOR_MAGENTA);

	//---------------------------------------------------------------------
	// Draw anim
	
	DrawPlayerBottom();
	DrawPlayerTop();

// VDP_SetColor(COLOR_LIGHT_YELLOW);

	return TRUE;
}

//-----------------------------------------------------------------------------
///
bool State_WinMatch()
{
	if(g_EventFrame == 0)
	{
		InitService();
		Game_SetState(State_MatchUpdate);
	}
	g_EventFrame++;
	return TRUE;
}

//-----------------------------------------------------------------------------
///
bool State_TrainingStart()
{
	VDP_EnableDisplay(FALSE);

	InitializeCourt();
	if(g_TrainSide == SIDE_BOTTOM)
	{
		g_ServeZone = 0;
		g_Server = SIDE_BOTTOM;
		Player_Initialize(SIDE_BOTTOM, g_TrainSide);
	}
	else
	{
		g_ServeZone = 2;
		g_Server = SIDE_TOP;
		Player_Initialize(SIDE_TOP, 1-g_TrainSide);
	}
	Launcher_Initialize();
	
	// Initialize score board
	g_TrainScore = 0;
	g_ScoreFct = TrainingScore;
	if(g_TrainSide == SIDE_BOTTOM)
	{
		GM2_LAYOUT(g_DataScoreL2_Names, 0, 0, 9, 3);
		GM2_LAYOUT(g_DataScoreL3_Names, 32-9, 0, 9, 3);
	}
	else // if(g_TrainSide == SIDE_TOP)
	{
		GM2_LAYOUT(g_DataScoreL4_Names, 0, 22, 9, 2);
		GM2_LAYOUT(g_DataScoreL5_Names, 32-9, 22, 9, 2);
	}
	DisplayTrainingScore();

	g_EventID = EVENT_NONE;
	Ball_Hide();
	Ball_ShootRandom();
	SetTrainingCursor();

	VDP_EnableDisplay(TRUE);
	Game_SetState(State_TrainingUpdate);
	return FALSE;
}

//-----------------------------------------------------------------------------
///
bool State_TrainingUpdate()
{
	//---------------------------------------------------------------------
	// Switch buffer

	// Prepare sprite objects
	if(g_TrainSide == SIDE_BOTTOM)
		Player_SwitchSprites(&g_Player[SIDE_BOTTOM]);
	else
		Player_SwitchSprites(&g_Player[SIDE_TOP]);
	Launcher_SwitchSprites();
	Ball_SwitchSprites();

	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return TRUE;

	//---------------------------------------------------------------------
	// Update input

	UpdateInput();

	// if(KEY_PRESS(KEY_F2))
		// Ball_ShootRandom();
	if(KEY_PRESS(KEY_F3)) // Activate/deactivate shadows
		g_FlickerShadow = 1 - g_FlickerShadow;
	if(KEY_PRESS(KEY_ESC)) // Return to main menu
	{
		VDP_HideSpriteFrom(0);
		Game_SetState(State_TitleStart);
	}

	#if (DEBUG)

		if(KEY_PRESS(KEY_HOME))
		{
			g_Debug = 1 - g_Debug;
			if(g_Debug)
			{
				u8 y = 4;
				Print_SelectTextFont(g_DataFont, OFFSET_GAME_FONT);
				Print_SetPosition(1, y++);
				Print_DrawText("ACT:");
				Print_SetPosition(1, y++);
				Print_DrawText("STP:");
				Print_SetPosition(1, y++);
				Print_DrawText("ANI:");
				y++;
				Print_SetPosition(1, y++);
				Print_DrawText("SHT:");
				Print_SetPosition(1, y++);
				Print_DrawText("DIR:");
				Print_SetPosition(1, y++);
				Print_DrawText("CNT:");
				y++;
				Print_SetPosition(1, y++);
				Print_DrawText("LST:");
				Print_SetPosition(1, y++);
				Print_DrawText("STA:");

				Print_SetPosition(1, 19);
				if(g_Freq == FREQ_50HZ)
					Print_DrawText("FRQ:50HZ");
				else if(g_Freq == FREQ_60HZ)
					Print_DrawText("FRQ:60HZ");
				else
					Print_DrawText("FRQ:???");

				#if (TARGET_TYPE == TYPE_ROM)
					Print_SetPosition(1, 20);
					Print_DrawText("ROM:");
					Print_DrawHex8(g_VersionROM);
					Print_SetPosition(1, 21);
					Print_DrawText("MSX:");
					Print_DrawHex8(g_VersionMSX);
				#endif
				
				Print_SetPosition(1, 22);
				#if (MSX2_ENHANCE)
					Print_DrawText((g_VersionVDP == VDP_VERSION_TMS9918A) ? "VDP:9918" : "VDP:9938+");
				#else
					Print_DrawText("VDP:9918");
				#endif
			}
		}

		if(g_Debug)
		{
			u8 y = 4;
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].action);
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].step);
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].anim);
			y++;
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].shot);
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].shotDir);
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Player[0].shotCnt);
			y++;
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Ball.lastPly);
			Print_SetPosition(5, y++);
			Print_DrawHex8(g_Ball.point);
		}
	#endif
	
	Player_HandleInput(&g_Player[g_TrainSide]);
	
	//---------------------------------------------------------------------
	// Update sprites position
	
	if(g_TrainSide == SIDE_BOTTOM)
		UpdatePlayerBottom();
	else
		UpdatePlayerTop();
		
	Ball_Update();

	Player_UpdateAction(&g_Player[g_TrainSide]);

	UpdateTrainingCursor();
	
	//---------------------------------------------------------------------
	// Draw anim
	
	if(g_TrainSide == SIDE_BOTTOM)
		DrawPlayerBottom();
	else
		DrawPlayerTop();
	
	if(g_EventID != EVENT_NONE)
	{
		if(g_EventTimer == 0)
		{
			Ball_ShootRandom();
			g_EventID = EVENT_NONE;
		}
		g_EventTimer--;
	}
	
	return TRUE;
}

//=============================================================================
//
//   M A I N
//
//=============================================================================

// #if (TARGET_TYPE == TYPE_ROM)
// //-----------------------------------------------------------------------------
// /// ISR for 48K ROM
// void VDP_InterruptHandler()
// {
// 	Game_VSyncHook();
// }
// #endif

//-----------------------------------------------------------------------------
/// Main loop
void VSyncCallback()
{
	if((g_Freq == FREQ_60HZ) && ((g_GameFrame % 6) == 0)) // skip 6th frame for 60 Hz
		return;

	if(g_PlayMusic)
		PT3_Decode();
	if(g_PlaySFX)
		ayFX_Update();
	if(g_PlayMusic || g_PlaySFX)
		PT3_UpdatePSG();
}

//-----------------------------------------------------------------------------
/// Main loop
void main()
{
	#if (MSX2_ENHANCE)
	g_KeyRow[0] = Keyboard_Read(0); // 0 1 2 3 4 5 6 7
	if(KEY_ON(KEY_1))
		g_VersionVDP = VDP_VERSION_TMS9918A;
	else if(KEY_ON(KEY_2))
		g_VersionVDP = VDP_VERSION_V9938;
	else
		g_VersionVDP = VDP_GetVersion();
	#endif
	
	#if (TARGET_TYPE == TYPE_ROM)
	if(g_VersionROM & 0x80)
		g_FreqDetected = FREQ_50HZ;
	else
		g_FreqDetected = FREQ_60HZ;
	g_Freq = g_FreqDetected;
	#endif

	// Initialize VDP
	#if (MSX2_ENHANCE)
	if(g_VersionVDP == VDP_VERSION_TMS9918A)
		Game_Initialize(VDP_MODE_GRAPHIC2);
	else // VDP_VERSION_V99xx
		Game_Initialize(VDP_MODE_GRAPHIC3);
	#else
	Game_Initialize(VDP_MODE_GRAPHIC2);
	#endif

	Game_SetVSyncCallback(VSyncCallback);
	Game_SetState(State_Init);

	while(1)
	{
		Game_Update();
	}
}

// void Bios_SetHookCallback(u16 hook, callback cb) {}