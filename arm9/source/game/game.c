#include "game/game_main.h"

bool currentBuffer;
int mainBG;
u16 mainScreen[256*192];

bool testStepByStep=false;

PrintConsole bottomScreen;

// extern md2Model_struct storageCubeModel, companionCubeModel, cubeDispenserModel; //TEMP
cubeDispenser_struct* testDispenser;
bigButton_struct* testButton;
bigButton_struct* testButton2;
platform_struct* testPlatform;

u16 vblCNT, frmCNT, FPS;

void initGame(void)
{
	int oldv=getMemFree();
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("initializing...");
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_0_2D);
	
	glInit();
	
	vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_LCD,VRAM_D_MAIN_BG_0x06000000);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_SUB_BG_0x06208000);
	
	glEnable(GL_TEXTURE_2D);
	// glEnable(GL_ANTIALIAS);
	glDisable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);
	
	glSetOutlineColor(0,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(7,RGB15(31,0,0)); //TEMP?
	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?
	
	glClearColor(31,31,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0,0,255,191);
	
	// initVramBanks(1);
	initVramBanks(2);
	initTextures();
	initSound();
	
	initCamera(NULL);
	
	initPlayer(NULL);
	
	initLights();
	initParticles();
	
	initMaterials();
	
	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");
	
	initElevators();
	initWallDoors();
	initTurrets();
	initBigButtons();
	initTimedButtons();
	initEnergyBalls();
	initPlatforms();
	initCubes();
	initEmancipation();
	initDoors();
	initSludge();

	NOGBA("lalala");

	getPlayer()->currentRoom=&gameRoom;
	
	currentBuffer=false;
	
	getVramStatus();
	fadeIn();
	
	mainBG=bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(mainBG, 0);
	REG_BG0CNT=BG_PRIORITY(3);
	
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 16, 0, false, true);
	consoleSelect(&bottomScreen);
	
	// glSetToonTableRange(0, 14, RGB15(16,16,16));
	// glSetToonTableRange(15, 31, RGB15(26,26,26));
	
	initPortals();
	
	//PHYSICS
	initPI9();

	// readMap("lalala.map", NULL);
	// newReadMap("../testedit.map", NULL, 255);
	newReadMap("test.map", NULL, 255);
	// newReadMap("default.map", NULL, 255);
	
	// testButton=createBigButton(NULL, vect(10,0,10)); //TEMP
	// testButton2=createBigButton(NULL, vect(6,0,4)); //TEMP
	// testDispenser=createCubeDispenser(NULL, vect(4,0,4), true); //TEMP
	// createEnergyDevice(NULL, vect(0,7,9), pX, true); //TEMP
	// createEnergyDevice(NULL, vect(20,0,9), pY, false); //TEMP
	// createPlatform(vect(TILESIZE*2,TILESIZE,TILESIZE*4),vect(TILESIZE*10,TILESIZE,TILESIZE*4), true); //TEMP
	// testPlatform=createPlatform(vect(-TILESIZE*2,TILESIZE,TILESIZE*4),vect(-TILESIZE*2,TILESIZE*4,TILESIZE*4), true); //TEMP
	// addActivatorTarget(&testButton2->activator,(void*)testDispenser,DISPENSER_TARGET); //TEMP
	// addActivatorTarget(&testButton->activator,(void*)testPlatform,PLATFORM_TARGET); //TEMP
	// createEmancipationGrid(NULL,vect(0,0,7),TILESIZE*8,false);
	
	transferRectangles(&gameRoom);
	makeGrid();
	generateRoomGrid(&gameRoom);
	gameRoom.displayList=generateRoomDisplayList(&gameRoom, vect(0,0,0), vect(0,0,0), false);
	
	getVramStatus();
	
	startPI();
	
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("vs mem free : %dko (%do)",oldv/1024,oldv);
}

bool testbool=false;
bool switchPortal=false;
portal_struct *currentPortal, *previousPortal;

