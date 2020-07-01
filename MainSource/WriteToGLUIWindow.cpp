/*
 *  WriteToGLUIWindow.cpp
 *  FireBall
 *
 *  Created by Dale Ranta on 8/28/12.
 *  Copyright 2012 SAIC. All rights reserved.
 *
 */

#include "firstFile.h"
#include "DialogSave.h"
#include "mThread.h"
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
/* #define EXTERN  */

#include "BackgroundQ.h"
#include "LoadFiles.h"
#include "CWindow.h"

#include <GL/glui.h>

extern "C" int WriteToGLUIWindow(char *message);

static int gluiID = -1;

static void textbox_cb(GLUI_Control *control) {
    //printf("Got textbox callback\n");
}

static void menu_select(int item);

int doFrequnecyFile(char *name);

static volatile int scanFlag=0;

static int insert=0;

int rxScan(void *rxv);

static double lineTime=0;


class GLUIAPI GLUI_TextBox2 : public GLUI_TextBox
{
public:
    virtual int mouse_down_handler( int local_x, int local_y );
    int  key_handler( unsigned char key,int modifiers );
    virtual int special_handler( int key, int modifiers);
    GLUI_TextBox2( GLUI_Node *parent, bool scroll = false,
                  int id=-1, GLUI_CB cb=GLUI_CB() );
    
};

static GLUI_TextBox2 *moo;

int GLUI_TextBox2::key_handler(unsigned char key, int modifiers)
{
  //  fprintf(stderr,"key %d\n",key);
    
    const char *test=moo->get_text();

    if(key == 6){
        
        menu_select(31);
        
        int ns=moo->sel_end+1;
        
        if(ns >= (int)(strlen(test)-1)){
            moo->update_and_draw_text();
            return false;
        }

        moo->sel_start=moo->sel_end+1;
        
        for(int n=moo->sel_end+1;n<(int)strlen(test);++n){
            if(test[n] == '\n'){
                moo->sel_end=n;
                break;
            }
        }
        
        moo->insertion_pt=moo->sel_end+1;
        moo->update_and_draw_text();

        return false;
    }else if(key == 2){

        int ns=moo->sel_start-2;
        
        if(ns < 0){
            ns=moo->sel_start=0;
            moo->sel_end=(int)strlen(test);
            for(int n=1;n<(int)strlen(test);++n){
                if(test[n] == '\n'){
                    moo->sel_end=n;
                    break;
                }
            }

        }else{
            moo->sel_end=moo->sel_start-1;
            
            moo->sel_start=0;
            
            for(int n=ns;n>0;--n){
                if(test[n] == '\n'){
                    moo->sel_start=n+1;
                    break;
                }
            }
        }
 
        menu_select(31);
        
        moo->update_and_draw_text();

        return false;
    }
    GLUI_TextBox::key_handler( key, modifiers);

    return false;

}
int GLUI_TextBox2::special_handler( int key, int modifiers)
{
//     fprintf(stderr,"key %d\n",key);
    
    if(key == 114)return false;
    
//    int ptsave=moo->insertion_pt;
    
    GLUI_TextBox::special_handler( key, modifiers);
    
    return false;
}
int GLUI_TextBox2::mouse_down_handler( int local_x, int local_y)
{
    
    GLUI_TextBox::mouse_down_handler( local_x, local_y);
    
   // fprintf(stderr,"a mouse_down_handler insertion_pt %d\n",insertion_pt);
    
    insert=insertion_pt;

    return false;
    
}

GLUI_TextBox2::GLUI_TextBox2( GLUI_Node *parent,
                             bool scroll, int id,
                             GLUI_CB cb) : GLUI_TextBox(parent,scroll,id,cb)
{
    ;
}


