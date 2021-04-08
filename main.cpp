#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "graphics.h"
#include "font.h"
#include "sprite.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

struct varTable {
	const char *name;
	int *var;
};

// Lerp/CosLerp helper functions
float Lerp(float a, float b, float t)		{ return a + t * (b - a); }
float CosLerp(float a, float b, float t)	{ return Lerp(a, b, (-cos(3.14f*t) / 2.0f) + 0.5f); }

int main(int argc, char* argv[]) {
	if (!initEgl(nwindowGetDefault())) {
		return EXIT_FAILURE;
	}

	romfsInit();

	gladLoadGL();
	initShaders();

	Font Text("romfs:/fonts/BerlinSans.ttf");
	Text.SetColor(0.0f, 0.0f, 0.0f);

	Sprite Cursor, Segments[44];
	Cursor.Init(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 4, 4, "romfs:/shapes/cursor.png");

	for(int i = 0; i < 44; i++) {
		char SegFile[64];
		sprintf(SegFile, "romfs:/shapes/%i.png", i+1);

		Segments[i].Init(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 1254/4, 1694/4, SegFile);
	}

	// For motion input
	    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    // It's necessary to initialize these separately as they all have different handle values
    HidSixAxisSensorHandle handles[4];
    hidGetSixAxisSensorHandles(&handles[0], 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidGetSixAxisSensorHandles(&handles[1], 1, HidNpadIdType_No1,      HidNpadStyleTag_NpadFullKey);
    hidGetSixAxisSensorHandles(&handles[2], 2, HidNpadIdType_No1,      HidNpadStyleTag_NpadJoyDual);
    hidStartSixAxisSensor(handles[0]);
    hidStartSixAxisSensor(handles[1]);
    hidStartSixAxisSensor(handles[2]);
    hidStartSixAxisSensor(handles[3]);

	// User Variables
	int MotionSensitivity = 150;
	int OrcaMoveSpeed = 400;
	int Flexibility = 1000;
	int WhaleColR = 255, WhaleColG = 255, WhaleColB = 255;
	int BGColR = 255, BGColG = 255, BGColB = 255;

	bool textHidden = false;
	bool isBehind = false;

	varTable variableTable[] = {
		{ "Motion Sensitivity: %i", &MotionSensitivity },
		{ "Whale Move Speed: %i", &OrcaMoveSpeed },
		{ "Whale Flexibility: %i", &Flexibility },
		{ "Whale Color R: %i", &WhaleColR },
		{ "Whale Color G: %i", &WhaleColG },
		{ "Whale Color B: %i", &WhaleColB },
		{ "Background Color R: %i", &BGColR },
		{ "Background Color G: %i", &BGColG },
		{ "Background Color B: %i", &BGColB },
	};

	int varSelection = 0;

	while (appletMainLoop()) {
		// Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

		//hidScanInput();
		u64 kDown = padGetButtonsDown(&pad); // hidKeysDown(CONTROLLER_P1_AUTO);
		u64 kHeld = padGetButtonsUp(&pad); // hidKeysHeld(CONTROLLER_P1_AUTO);
		if (kDown & HidNpadButton_Plus) {
			break;
		}

		else if(kDown & HidNpadButton_Y) {
			Cursor.posX = SCREEN_WIDTH/2;
			Cursor.posY = SCREEN_HEIGHT/2;
		}
		else if(kDown & HidNpadButton_X) {
			isBehind = !isBehind;
		}
		else if(kDown & HidNpadButton_B) {
			textHidden = !textHidden;
		}

		else if((kDown & HidNpadButton_AnyDown) && varSelection < (int)(sizeof(variableTable)/sizeof(varTable)) - 1) {
			varSelection++;
		}
		else if((kDown & HidNpadButton_AnyUp) && varSelection > 0) {
			varSelection--;
		}
		else if(kDown & HidNpadButton_AnyRight) {
			if(varSelection == 0) MotionSensitivity += 10;
			else if(varSelection == 1) OrcaMoveSpeed += 10;
			else if(varSelection == 2) Flexibility += 10;
		}
		else if(kHeld & HidNpadButton_AnyRight) {
			if(varSelection == 3 && WhaleColR < 255) WhaleColR++;
			else if(varSelection == 4 && WhaleColG < 255) WhaleColG++;
			else if(varSelection == 5 && WhaleColB < 255) WhaleColB++;
			else if(varSelection == 6 && BGColR < 255) BGColR++;
			else if(varSelection == 7 && BGColG < 255) BGColG++;
			else if(varSelection == 8 && BGColB < 255) BGColB++;
		}
		else if(kDown & HidNpadButton_AnyLeft) {
			if(varSelection == 0) MotionSensitivity -= 10;
			else if(varSelection == 1) OrcaMoveSpeed -= 10;
			else if(varSelection == 2) Flexibility -= 10;
		}
		else if(kHeld & HidNpadButton_AnyLeft) {
			if(varSelection == 3 && WhaleColR > 0) WhaleColR--;
			else if(varSelection == 4 && WhaleColG > 0) WhaleColG--;
			else if(varSelection == 5 && WhaleColB > 0) WhaleColB--;
			else if(varSelection == 6 && BGColR > 0) BGColR--;
			else if(varSelection == 7 && BGColG > 0) BGColG--;
			else if(varSelection == 8 && BGColB > 0) BGColB--;
		}


		HidSixAxisSensorState sixaxis = {0};
        u64 style_set = padGetStyleSet(&pad);
if (style_set & HidNpadStyleTag_NpadHandheld)
            hidGetSixAxisSensorStates(handles[0], &sixaxis, 1);
        else if (style_set & HidNpadStyleTag_NpadFullKey)
            hidGetSixAxisSensorStates(handles[1], &sixaxis, 1);
        else if (style_set & HidNpadStyleTag_NpadJoyDual) {
            // For JoyDual, read from either the Left or Right Joy-Con depending on which is/are connected
            u64 attrib = padGetAttributes(&pad);
            if (attrib & HidNpadAttribute_IsLeftConnected)
                hidGetSixAxisSensorStates(handles[2], &sixaxis, 1);
            else if (attrib & HidNpadAttribute_IsRightConnected)
                hidGetSixAxisSensorStates(handles[3], &sixaxis, 1);
        }



    hidInitializeTouchScreen();
    s32 prev_touchcount=0;
    HidTouchScreenState state={0};
        if (hidGetTouchScreenStates(&state, 1))
		{
            if (state.count != prev_touchcount)
            {
                prev_touchcount = state.count;
            }

            for(s32 i=0; i<state.count; i++)
            {
                Cursor.posX = state.touches[i].x;
                Cursor.posY = state.touches[i].y;
            }
        }

		Cursor.posX -= sixaxis.angular_velocity.z * MotionSensitivity;
		Cursor.posY -= sixaxis.angular_velocity.x * MotionSensitivity;

		// Cursor boundaries
		if(Cursor.posX < 0) Cursor.posX = 0;
		else if(Cursor.posX > SCREEN_WIDTH) Cursor.posX = SCREEN_WIDTH;
		if(Cursor.posY < 0) Cursor.posY = 0;
		else if(Cursor.posY > SCREEN_HEIGHT) Cursor.posY = SCREEN_HEIGHT;

		for(int i = 0; i < 44; i++) {
			if(i != 0) {
				Segments[i].posX = CosLerp(Segments[i].posX, Segments[i-1].posX, Flexibility);
				Segments[i].posY = CosLerp(Segments[i].posY, Segments[i-1].posY, Flexibility);
			} else {
				Segments[i].posX = CosLerp(Segments[i].posX, Cursor.posX, OrcaMoveSpeed);
				Segments[i].posY = CosLerp(Segments[i].posY, Cursor.posY - 100, OrcaMoveSpeed); // Added 100 to align orca with cursor correctly
			}
		}

		// Render loop begins here
		glClearColor(BGColR/255.0f, BGColG/255.0f, BGColB/255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!textHidden) {
			Text.DrawText(10, 10, 0.75f, "Killer Whale Cursor for Switch v1.0");
			Text.DrawText(10, 50, 0.65f, "Use Joy-Con R or touchscreen to move the whale");
			Text.DrawText(10, 90, 0.65f, "[Y] Reset cursor position");
			Text.DrawText(10, 120, 0.65f, "[X] Toggle between front/back view");
			Text.DrawText(10, 150, 0.65f, "[B] Show/hide on-screen text");
			Text.DrawText(10, 195, 0.65f, "Use D-pad/sticks to change values");

			for(int i = 0; i < (int)(sizeof(variableTable)/sizeof(varTable)); i++) {
				if(varSelection == i) {
					Text.SetColor(1.0f, 0.0f, 0.0f);
				}
				Text.DrawText(10, (i*30)+225, 0.65f, variableTable[i].name, *variableTable[i].var);
				Text.SetColor(0.0f, 0.0f, 0.0f);
			}
		}

		if(!isBehind) {
			for(int i = 0; i < 44; i++) {
				Segments[i].SetColor(WhaleColR/255.0f, WhaleColG/255.0f, WhaleColB/255.0f);
				Segments[i].Draw();
			}
		} else {
			for(int i = 43; i >= 0; i--) {
				Segments[i].SetColor(WhaleColR/255.0f, WhaleColG/255.0f, WhaleColB/255.0f);
				Segments[i].Draw();
			}
		}
		Cursor.Draw();

		SwapBuffers();
	}

	hidStopSixAxisSensor(handles[0]);
	hidStopSixAxisSensor(handles[1]);
	hidStopSixAxisSensor(handles[2]);
	hidStopSixAxisSensor(handles[3]);

	deinitShaders();
	deinitEgl();

	romfsExit();
	return EXIT_SUCCESS;
}