void postProcess(u16* scrP, u32* stackP);
bool orangeSeen, blueSeen;
extern u16** stackEnd;
u16* ppStack[192*16];

u32 prevTiming;

u32 cpuEndSlice()
{
	u32 temp=prevTiming;
	prevTiming=cpuGetTiming();
	return prevTiming-temp;
}

static inline void render1(void)
{
	scanKeys();
	
	// cpuEndSlice();
	playerControls(NULL);
	// iprintf("controls : %d  \n",cpuEndSlice());
	
		updatePlayer(NULL);
	// iprintf("player : %d  \n",cpuEndSlice());
	
		updatePortals();
		updateTurrets();
		updateBigButtons();
		updateTimedButtons();
		updateEnergyDevices();
		updateEnergyBalls();
		updatePlatforms();
		updateCubeDispensers();
		updateEmancipators();
		updateEmancipationGrids();
		updateDoors();
		updateWallDoors();
	// iprintf("updates : %d  \n",cpuEndSlice());
	
	// if(currentPortal)GFX_CLEAR_COLOR=currentPortal->color|(31<<16);
	// else GFX_CLEAR_COLOR=0;
	u16 color=getCurrentPortalColor(getPlayer()->object->position);
	// NOGBA("col %d",color);
	// GFX_CLEAR_COLOR=color|(31<<16);
	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);
	
		if(fifoCheckValue32(FIFO_USER_08))iprintf("\x1b[0J");
		while(fifoCheckValue32(FIFO_USER_08)){int32 cnt=fifoGetValue32(FIFO_USER_08);iprintf("ALERT %d      \n",cnt);NOGBA("ALERT %d      \n",cnt);}
	
	projectCamera(NULL);

	glPushMatrix();
		
		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);
		
		renderGun(NULL);
		
		transformCamera(NULL);
		
		cpuEndSlice();
			// drawRoomsGame(128, color);
			drawRoomsGame(0, color);
			// drawCell(getCurrentCell(getPlayer()->currentRoom,getPlayerCamera()->position));
		// iprintf("room : %d  \n",cpuEndSlice());
		
		updateParticles();
		drawParticles();
		// iprintf("particles : %d  \n",cpuEndSlice());
		
			drawOBBs();
		// iprintf("OBBs : %d  \n",cpuEndSlice());
			drawBigButtons();
			drawTimedButtons();
			drawEnergyDevices();
			drawEnergyBalls();
			drawPlatforms();
			drawCubeDispensers();
			drawTurretsStuff();
			drawEmancipators();
			drawEmancipationGrids();
			drawDoors();
			drawWallDoors();
			drawSludge(&gameRoom);
		// iprintf("stuff : %d  \n",cpuEndSlice());
		
		drawPortal(&portal1);
		drawPortal(&portal2);
			
	glPopMatrix(1);
	
	glFlush(0);
}

static inline void render2(void)
{
	if(!(orangeSeen||blueSeen)){previousPortal=NULL;return;}
	if((orangeSeen&&blueSeen)/*||(!orangeSeen&&!blueSeen)*/)
	{
		if(switchPortal)currentPortal=&portal1;
		else currentPortal=&portal2;
	}else if(orangeSeen)currentPortal=&portal1;
	else if(blueSeen)currentPortal=&portal2;
	
	previousPortal=currentPortal;
	
	switchPortal^=1;
	
	glClearColor(0,0,0,31);
	
	updatePortalCamera(currentPortal, NULL);
	projectCamera(&currentPortal->camera);

	glPushMatrix();
		
		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);
		
		renderGun(NULL);
		
		transformCamera(&currentPortal->camera);
		
		// drawRoomsGame(0);
		drawPortalRoom(currentPortal);
		
		drawPlayer(NULL);
		
		drawOBBs();
		drawBigButtons();
		drawTimedButtons();
		drawEnergyDevices();
		drawEnergyBalls();
		drawPlatforms();
		drawCubeDispensers();
		drawTurretsStuff();
		drawEmancipators();
		drawEmancipationGrids();
		drawDoors();
		drawWallDoors();
		drawSludge(&gameRoom);

	glPopMatrix(1);
	
	glFlush(0);
}

