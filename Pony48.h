/*
	Pony48 source - Pony48.h
	Copyright (c) 2014 Mark Hutcheson
*/
#ifndef PONY48ENGINE_H
#define PONY48ENGINE_H

#include "Engine.h"
#include "bg.h"
#include <vector>
#include <set>
#include "webcam.h"
#include "luainterface.h"

#define DEFAULT_WIDTH	800
#define DEFAULT_HEIGHT	600

#define DEFAULT_TIMESCALE	1.0

//const variables
#define BOARD_WIDTH	4
#define BOARD_HEIGHT 4

#define TILE_WIDTH 2.0
#define TILE_HEIGHT 2.0
#define TILE_SPACING 0.25

#define GAMEOVER_KEY_DELAY 0.5
#define GAMEOVER_FREEZE_CAM_TIME	0.7f

#define MAX_TILE_VALUE	4098	//Above this, the game would crash, so cap it here

//Defined by SDL
#define JOY_AXIS_MIN	-32768
#define JOY_AXIS_MAX	32767
#define JOY_MINMOVE_TRIP	3000

const float soundFreqDefault = 44100.0;

#define INTRO_FADEIN_TIME	2.0	//How long the intro fadein takes
#define INTRO_FADEIN_DELAY	2.0	//How long before the intro fadein starts

class ColorPhase
{
public:
	Color* colorToChange;
	bool pingpong;
	float32 destr, destg, destb;
	float32 srcr, srcg, srcb;
	float32 amtr, amtg, amtb;
	bool dir;
};

class TilePiece
{
public:
	physSegment* seg;
	physSegment* bg;
	int 	value;		//The actual value of the piece (2, 4, 8, etc.)
	
	//Animation stuff
	Point	drawSlide;	//Used for the slide animation
	Point	drawSize;	//Used for appearing-tile animation
	int destx, desty;	//Destination piece we're trying to slide into
	Color origCol;
	int iAnimDir;
	bool joined;
	
	TilePiece();
	~TilePiece();
	
	//Helper functions (Defined in board.cpp)
	void draw();
};

typedef enum 
{
	LEFT,
	RIGHT,
	UP,
	DOWN	
} direction;

typedef enum
{
	PLAYING,
	GAMEOVER,
	INTRO,
} gameMode;

class Pony48Engine : public Engine
{
	friend class PonyLua;
private:
	//Important general-purpose game variables
	ttvfs::VFSHelper vfs;
	Vec3 CameraPos;
	HUD* m_hud;
	bool m_bMouseGrabOnWindowRegain;
	float32 m_fDefCameraZ;	//Default position of camera on z axis
	list<ColorPhase> m_ColorsChanging;
	SDL_Joystick *m_joy;
	SDL_Haptic* m_rumble;
	map<string, Cursor*> m_mCursors;

	//Webcam stuffs!
	Webcam* m_cam;
	float32 m_fGameoverWebcamFreeze;
	float32 m_fWebcamDrawSize;
	Point m_ptWebcamDrawPos;
	bool m_bDrawFacecam;
	bool m_bSavedFacepic;
	
	//Game stuff!
	LuaInterface* Lua;
	Color m_BoardBg;
	Color m_TileBg[BOARD_WIDTH][BOARD_HEIGHT];
	Color m_BgCol;
	TilePiece* m_Board[BOARD_WIDTH][BOARD_HEIGHT];
	list<TilePiece*> m_lSlideJoinAnimations;
	Vec3 m_BoardRot;
	float32 m_BoardRotAngle;
	uint32_t m_iScore;
	uint32_t m_iHighScore;
	Background* m_bg;
	gameMode m_iCurMode;
	float m_fGameoverKeyDelay;
	bool bJoyVerticalMove, bJoyHorizontalMove;
	float m_fLastFrame;	//For fps counter
	int m_iCAM_FRAME_SKIP;
	int m_iCAM;
	int m_iCurCamFrameSkip;
	SDL_JoystickID m_lastJoyHatMoved;	//Keep track of which joystick hat we're moving
	