int doFrequnecyFile(char *path)
{
    char buff[5120],word[20000];
    FILE *inout;
    int n,m,k,itWas,c;

    if(!path)return 1;
    
    if((inout=fopen(path,"rb")) == NULL){
        sprintf(buff,"doFrequnecyFile Cannot open file : %s to read%c\n",path,0);
        fprintf(stderr,"%s",buff);
        return 1;
    }

    itWas = -7777;
    k=0;
    while((m=(int)fread(buff,1,5120,inout)) > 0){
        for(n=0;n<m;++n){
            c=buff[n];
            if(c == '\n' || c == '\r' || (k >= 19998)){
                if((c == '\n') && (itWas == '\r')){
                    continue;
                }
                word[k++]='\n';
                word[k++]='\0';
                WriteToGLUIWindow(word);
                k=0;
            }else{
                word[k++]=buff[n];
            }
            itWas = c;
        }
    }
    if(k != 0){
        word[k++]='\n';
        word[k++]='\0';
        WriteToGLUIWindow(word);
    }

    if(inout)fclose(inout);
    
    return 0;
}
static void SaveIt(struct Scene *scene,char *name)
{
	FILE *out;
	GLUI *glui;
	
	glui = GLUI_Master.find_glui_by_window_id(gluiID);
	
	if(glui){
		if(moo){
			const char *text=moo->get_text();
			if(text){
				out=fopen(name,"w");
				if(out){
					fprintf(out,"%s",text);
					fclose(out);
				}
			}
		}
	}
}