static inline void postProcess1(void)
{
	postProcess(mainScreen,ppStack);
}

static inline void postProcess2(void)
{
	u16* p1=mainScreen;
	u16* p2=portal1.viewPoint;
	u16* p3=portal2.viewPoint;

	u16** stp;
	blueSeen=orangeSeen=false;
	int oval=0;
	for(stp=ppStack;stp<stackEnd;stp++)
	{
		int val=(int)((*stp)-p1);
		if(val>256*192*2){blueSeen=true;val-=256*192*2;if(p1[oval-1]==65504)oval--;if(p1[val]==65504)val++;if(val-oval>0)dmaCopy(&(p3[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		else if(val>256*192){orangeSeen=true;val-=256*192;if(p1[oval-1]==33791)oval--;if(p1[val]==33791)val++;if(val-oval>0)dmaCopy(&(p2[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		else {if(val-oval>0)dmaCopy(&(p1[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		oval=val;
	}
}

u32 debugVal; //TEMP

void gameFrame(void)
{
	int lala;
	switch(currentBuffer)
	{
		case false:
			iprintf("\x1b[0;0H");
			iprintf("%d FPS   \n", FPS);
			iprintf("%d (debug)   \n", debugVal);
			iprintf("%d (free ram)   \n", getMemFree()/1024);
			iprintf("%p (portal)   \n", portal1.displayList);
			iprintf("%p (portal)   \n", portal2.displayList);
			cpuEndSlice();
			postProcess1();
			// iprintf("postproc : %d  \n",cpuEndSlice());
			render1();

			// if(keysDown()&KEY_SELECT)testStepByStep^=1; //TEMP
			iprintf("full : %d (%d)  \n",cpuEndTiming(),testStepByStep);
			swiWaitForVBlank();
			cpuStartTiming(0);
			prevTiming=0;
			if(previousPortal)dmaCopy(VRAM_C, previousPortal->viewPoint, 256*192*2);			
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			frmCNT++;
			break;
		case true:
			// cpuStartTiming(0);
			postProcess2();
			// iprintf("frm 2 : %d  \n",cpuGetTiming());
			render2();
			listenPI9();
			updateOBBs();
			// iprintf("frm 2 : %d  \n",cpuEndTiming());
			iprintf("fake frame : %d   \n",cpuEndTiming());
			swiWaitForVBlank();
			cpuStartTiming(0);
			prevTiming=0;
			dmaCopy(VRAM_C, mainScreen, 256*192*2);			
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			break;
	}
	
	// if(testStepByStep){int i=0;while(!(keysUp()&KEY_TOUCH)){scanKeys();listenPI9();swiWaitForVBlank();}NOGBA("WAITED");scanKeys();scanKeys();if(keysHeld()&KEY_SELECT)testStepByStep=false;}
	// else if(keysDown()&KEY_SELECT)testStepByStep=true;
	
	currentBuffer^=1;
}

void killGame(void)
{
	fadeOut();
	NOGBA("KILLING IT");
	freePlayer();
	freeEnergyBalls();
	freeBigButtons();
	freeCubes();
	freeDoors();
	freeElevators();
	freeEmancipation();
	freePlatforms();
	freeTimedButtons();
	freeTurrets();
	freeWallDoors();
	freeSludge();
	freeRoom(&gameRoom);
	freePortals();
	freeState(NULL);
	freeSound();

	resetAllPI();

	NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void gameVBL(void)
{
	vblCNT++;
	if(vblCNT>=60)
	{
		FPS=frmCNT;
		frmCNT=vblCNT=0;
	}
}
