//
//  Radio.h
//  SdrGlut
//
//  Created by Dale on 10/8/16.
//  Copyright © 2016 Dale Ranta. All rights reserved.
//

#ifndef RadioRA_h
#define RadioRA_h
#include "RaceFastsdr.h"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include "ulibTypes.h"
#include <GL/glui.h>

#include "smeter.h"
#include "audiolib.h"

#include <cstdlib>
#include <cstddef>
#include <iostream>
//#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sndfile.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#include <time.h>

#include "Utilities.h"

#include "CLines.h"
#include "GridPlot.h"
#include "paletteDraw.h"
#include "DialogSave.h"

struct playData4{
    int samplerate;
    int Debug;
    float gain;
    double fc;
    double f;
    double dt;
    double sino;
    double coso;
    double sindt;
    double cosdt;
    double w;

    int deviceNumber;

    int decodemode;
    
    int size;
    
    int debug;
};


struct WaterFall4{
    unsigned char *data;
    unsigned char ic[2*32768];
    double amin,amax;
    uRect SRect;
    uRect DRect;
    int nline;
    int xsize;
    int ysize;
};

struct DialogRadioData{
    GLUI *glui;
    char text1[255];
    char text2[255];
    char text3[255];
    char text4[255];
    char text5[255];
    char text15[255];
    char text16[255];
    char text17[255];
    char text6[255];
    char text7[255];
    char text8[255];
    
    GLUI_EditText *edittext1;
    GLUI_EditText *edittext2;
    GLUI_EditText *edittext3;
    GLUI_EditText *edittext4;
    GLUI_EditText *edittext5;
    GLUI_EditText *edittext15;
    GLUI_EditText *edittext16;
    GLUI_EditText *edittext17;
    GLUI_EditText *edittext6;
    GLUI_EditText *edittext7;
    GLUI_EditText *edittext8;
    
    char text1z[255];
    GLUI_EditText *edittext1z[20];
    GLUI_Scrollbar *line_scroll[20];
    float line_Index[20];
    float line_Index_old[20];
    int gain_Index;
    int iic;
    int useagc;
    struct Scene *sceneLocal;
    int sub_window;
};

class CLines;

class Radio;

class Radio: public CWindow{
public:
    Radio(struct Scene *scene);
    virtual ~Radio();
    virtual int LoadFile (struct Scene *scene,char *name, int fileType);
    virtual int BackGroundEvents(struct Scene *scene);
    virtual int UpdateTemperatureColors(struct Scene *scene);
    virtual int SetFrequency(struct Scene *scene,double f,double bw,int messageType);
    int SetWindow(struct Scene *scene);
    int mMenuSelectl(struct Scene *scene,int item);
    int KillScene (struct Scene *scene);
    int dialogRadio (struct Scene *scene);
    int setFrequency(struct playData *play);
    int setFrequencyCoefficients(struct playData *rx);
    int stopPlay(struct playData4 *play);
    void playRadio (struct playData4 *play);
    int startRadio (struct playData4 *play);
    int initPlay(struct playData4 *play);
    virtual int FindPoint(struct Scene *scene,int x,int y);
    virtual int OpenWindows(struct Scene *scene);
    void getMouse(int button, int state, int x, int y);
    int Display(struct Scene *scene);
    char ApplicationDirectory[2048];
    char filename[2048];
    int resetDemod();

    volatile int inDialog;
    
    volatile int backGroundEvents;

    struct WaterFall4 water;
    
    struct playData *rx;
    
    struct playData rxs;
    
    struct DialogRadioData dd;

    double real[2*32768*sizeof(double)];
    double imag[2*32768*sizeof(double)];
    long FFTlength;


    
    CLines *lines;
    struct Scene *scenel;
    CLines *lines2;
    struct Scene *scenel2;
    double *range;
    double *dose;
    double *lreal;
    double *limag;
    double lineTime;
    double lineDumpInterval;
    double lineAlpha;
     int updateLine(void);
    
    struct paletteDraw pd;

    int setDialogFrequency(double frequency);
    
    int setDialogFc(double frequency);
    
    int setDialogRange(double pmin,double pmax);
    
    int setDialogPower(double power);
            
    
    double fcdown;
    
    double fdown;
    
    int fcount;

    uRect box;
    
    double power;
    
    int w,h;
    
    int window1;

    int window2;

};

typedef Radio *RadioPtr;

extern RadioPtr FindSceneRadio(int window);

extern RadioPtr FindSdrRadioWindow(int window);

/*
extern int testRadio(struct playData3 *rx,SoapySDR::Kwargs deviceArgs);
*/

#endif /* FMRadio_h */