int rxScan(void *rxv)
{
     int n;
    
    const char *test=moo->get_text();
    
    char *buff = new char[strlen(test)+10];
    
    int start=moo->sel_start;
    int end=moo->sel_end;
   // fprintf(stderr,"Start Scan start %d end %d\n",start,end);
    if(start > end){
        n=start;
        start=end;
        end=n;
    }
    n=0;
    for(int k=start;k<end;++k){
        if(test[k] == ',' || test[k] == 10){
            buff[n++]=0;
        }else{
            buff[n++]=test[k];
        }
    }
    buff[n++]=0;

    sendMessageGlobal((char *)"0",(char *)"0",M_FREQUENCY_SCAN);
    
    int n1=-1;
    int n2=-1;
    for(int k=0;k<n;++k){
      if(n1 == -1 && buff[k] == 0){
        n1=k+1;
      }else if(n1 > -1 && n2 == -1 && buff[k] == 0){
          n2=k+1;
         // fprintf(stderr,"buff1 %s buff2 %s\n",&buff[n1],&buff[n2]);
          sendMessageGlobal(&buff[n1],&buff[n2],M_FREQUENCY_SCAN);
          for(int i=n2;i<n;++i){
              if(buff[i] == 0){
                  k=i;
                  break;
              }
          }
          n1=-1;
          n2=-1;
      }
    }
    sendMessageGlobal((char *)"-1",(char *)"-1",M_FREQUENCY_SCAN);

    return 1;
}
static void menu_select(int item)
{
    static char buff[4096];
	GLUI *glui;
    int n;
    
    
    //fprintf(stderr,"menu_select window %d\n",glutGetWindow());

    if(item == 400){
        scanFlag=1;
        launchThread((void *)moo,rxScan);
    }else if(item == 401){
        scanFlag=0;
    }else if(item == 32){
		dialogSaveC(NULL,SaveIt,3,NULL);
		return;
    }else if(item == 35){
        const char *test=moo->get_text();
        int start=moo->sel_start;
        int end=moo->sel_end;
        if(start > end){
            n=start;
            start=end;
            end=n;
        }
        n=0;
        for(int k=start;k<end;++k){
            buff[n++]=test[k];
            if(n > 4094)break;
        }
        buff[n++]=0;
    }else if(item == 36){
        moo->text.insert(moo->insertion_pt,buff);
        moo->insertion_pt += (int)strlen(buff);
      //  fprintf(stderr,"point %d buff %s\n",moo->insertion_pt,buff);
    }else if(item == 33){
        glui = GLUI_Master.find_glui_by_window_id(gluiID);

		if(glui){
			glui->close();
		}
		
		moo=NULL;
		
		gluiID = -1;
    }else if(item == 31){
        int n;
        
        const char *test=moo->get_text();
        
        int start=moo->sel_start;
        int end=moo->sel_end;
        if(start > end){
            n=start;
            start=end;
            end=n;
        }
        n=0;
        for(int k=start;k<end;++k){
            buff[n++]=test[k];
            if(n > 254)break;
        }
        buff[n++]=0;
        int n1=-1;
        int n2=-1;
        for(int k=0;k<n;++k){
            if(n1 == -1 && buff[k] == ','){
                n1=k+1;
            }else if(n1 > -1 && n2 == -1 && buff[k] == ','){
                buff[k]=0;
                n2=k+1;
            } else if(buff[k] == ','){
                buff[k]=0;
            }
          
        }
        
        if(n1 == -1 || n2 == -1)return;

       // fprintf(stderr,"%d %d buff %s\n",moo->sel_start,moo->sel_end,buff);
        
        sendMessageGlobal(&buff[n1],&buff[n2],M_SEND);
        
    }
}
static void iddle(void){
    static int nn=0;
    if(rtime() < lineTime)return;
    lineTime = rtime()+5;
    int jj = nn % 4;
    fprintf(stderr,"nn %d jj %d\n",nn++,jj);
    if(jj == 0){
        sendMessageGlobal((char *)"101.5",(char *)"FM",M_SEND);
    }else if(jj == 1){
        sendMessageGlobal((char *)"100.7",(char *)"FM",M_SEND);
    }else if(jj == 2){
        sendMessageGlobal((char *)"102.9",(char *)"FM",M_SEND);
    }else if(jj == 3){
        sendMessageGlobal((char *)"105.3",(char *)"FM",M_SEND);
    }
    return;
}
int WriteToGLUIWindow(char *message)
{
	GLUI *glui;

    static int window=-1;
    
	if(!message)return 1;
	
	glui = GLUI_Master.find_glui_by_window_id(gluiID);
	
	if(!glui){
        glui = GLUI_Master.create_glui("BatchPrint", 0);
        if(!glui)return 1;
        gluiID=glui->get_glut_window_id();
        glui->set_main_gfx_window(glutGetWindow());
        GLUI_Panel *ep = new GLUI_Panel(glui,"",true);
        moo = new GLUI_TextBox2(ep,true,1,textbox_cb);
        moo->set_h(400);
        moo->set_w(610);
        
        // GLUI_Master.set_glutIdleFunc(iddle);

        GLUI_Panel *panel3 = new GLUI_Panel(glui, "Scan Frequencies");
        new GLUI_Button(panel3, "Stop", 401, menu_select);

        window=glutGetWindow();
        
        glutSetWindow(gluiID);

		glutCreateMenu(menu_select);
		
        glutAddMenuEntry("Set Frequency", 31);
        glutAddMenuEntry("Send Scan Frequencies", 400);
        glutAddMenuEntry("Copy", 35);
        glutAddMenuEntry("Paste", 36);
        glutAddMenuEntry("Save", 32);
        glutAddMenuEntry("-------------", 34);
        glutAddMenuEntry("Close", 33);

		glutAttachMenu(GLUT_RIGHT_BUTTON);

		glutSetWindow(window);
        
        moo->insertion_pt = -2;
        
	}
        if(moo){
            if(moo->insertion_pt == -2){
                moo->insertion_pt = -1;
                moo->text=message;
                insert=(int)strlen(message);
                moo->insertion_pt += (int)strlen(message)+1;
            }else{
                if(moo->insertion_pt == -1){
                    moo->insertion_pt=insert;
               }
                moo->text.insert(moo->insertion_pt,message);
                moo->insertion_pt += (int)strlen(message);
                insert=moo->insertion_pt;
                moo->redraw_window();
            }
        }
  
	return 0;
}

