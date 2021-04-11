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

	Sprite Cursor, Segments[44]; // Load the whale image as array elements.
	Cursor.Init(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 4, 4, "romfs:/shapes/cursor.png");

	for(int i = 0; i < 44; i++) {
		char SegFile[64];
		sprintf(SegFile, "romfs:/shapes/%i.png", i+1);

		Segments[i].Init(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 1254/4, 1694/4, SegFile);
	}

	// Configure rumble
    HidVibrationDeviceHandle VibrationDeviceHandles[2][2];
    Result rc = 0, rc2 = 0;
    u32 target_device=0;

    HidVibrationValue VibrationValue;
    HidVibrationValue VibrationValue_stop;
    HidVibrationValue VibrationValues[2];
	// Configure rumble END


	// SixAxis
    // It's necessary to initialize these separately as they all have different handle values
    HidSixAxisSensorHandle handles[4];
    hidGetSixAxisSensorHandles(&handles[0], 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidGetSixAxisSensorHandles(&handles[1], 1, HidNpadIdType_No1,      HidNpadStyleTag_NpadFullKey);
    hidGetSixAxisSensorHandles(&handles[2], 2, HidNpadIdType_No1,      HidNpadStyleTag_NpadJoyDual);
    hidStartSixAxisSensor(handles[0]);
    hidStartSixAxisSensor(handles[1]);
    hidStartSixAxisSensor(handles[2]);
    hidStartSixAxisSensor(handles[3]);
    // SixAxis END

  	// Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

	// Rumble variables
	// Two VibrationDeviceHandles are returned: first one for left-joycon, second one for right-joycon.
    // Change the total_handles param to 1, and update the hidSendVibrationValues calls, if you only want 1 VibrationDeviceHandle.
    rc = hidInitializeVibrationDevices(VibrationDeviceHandles[0], 2, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);

    // Setup VibrationDeviceHandles for HidNpadIdType_No1 too, since we want to support both HidNpadIdType_Handheld and HidNpadIdType_No1.
    if (R_SUCCEEDED(rc)) rc = hidInitializeVibrationDevices(VibrationDeviceHandles[1], 2, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyDual);
    printf("hidInitializeVibrationDevices() returned: 0x%x\n", rc);

    if (R_SUCCEEDED(rc)) printf("Hold R to vibrate, and press A/B/X/Y while holding R to adjust values.\n");

    VibrationValue.amp_low   = 0.2f;
    VibrationValue.freq_low  = 10.0f;
    VibrationValue.amp_high  = 0.2f;
    VibrationValue.freq_high = 10.0f;

    memset(VibrationValues, 0, sizeof(VibrationValues));

    memset(&VibrationValue_stop, 0, sizeof(HidVibrationValue));
    // Switch like stop behavior with muted band channels and frequencies set to default.
    VibrationValue_stop.freq_low  = 160.0f;
    VibrationValue_stop.freq_high = 320.0f;

	// Rumble END

	// User Variables
	int MotionSensitivity = 150;
	int OrcaMoveSpeed = 400;
	int Flexibility = 1000;
	int WhaleColR = 255, WhaleColG = 255, WhaleColB = 255;
	int BGColR = 255, BGColG = 255, BGColB = 255;

	bool textHidden = false;
	bool isBehind = false;

	varTable variableTable[] =
	{
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

	varTable joyonData[] =
	{
		{"rumble high amp"},
		{"rumble low amp"},
		{"rumble high freq"},
		{"rumble low freq"},
	};
	int joyconselection = 0;

	while (appletMainLoop()) {
		// Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);


        u64 kDown = padGetButtonsDown(&pad);
        u64 kHeld = padGetButtons(&pad);
        u64 kUp = padGetButtonsUp(&pad);

		if (kDown & HidNpadButton_Plus) {
			break;
		}
		else if(kDown & HidNpadButton_StickL) {
			Cursor.posX = SCREEN_WIDTH/2;
			Cursor.posY = SCREEN_HEIGHT/2;
		}
		else if(kDown & HidNpadButton_StickR) {
			isBehind = !isBehind;
		}
		else if(kDown & HidNpadButton_Minus) {
			textHidden = !textHidden;
		}

		else if((kDown & HidNpadButton_StickLDown) && varSelection < (int)(sizeof(variableTable)/sizeof(varTable)) - 1) {
			varSelection++;
		}
		else if((kDown & HidNpadButton_StickLUp) && varSelection > 0) {
			varSelection--;
		}
		else if(kDown & HidNpadButton_StickLRight) {
			if(varSelection == 0) MotionSensitivity += 10;
			else if(varSelection == 1) OrcaMoveSpeed += 10;
			else if(varSelection == 2) Flexibility += 10;
		}
		else if(kHeld & HidNpadButton_StickLRight) {
			if(varSelection == 3 && WhaleColR < 255) WhaleColR++;
			else if(varSelection == 4 && WhaleColG < 255) WhaleColG++;
			else if(varSelection == 5 && WhaleColB < 255) WhaleColB++;
			else if(varSelection == 6 && BGColR < 255) BGColR++;
			else if(varSelection == 7 && BGColG < 255) BGColG++;
			else if(varSelection == 8 && BGColB < 255) BGColB++;
		}
		else if(kDown & HidNpadButton_StickLLeft) {
			if(varSelection == 0) MotionSensitivity -= 10;
			else if(varSelection == 1) OrcaMoveSpeed -= 10;
			else if(varSelection == 2) Flexibility -= 10;
		}
		else if(kHeld & HidNpadButton_StickLLeft) {
			if(varSelection == 3 && WhaleColR > 0) WhaleColR--;
			else if(varSelection == 4 && WhaleColG > 0) WhaleColG--;
			else if(varSelection == 5 && WhaleColB > 0) WhaleColB--;
			else if(varSelection == 6 && BGColR > 0) BGColR--;
			else if(varSelection == 7 && BGColG > 0) BGColG--;
			else if(varSelection == 8 && BGColB > 0) BGColB--;
		}

		// Rumble activation
		//	Select which devices to vibrate.
        target_device = padIsHandheld(&pad) ? 0 : 1;

		// HD rumble need a proper implementation
	if (R_SUCCEEDED(rc) && (kHeld & HidNpadButton_ZR))
        {
           //Calling hidSendVibrationValue/hidSendVibrationValues is really only needed when sending new VibrationValue(s).
            //If you just want to vibrate 1 device, you can also use hidSendVibrationValue.

            //~ memcpy(&VibrationValues[0], &VibrationValue, sizeof(HidVibrationValue));
            //~ memcpy(&VibrationValues[1], &VibrationValue, sizeof(HidVibrationValue));

            //~ rc2 = hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);
            //~ if (R_FAILED(rc2)) printf("hidSendVibrationValues() returned: 0x%x\n", rc2);

            if (kDown & HidNpadButton_A) VibrationValue.amp_low   += 0.1f;
            if (kDown & HidNpadButton_B) VibrationValue.freq_low  += 5.0f;
            if (kDown & HidNpadButton_X) VibrationValue.amp_high  += 0.1f;
            if (kDown & HidNpadButton_Y) VibrationValue.freq_high += 12.0f;
        }
        else if(kUp & HidNpadButton_ZR)//Stop vibration for all devices.
        {
            memcpy(&VibrationValues[0], &VibrationValue_stop, sizeof(HidVibrationValue));
            memcpy(&VibrationValues[1], &VibrationValue_stop, sizeof(HidVibrationValue));

            rc2 = hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);
            if (R_FAILED(rc2)) printf("hidSendVibrationValues() for stop returned: 0x%x\n", rc2);

            //Could also do this with 1 hidSendVibrationValues() call + a larger VibrationValues array.
            rc2 = hidSendVibrationValues(VibrationDeviceHandles[1-target_device], VibrationValues, 2);
            if (R_FAILED(rc2)) printf("hidSendVibrationValues() for stop other device returned: 0x%x\n", rc2);
        }
        else if (kHeld & HidNpadButton_ZL)
		{
			memcpy(&VibrationValues[0], &VibrationValue, sizeof(HidVibrationValue));
            memcpy(&VibrationValues[1], &VibrationValue, sizeof(HidVibrationValue));
            rc2 = hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);
            if (kDown & HidNpadButton_A) VibrationValue.amp_low   -= 0.1f;
            if (kDown & HidNpadButton_B) VibrationValue.freq_low  -= 5.0f;
            if (kDown & HidNpadButton_X) VibrationValue.amp_high  -= 0.1f;
            if (kDown & HidNpadButton_Y) VibrationValue.freq_high -= 12.0f;
		}


		// Rumble activation END


		// Get gyro data bason on controller connection type
		HidSixAxisSensorState sixaxis = {0};
        u64 style_set = padGetStyleSet(&pad);
		if (style_set & HidNpadStyleTag_NpadHandheld)
					hidGetSixAxisSensorStates(handles[0], &sixaxis, 1);
		else if (style_set & HidNpadStyleTag_NpadFullKey)
			hidGetSixAxisSensorStates(handles[1], &sixaxis, 1);
		else if (style_set & HidNpadStyleTag_NpadJoyDual)
		{
			// For JoyDual, read from either the Left or Right Joy-Con depending on which is/are connected
			u64 attrib = padGetAttributes(&pad);
			if (attrib & HidNpadAttribute_IsLeftConnected)
				hidGetSixAxisSensorStates(handles[2], &sixaxis, 1);
			else if (attrib & HidNpadAttribute_IsRightConnected)
				hidGetSixAxisSensorStates(handles[3], &sixaxis, 1);
		}
		// Gyro Read END


	//	Touchscreen

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
	//	Move the curson accordingly
		Cursor.posX -= sixaxis.angular_velocity.z * MotionSensitivity;
		Cursor.posY -= sixaxis.angular_velocity.x * MotionSensitivity;
	//	Touchscreen end




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
			Text.DrawText(10, 10, 0.50f, "Killer Whale Cursor for Switch v1.1");
			//~ Text.DrawText(10, 50, 0.65f, "Use Joy-Con R or touchscreen to move the whale");
			//~ Text.DrawText(10, 90, 0.65f, "[Y] Reset cursor position");
			//~ Text.DrawText(10, 120, 0.65f, "[X] Toggle between front/back view");
			//~ Text.DrawText(10, 150, 0.65f, "[B] Show/hide on-screen text");
			//~ Text.DrawText(10, 195, 0.65f, "Use D-pad/sticks to change values");

			for(int i = 0; i < (int)(sizeof(variableTable)/sizeof(varTable)); i++) {
				if(varSelection == i) {
					Text.SetColor(1.0f, 0.0f, 0.0f);
				}
				Text.DrawText(10, (i*30)+50, 0.65f, variableTable[i].name, *variableTable[i].var);
				Text.SetColor(0.0f, 0.0f, 0.0f);
			}
			Text.SetColor(0.120f, 0.405f, 0.503f);
			int textYpos = 400;
			Text.DrawText(10, textYpos, 0.65f, "Acceleration: x=[%.4f], y=[%.4f], z=[%.4f]",MotionSensitivity*sixaxis.acceleration.x, MotionSensitivity*sixaxis.acceleration.y, MotionSensitivity*sixaxis.acceleration.z);
			Text.DrawText(10, textYpos+25, 0.65f, "Angular velocity: x=[%.4f], y=[%.4f], z=[%.4f]", MotionSensitivity*sixaxis.angular_velocity.x, MotionSensitivity*sixaxis.angular_velocity.y, MotionSensitivity*sixaxis.angular_velocity.z);
			Text.DrawText(10, textYpos+50, 0.65f, "Angle: x=[%.4f], y=[%.4f], z=[%.4f]", MotionSensitivity*sixaxis.angle.x, MotionSensitivity*sixaxis.angle.y, MotionSensitivity*sixaxis.angle.z);
			Text.DrawText(10, textYpos+75, 0.65f, "Rumble High Freq: %.2f",VibrationValue.freq_high);
			Text.DrawText(10, textYpos+100, 0.65f, "Rumble Low Freq: %.2f",VibrationValue.freq_low);
			Text.DrawText(10, textYpos+125, 0.65f, "Rumble High Amp: %.2f",VibrationValue.amp_high);
			Text.DrawText(10, textYpos+150, 0.65f, "Rumble Low Amp: %.2f",VibrationValue.amp_low);


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

		if (MotionSensitivity*sixaxis.angular_velocity.x >.5)
		{
			memcpy(&VibrationValues[0], &VibrationValue, sizeof(HidVibrationValue));
            memcpy(&VibrationValues[1], &VibrationValue, sizeof(HidVibrationValue));
            rc2 = hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);
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