	//audio.cpp stuff!
	string sLuaUpdateFunc;
	float32 beatThresholdVolume;	//The threshold over which to recognize a beat
	uint32_t beatThresholdBar;		//The bar in the volume distribution to examine
	float32 beatMul;				//The multiplication factor to move the camera
	float32 maxCamz;				//The maximum value for the camera's z axis
	float32 m_fCamBounceBack;
	map<string, ParticleSystem*> songParticles;
	
	//Keybinding stuff!
	uint32_t JOY_BUTTON_BACK;
	uint32_t JOY_BUTTON_RESTART;
	uint32_t JOY_AXIS_HORIZ;
	uint32_t JOY_AXIS_VERT;
	uint32_t JOY_AXIS2_HORIZ;
	uint32_t JOY_AXIS2_VERT;
	uint32_t JOY_AXIS_LT;
	uint32_t JOY_AXIS_RT;
	int32_t JOY_AXIS_TRIP;
	
	//Random gameover stuff!
	TilePiece* m_highestTile;
	float32 m_gameoverTileRot;
	float32 m_gameoverTileVel;
	float32 m_gameoverTileAccel;

protected:
	void frame(float32 dt);
	void draw();
	void init(list<commandlineArg> sArgs);
	void handleEvent(SDL_Event event);
	void pause();
	void resume();

public:
	//Pony48.cpp functions - fairly generic 
	Pony48Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sAppName, string sIcon, bool bResizable = false);
	~Pony48Engine();
	
	void setLua(LuaInterface* l)	{Lua = l;};
	
	bool _shouldSelect(b2Fixture* fix);

	void hudSignalHandler(string sSignal);	//For handling signals that come from the HUD
	void handleKeys();						//Poll the keyboard state and update the game accordingly
	Point worldPosFromCursor(Point cursorpos);	//Get the worldspace position of the given mouse cursor position
	Point worldMovement(Point cursormove);		//Get the worldspace transform of the given mouse transformation
	
	//Functions dealing with program defaults
	bool loadConfig(string sFilename);
	void saveConfig(string sFilename);
	
	//Other stuff in Pony48.cpp
	obj* objFromXML(string sXMLFilename, Point ptOffset, Point ptVel = Point(0,0));
	Rect getCameraView();		//Return the rectangle, in world position z=0, that the camera can see 
	void changeMode(gameMode gm);	//Change to the specified game mode
	
	//color.cpp functions
	void updateColors(float32 dt);
	void phaseColor(Color* src, Color dest, float time, bool bPingPong = false);
	void clearColors();
	
	//audio.cpp functions
	void beatDetect();					//Bounce to da beat
	void loadSongs(string sFilename);	//Load songs to play into memory
	void loadSongXML(string sFilename);	//Load a song + playback stuff from XML
	void scrubPause();					//Pauses music with a decreasing-frequency effect
	void scrubResume();					//Resumes music with an increasing-frequency effect
	void soundUpdate(float32 dt);		//Updates audio fx
	
	//board.cpp functions
	void updateBoard(float32 dt);			//Update sliding pieces on the board
	void clearBoardAnimations();			//Kill all currently-running animations on the board
	void drawBoard();						//Draw the tiles and such on the board
	TilePiece* loadTile(string sFilename);	//Load a tile piece from an XML file
	void move(direction dir);				//Move in the given direction (if possible)
	bool slide(direction dir);				//Slide pieces this direction (move part 1)
	bool join(direction dir);				//Join pieces that you can in this direction (move part 2)
	bool movePossible(direction dir);		//Test to see if it's possible to move in the given direction
	bool movePossible();					//Test to see if it's possible to move at all
	void placenew();						//Places a new tile at a random location
	void resetBoard();						//Starts a new game
	void clearBoard();						//Clears memory associated with the game board
	void addScore(uint32_t amt);			//Add a value to the score (in function so we can have cool anim stuff)
	void pieceSlid(int startx, int starty, int endx, int endy);	//Called when a piece slides, to update animations
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to the engine
float myAbs(float v);	//Because stinking namespace stuff


#endif
