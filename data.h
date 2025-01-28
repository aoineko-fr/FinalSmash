//_____________________________________________________________________________
//  ▄▄▄▄ ▄            ▄▄     ▄▄▄                ▄▄  
//  ██▄  ▄  ██▀▄ ▄▀██ ██    ▀█▄  ▄█▄█ ▄▀██  ██▀ ██▄ 
//  ██   ██ ██ █ ▀▄██ ▀█▄   ▄▄█▀ ██ █ ▀▄██ ▄██  ██ █
//
//  ▄▄▄   ▄▄  ▄▄▄▄  ▄▄ 
//  ██ █ ██▄█  ██  ██▄█
//  ██▄▀ ██ █  ██  ██ █
//_____________________________________________________________________________

//=============================================================================
//
//  ROM DATA - PAGE 0
//
//=============================================================================

extern const unsigned char g_DataPlayer1[];
extern const unsigned char g_DataPlayer2[];
extern const unsigned char g_DataSFX[];
extern const unsigned char g_DataMusic[];
extern const unsigned char g_DataLogoBall[];
extern const unsigned char g_DataCourt_Names[];
extern const unsigned char g_DataCourt_Patterns[];
extern const unsigned char g_DataCourt_Colors[];

//=============================================================================
//
//  ROM DATA - PAGE 1-2
//
//=============================================================================

// Menu title (GM2 tables)
#include "content/data_logo.h"

// Score board
#include "content/data_board.h"

// Pentaru referee
#include "content/data_referee.h"

// Ball launcher sprites
#include "content/data_launcher0.h"
#include "content/data_launcher1.h"

// Fonts data
#include "content/data_font.h"
#include "content/data_scrfont.h"

// Score point sprites
#include "content/data_points.h"
#include "content/data_event.h"

// Cup Sprites
#include "content/data_cup.h" 
// Net Sprites
#include "content/data_net.h" 
// Racket sprites
#include "content/data_racket.h"
// Ball sprites
#include "content/data_ball.h"

// Misc
#include "pt3/pt3_notetable2.h"
#include "mathtable/mt_trigo_Q10.6_64.inc"
