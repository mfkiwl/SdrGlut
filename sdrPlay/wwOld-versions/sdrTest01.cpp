#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>


#include <liquid/liquid.h>

#include <RtAudio.h>

#include <iostream>
#include <csignal>
#include <mutex>
#include <cstdio>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <sys/time.h>

#include <time.h>


#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/alut.h>
#include <AL/al.h>
#endif

#include "mThread.h"





#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6

/*
#define LIQUID_VERSION_4 1
*/


/*
g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -framework OpenAL

g++ -O2 -std=c++11 -Wno-deprecated -o sdrTest sdrTest.cpp mThread.cpp cMalloc.c -lrtaudio -lSoapySDR -lliquid -lopenal

./sdrTest -fc 1e6 -f 0.6e6 -am -gain 1

./sdrTest -fc 1e6 -f 0.76e6 -am -gain 1

./sdrTest -fc 1e6 -f 1.17e6 -am -gain 1

./sdrTest -fc 10.1e6 -f 10.0e6 -am -gain 1

./sdrTest -fc 27.1e6 -f 27.185e6 -gain 1

./sdrTest -fc 101.1e6 -f 101.5e6 -fm -gain 1

./sdrTest -fc 103.0e6 -f 103.7e6 -fm -gain 1

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1

./sdrTest -fc 162.0e6 -f 162.4e6 -nbfm -gain 1

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 0.9 -faudio 12000 -file test.raw -timeout 10

./sdrTest -fc 101.1e6 -f 101.5e6 -fm -gain 0.9 -faudio 12000 -file test.raw -timeout 10

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 15 -faudio 12000 -file test.raw

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -timeout 601 -dumpbyminute -faudio 12000

./sdrTest -fc 102.0e6 -f 102.1e6 -fm -gain 1 -dumpbyminute -faudio 12000

./sdrTest -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw

./sdrTest -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw -device 1

/opt/local/bin/sox -t raw -r 48000 -b 16 -c 1 -L -e signed-integer test01.raw -n stat

./sdrTest -fc 602e6 -f 602.308400e6 -usb -timeout 10 -file test01.raw -device 1 -PPM -0.380
./sdrTest -fc 602e6 -f 602.308400e6 -usb -timeout 5 -file test01.raw -device 0
/opt/local/bin/sox -t raw -r 48000 -b 16 -c 1 -L -e signed-integer test01.raw -n stat

*/

#define NUM_SOURCES 1

#define NUM_BUFFERS 5

#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10

struct playData{
  //  ALCdevice *dev;
  //  ALCcontext *ctx;
  //  ALuint buffers[NUM_BUFFERS];
    int bufferState[NUM_BUFFERS];
  //  ALuint source;
    int channels;
    int samplerate;
	int Debug;
	double gain;
	double fc;
	double f;
	double dt;
	double sino;
	double coso;
	double sindt;
	double cosdt;
	double w;
	double bw;
	int decodemode;

	SoapySDR::Stream *rxStream;
	SoapySDR::Device *device;
	

	volatile int doWhat;
	volatile int witch;
	
    volatile int audioOut;

    unsigned long MTU;
    
    float faudio;
    
    float Ratio;

    float *buff[NUM_DATA_BUFF];
    int buffStack[NUM_DATA_BUFF];
    int bufftop;
    
    short int *buffa[NUM_ABUFF];
    int buffStacka[NUM_ABUFF];
    int bufftopa;
    
    int size; 
    
    int antennaCount;
    char **antenna;
    char *antennaUse;
    int channel;
    
    unsigned int deviceNumber;
    
    volatile int frame;
    

   	volatile int thread; 

 	int al_state;
 	
 	std::string set[20];
 	std::string value[20];
 	int setcount;
 	
 	double timeout;
 	double timestart;
 	FILE *out;
 	int dumpbyminute;
    int idump;
    
    double aminGlobal;
    double amaxGlobal;
 	
 	double PPM;

};

struct Filters{
	    int np;
    ampmodem demodAM;
	freqdem demod;
	msresamp_crcf iqSampler;
	msresamp_rrrf iqSampler2;
	nco_crcf fShift;
	int thread;	
	double amHistory;
};

	 
	std::mutex mutex;
	
	std::mutex mutexa;
	
	std::mutex mutexo;



//ALvoid DisplayALError(unsigned char *szText, ALint errorCode);

void *cMalloc(unsigned long r, int tag);

char *strsave(char *s,int tag);


int zerol(unsigned char *s,unsigned long n);

static int initPlay(struct playData *rx);

static int stopPlay(struct playData *rx);

int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f);

int doAudio(float *aBuff,struct playData *rx);

int setFilters(struct playData *rx,struct Filters *f);

int findRadio(struct playData *rx);

int playRadio(struct playData *rx);

int rxBuffer(void *rxv);

int Process(void *rxv);

int pushBuff(int nbuffer,struct playData *rx);
int popBuff(struct playData *rx);
int AudioReset(struct playData *rx);

int pushBuffa(int nbuffer,struct playData *rx);
int popBuffa(struct playData *rx);


int setBuffers(struct playData *rx, int numBuff);

int StartIt(struct playData *rx);

int GetTime(long *Seconds,long *milliseconds);

double rtime(void);

int testRadio(struct playData *rx,SoapySDR::Kwargs deviceArgs);

int doAudioOLD2(float *aBuff,struct playData *rx);

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );


int printInfo(void);

volatile int threadexit; 

int audiodevice;

static void list_audio_devices(const ALCchar *devices);
static void list_audio(void);

void signalHandler( int signum ) {
	//fprintf(stderr,"signum %d\n",signum);
	threadexit=1;
}

int main (int argc, char * argv [])
{	
	struct playData rx;

	zerol((unsigned char *)&rx,sizeof(rx));
	
	rx.samplerate=2000000;
	rx.deviceNumber=0;
	audiodevice=0;
	threadexit=0;
	rx.gain=0.5;
	rx.fc=1.0e6;
	rx.f=0.6e6;
    rx.antennaUse=NULL;
    rx.channel=0;
    rx.setcount=0;
    rx.faudio=48000;
    rx.out=NULL;
    rx.timeout=0;
    rx.timestart=0;
    rx.dumpbyminute=0;
    rx.idump=0;
    rx.PPM=0;
    rx.aminGlobal=0;
    rx.amaxGlobal=0;
    rx.decodemode = MODE_AM;

	 	

	signal(SIGINT, signalHandler);  

	for(int n=1;n<argc;++n){
	    if(!strcmp(argv[n],"-debug")){
		   rx.Debug = 1;
	    }else if(!strcmp(argv[n],"-dumpbyminute")){
		   char filename[256];
		   sprintf(filename,"minute-%08d.raw",rx.idump++);
		   fprintf(stderr,"filename %s\n",filename);
		   rx.out=fopen(filename,"wb");
		   if(rx.out == NULL){
				fprintf(stderr,"Could Not Open %s to Write\n",filename);
				exit(1);
		   }
		   rx.dumpbyminute = 1;
	    }else if(!strcmp(argv[n],"-am")){
		   rx.decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-fm")){
		   rx.decodemode = MODE_FM;
	    }else if(!strcmp(argv[n],"-nbfm")){
		   rx.decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            rx.decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            rx.decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	         rx.gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-PPM")){
	         rx.PPM=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         rx.fc=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-f")){
	         rx.f=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-file")){
	         rx.out=fopen(argv[++n],"wb");
	         if(rx.out == NULL){
	             fprintf(stderr,"Could Not Open %s to Write\n",argv[n]);
	         }
	    }else if(!strcmp(argv[n],"-faudio")){
	         rx.faudio=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-device")){
	         rx.deviceNumber=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-audiodevice")){
	         audiodevice=atoi(argv[++n]);
	    }else if(!strcmp(argv[n],"-samplerate")){
            rx.samplerate=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-timeout")){
            rx.timeout=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-antenna")){
            rx.antennaUse=strsave(argv[++n],9870);
	    }else if(!strcmp(argv[n],"-set")){
	     rx.set[rx.setcount]=argv[++n];
	     rx.value[rx.setcount++]=argv[++n];
	    }else{
	    	fprintf(stderr,"Unknown Command = \"%s\"\n",argv[n]);
			// infilename = argv [n] ;
		}
	}
	
	rx.channels=2;
	
    
	initPlay(&rx);
		
	if(findRadio(&rx) || rx.device == NULL){
	    fprintf(stderr,"Error Opening SDR\n");
		return 1;
	}
	
	rx.doWhat = 0;
	
	launchThread((void *)&rx,rxBuffer);   	
	
	RtAudio dac;
	
	if(!rx.out){
	
		int deviceCount=dac.getDeviceCount();
		
	
	
		if (deviceCount < 1 ) {
			fprintf(stderr,"\nNo audio devices found!\n");
			exit( 0 );
		}
	
	
		fprintf(stderr,"\nAudio device Count %d default output device %d audiodevice %d\n",deviceCount,dac.getDefaultOutputDevice(),audiodevice);
	
		RtAudio::DeviceInfo info;
		for (int i=0; i<deviceCount; i++) {
		
			try {
				info=dac.getDeviceInfo(i);
				if(info.outputChannels > 0){
				// Print, for example, the maximum number of output channels for each device
					fprintf(stderr,"audio device = %d : output  channels = %d Device Name = %s",i,info.outputChannels,info.name.c_str());
					if(info.sampleRates.size()){
						fprintf(stderr," sampleRates = ");
						for (int ii = 0; ii < info.sampleRates.size(); ++ii){
							fprintf(stderr," %d ",info.sampleRates[ii]);
					   }
					}
					fprintf(stderr,"\n");
				 }
			 
				if(info.inputChannels > 0){
				// Print, for example, the maximum number of output channels for each device
					fprintf(stderr,"audio device = %d : input   channels = %d Device Name = %s",i,info.inputChannels,info.name.c_str());
					 if(info.sampleRates.size()){
						fprintf(stderr," sampleRates = ");
						for (int ii = 0; ii < info.sampleRates.size(); ++ii){
							fprintf(stderr," %d ",info.sampleRates[ii]);
					   }
					}
					fprintf(stderr,"\n");
			   }

			}
			catch (RtAudioError &error) {
				error.printMessage();
				break;
			}
		
		}
	
		fprintf(stderr,"\n");
	
		list_audio();
	
		printInfo();
	
	
		RtAudio::StreamParameters parameters;
		parameters.deviceId = dac.getDefaultOutputDevice();
		parameters.deviceId = audiodevice;
		parameters.nChannels = 2;
		parameters.nChannels = 1;
		parameters.firstChannel = 0;
		unsigned int bufferFrames = rx.faudio/50; // 256 sample frames


		try {
			dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
							rx.faudio, &bufferFrames, &sound, (void *)&rx );
			dac.startStream();
		}
		catch ( RtAudioError& e ) {
			e.printMessage();
			exit( 0 );
		}
	}	
	
	playRadio(&rx);

	rx.doWhat=1;
	
	Sleep2(100);
		
	stopPlay(&rx);
	
	
	if(!rx.out){
		  try {
			// Stop the stream
			dac.stopStream();
		  }
		  catch (RtAudioError& e) {
			e.printMessage();
		  }
		  if ( dac.isStreamOpen() ) dac.closeStream();
	}
	
    	
	if(rx.out)fclose(rx.out);
	
	return 0 ;
} /* main */

int printInfo(void)
{
    std::cout << "SoapySDR Library" << std::endl;
    std::cout << "Lib Version: v" << SoapySDR::getLibVersion() << std::endl;
    std::cout << "API Version: v" << SoapySDR::getAPIVersion() << std::endl;
    std::cout << "ABI Version: v" << SoapySDR::getABIVersion() << std::endl;
    std::cout << "Install root: " << SoapySDR::getRootPath() << std::endl;
    
    std::vector<std::string> path=SoapySDR::listSearchPaths();
    for(size_t i=0;i<path.size();++i){
        std::cout << "Search path: " << path[i] << std::endl;
    }

    std::vector<std::string> mod=SoapySDR::listModules();
    
    for (size_t k=0;k<mod.size();++k)
    {
        std::cout << "Module found: " << mod[k];
        /*
        const auto &errMsg = SoapySDR::loadModule(mod[k]);
        if (not errMsg.empty()) std::cout << "\n  " << errMsg;
        */
        std::cout << std::endl;
    
    }
    if (mod.empty()) std::cout << "No modules found!" << std::endl;
    
    std::cout << std::endl;

    return 0;
}



int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  short int *buffer = (short int *) outputBuffer;
    
  struct playData *rx=(struct playData *)userData;
  
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
  
	int ibuff;
	ibuff=popBuffa(rx);
	if (ibuff >= 0){

		short int *buff= rx->buffa[ibuff % NUM_ABUFF];
	    
	    int n=0;
	    
		for ( i=0; i<nBufferFrames; i++ ) {
		    short int v=buff[i];
		/*
            if(v > 1.0){
            	v=1.0;
            } else if(v < -1.0){
           		v=-1.0;
            }
        */
 		    //if(v < amin)amin=v;
		    //if(v > amax)amax=v;
			//for ( j=0; j<2; j++ ) {
			  	buffer[n++] = v;
			//}
		}
	}else{
  
	for ( i=0; i<nBufferFrames; i++ ) {
		//for ( j=0; j<2; j++ ) {
		  *buffer++ = 0;
		//}
	}
  }
  
 // fprintf(stderr,"streamTime %f nBufferFrames %d audioOut %d doWhat %d ibuff %d\n",
  //     streamTime,nBufferFrames,rx->audioOut,rx->doWhat,ibuff);
  
  return 0;
}

int StartIt(struct playData *rx)
{
//	ALenum error;

	for(int i=0;i<4;){
		int ibuff;
		ibuff=popBuffa(rx);
		if (ibuff >= 0){
			++i;
			if(setBuffers(rx, ibuff)){
				;
			}
		}
   	}
/*
 	alSourcePlay(rx->source);
	if ((error = alGetError()) != AL_NO_ERROR) 
	{ 
		DisplayALError((unsigned char *)"doAudio alSourcePlay : ", error); 
	} 
*/
	return 0;
}

int playRadio(struct playData *rx)
{

        double rate=rx->device->getSampleRate(SOAPY_SDR_RX, 0);
              
        int size=rate/50;
    
       	if(rx->out)size=rate/10;
 	        
        rx->size=size;
        
        fprintf(stderr,"rate %f rx->size %d\n",rate,rx->size);
                
        for(int k=0;k<NUM_DATA_BUFF;++k){
        	if(rx->buff[k])free(rx->buff[k]);
        	rx->buff[k]=(float *)malloc(2*size*4);
        	if(!rx->buff[k]){
        	    fprintf(stderr,"1 malloc Errror %ld\n",(long)(2*size*4));
       	     	return 1;
       		}
        	zerol((char *)rx->buff[k],2*size*4);
        	rx->buffStack[k]=-1;
        }

        for(int k=0;k<NUM_ABUFF;++k){
        	if(rx->buffa[k])free(rx->buffa[k]);
        	rx->buffa[k]=(short int *)malloc(2*rx->faudio*4);
        	if(!rx->buffa[k]){
        	    fprintf(stderr,"1 malloc Errror %ld\n",(long)(2*rx->faudio*4));
       	     	return 1;
       		}
        	zerol((char *)rx->buffa[k],2*rx->faudio*4);
        	rx->buffStacka[k]=-1;
        }
        
        for(int k=0;k<NUM_BUFFERS;++k){
        	rx->bufferState[k]=0;
        }
    
    	if(rx->dumpbyminute){
    		struct tm today;
    		struct tm next;
    		fprintf(stderr,"Waiting For Next Minute\n");
    		time_t now;
    		time(&now);
    		today = *localtime(&now);
    		do {
    			Sleep2(5);
      			time(&now);
    			next = *localtime(&now);
  			}while(today.tm_min == next.tm_min);
  			
    	    rx->doWhat=2;

    	    fprintf(stderr,"Start Time: ");
    		fprintf(stderr,"year %d ",next.tm_year+1900);
       		fprintf(stderr,"month %d ",next.tm_mon+1); 		
      		fprintf(stderr,"day %d ",next.tm_mday);
    		fprintf(stderr,"hour %d ",next.tm_hour);
     		fprintf(stderr,"min %d ",next.tm_min);
   	        fprintf(stderr,"sec %d \n",next.tm_sec);


    	}else{
    	   rx->doWhat=2;
    	}
        
        rx->witch=0;
        
        rx->frame=0;
        
       // launchThread((void *)rx,Process);   	

       // launchThread((void *)rx,Process); 
          	
        launchThread((void *)rx,Process);   	        
        
		Sleep2(100);
 
        fprintf(stderr,"Start playing\n");
        
    int count=0;
	rx->timestart=rtime();
  	while(!threadexit){
  		Sleep2(50);
  		
		if(rx->timeout > 0 && rtime() > rx->timeout+rx->timestart){
			break;
		}
	
		if(rx->out){
			int ibuff;
			ibuff=popBuffa(rx);
			if (ibuff >= 0){
			   	char filename[256];
				short int *buff= rx->buffa[ibuff % NUM_ABUFF];
				fwrite(buff, 2, rx->faudio/10,rx->out);
				if(rx->dumpbyminute){
					if(++count == 10*60){
						fclose(rx->out);
						sprintf(filename,"minute-%08d.raw",rx->idump++);
						fprintf(stderr,"filename %s\n",filename);
						rx->out=fopen(filename,"wb");
						if(rx->out == NULL){
							fprintf(stderr,"Could Not Open %s to Write\n",filename);
							exit(1);
						}
						count=0;
					}
				}
			}
		}
		
		
   	}      
    double end=rtime();
    
    double total=end-rx->timestart;
    
    fprintf(stderr," Seconds %.2f\n",total);
    
    rx->doWhat=0;
        
    rx->frame=-1;

	Sleep2(100);

    return 0;
  
        
       // StartIt(rx);
/*  
	double start=rtime();
  	for(int i=0;i<100;){
  		Sleep2(5);
		int ibuff;
		ibuff=popBuffa(rx);
		if (ibuff >= 0){
			++i;
		}
   	}      
    double end=rtime();
    
    double total=end-start;
    
    fprintf(stderr," Seconds %.4f Seconds/frame %.4f\n",total,total/100);
    
    exit(1);
*/   

}


int setBuffers(struct playData *rx, int numBuff)
{
    
 	for(int m=0;m<NUM_BUFFERS;++m){
	    if(rx->bufferState[m] == 0){
	        // buffer=rx->buffers[m];
	         rx->bufferState[m]=1;
	         break;
	    }
	}


    {
        static int count=0;
        static FILE *out=NULL;
        if(out==NULL){
            out=fopen("sound.raw","w");
        }
        
        fwrite(rx->buffa[numBuff % NUM_ABUFF], 2, 4800,out);
        if(count++ > 100){
            fclose(out);
            exit(1);
        }
 
    }

	return 0;
}

int Process(void *rxv)
{
	struct playData *rx=(struct playData *)rxv;
	
	struct Filters f;
	
	zerol((char *)&f,sizeof(f));

	f.thread=rx->thread++;
	
	setFilters(rx,&f);
	
	float *wBuff=(float *)malloc(2*rx->size*4);
    if(!wBuff){
        fprintf(stderr,"2 malloc Errror %ld\n",(long)(2*rx->size*4));
       	 return 1;
    }
    zerol((char *)wBuff,2*rx->size*4);

	fprintf(stderr,"Process Start rx->frame %d\n",rx->frame);
	
	
	float *aBuff=(float *)malloc(2*rx->faudio*4);
    if(!aBuff){
        fprintf(stderr,"3 malloc Errror %ld\n",(long)(2*rx->faudio*4));
       	 return 1;
    }
    zerol((char *)aBuff,2*rx->faudio*4);
	
	while(rx->frame >= 0){
		if(doFilter(rx,wBuff,aBuff,&f)){
			Sleep2(5);
		}else{
			doAudio(aBuff,rx);
		}
	}
	fprintf(stderr,"Process return rx->frame %d\n",rx->frame);
	
	if(wBuff)free(wBuff);
	
	if(aBuff)free(aBuff);
	
	if (f.iqSampler)msresamp_crcf_destroy(f.iqSampler);
	
	if (f.iqSampler2)msresamp_rrrf_destroy(f.iqSampler2);
	
	
    if(f.fShift)nco_crcf_destroy(f.fShift);
    
    if(f.demod)freqdem_destroy(f.demod);
    
    if(f.demodAM)ampmodem_destroy(f.demodAM);

	return 0;
}

int doFilter(struct playData *rx,float *wBuff,float *aBuff,struct Filters *f)
{
 	int ip=popBuff(rx);
 	if(ip < 0){
 	// fprintf(stderr,"wait thread %d\n",f->thread);
 	     return 1;
 	}
 	
 	//fprintf(stderr,"ip %d thread %d\n",ip,f->thread);
 	
 	int witch=ip % NUM_DATA_BUFF;
 	
 	
 	//fprintf(stderr,"doFilter witch %d ip %d start \n",witch,ip);
	
 	
 	// fprintf(stderr,"shift %f size %d fShift %p\n",shift,rx->size,rx->fShift);
 /*
 
 	float shift=rx->fc-rx->f;
 
	if (shift != 0) {
		if (shift > 0) {
			nco_crcf_mix_block_up(f->fShift, (liquid_float_complex *)rx->buff[witch], (liquid_float_complex *)wBuff, rx->size);
		} else {
			nco_crcf_mix_block_down(f->fShift, (liquid_float_complex *)rx->buff[witch], (liquid_float_complex *)wBuff,rx->size);
		}
	 }
*/


		float *buf=rx->buff[witch];
		float *buf2=wBuff;
				
		double sint,cost;

        for (int k = 0 ; k < rx->size ; k++){
            float r = buf[k * rx->channels];
            float i = buf[k * rx->channels + 1];
            //r = 0.001*(rand() % 100);
            //i = 0.001*(rand() % 100);
            if(rx->dt > 0){
                buf2[k * rx->channels] = (r*rx->coso - i*rx->sino);
                buf2[k * rx->channels + 1] = (i*rx->coso + r*rx->sino);
                sint=rx->sino*rx->cosdt+rx->coso*rx->sindt;
                cost=rx->coso*rx->cosdt-rx->sino*rx->sindt;
                rx->coso=cost;
                rx->sino=sint;
             }else{
                buf2[k * rx->channels] = r;
                buf2[k * rx->channels + 1] = i;
            }
            
            
            
        }
        
        
    double r=sqrt(rx->coso*rx->coso+rx->sino*rx->sino);
    rx->coso /= r;
    rx->sino /= r;
      

	//float *buf=aBuff;
	buf=aBuff;
	
//	float *buf2=rx->wBuff;
	
    unsigned int num;
    unsigned int num2;
    
    num=0;
    num2=0;
    
    msresamp_crcf_execute(f->iqSampler, (liquid_float_complex *)buf2, rx->size, (liquid_float_complex *)buf, &num);  // decimate
        
    if(rx->decodemode < MODE_AM){

		freqdem_demodulate_block(f->demod, (liquid_float_complex *)buf, (int)num, (float *)buf2);

        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate

        //fprintf(stderr,"2 rx->size %d num %u num2 %u\n",rx->size,num,num2);

    }else if(rx->decodemode < MODE_USB){
        #define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

        for(unsigned int n=0;n<num;++n){
            double mag=sqrt(buf[2*n]*buf[2*n]+buf[2*n+1]*buf[2*n+1]);
            double z0=mag + (f->amHistory * DC_ALPHA);
            buf2[n]=(float)(z0-f->amHistory);
            f->amHistory=z0;
        }
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
    }else{
        ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buf, (int)num, (float *)buf2);
        msresamp_rrrf_execute(f->iqSampler2, (float *)buf2, num, (float *)buf, &num2);  // interpolate
   }

        
	return 0;
}

int pushBuffa(int nbuffer,struct playData *rx)
{

	mutexa.lock();
//	fprintf(stderr,"pushBuffa in %d\n",rx->bufftopa);
	
    if(rx->bufftopa >= NUM_ABUFF){
        rx->bufftopa=NUM_ABUFF;
        int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF;++k){
             if(rx->buffStacka[k] < small){
             	small=rx->buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	rx->buffStacka[ks]=nbuffer;
        }
   }else{
    	rx->buffStacka[rx->bufftopa++]=nbuffer;
    }
    
//    fprintf(stderr,"pushBuffa nbuffer %d top %d\n",nbuffer,rx->bufftopa);
    
//	fprintf(stderr,"pushBuffa out\n");
	mutexa.unlock();

	return 0;
}

int popBuffa(struct playData *rx)
{
	int ret;
	
	
	mutexa.lock();
//	fprintf(stderr,"popBuffa in %d\n",rx->bufftopa);
	
	ret=-1;
	
 	if(rx->bufftopa < 1)goto Out;
 	
 	if(rx->bufftopa == 1){
 		ret=rx->buffStacka[0];
 		rx->bufftopa=0;
 		goto Out;
 	}
 	
       int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftopa;++k){
             if(rx->buffStacka[k] < small){
             	small=rx->buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=rx->buffStacka[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<rx->bufftopa;++k)
        	{
        		if(k == ks)continue;
        		rx->buffStacka[kk++]=rx->buffStacka[k];
        	}
        	rx->bufftopa--;
        }
	
	
Out:
//    if(ret > 0)fprintf(stderr,"popBuffa ret %d top %d\n",ret,rx->bufftopa);
//	fprintf(stderr,"popBuffa out\n");
	mutexa.unlock();

	return ret;
}

int pushBuff(int nbuffer,struct playData *rx)
{

	mutex.lock();
	
    if(rx->bufftop >= NUM_DATA_BUFF){
        rx->bufftop=NUM_DATA_BUFF;
        int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<NUM_DATA_BUFF;++k){
             if(rx->buffStack[k] < small){
             	small=rx->buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	rx->buffStack[ks]=nbuffer;
        }
   }else{
    	rx->buffStack[rx->bufftop++]=nbuffer;
    }
    
	mutex.unlock();

	
	return 0;
}

int popBuff(struct playData *rx)
{
	int ret;
	
	
	mutex.lock();

	
	ret=-1;
	
 	if(rx->bufftop < 1)goto Out;
 	
 	if(rx->bufftop == 1){
 		ret=rx->buffStack[0];
 		rx->bufftop=0;
 		goto Out;
 	}
 	
       int small,ks;
        small=1000000000;
        ks=-1;
        for(int k=0;k<rx->bufftop;++k){
             if(rx->buffStack[k] < small){
             	small=rx->buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=rx->buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<rx->bufftop;++k)
        	{
        		if(k == ks)continue;
        		rx->buffStack[kk++]=rx->buffStack[k];
        	}
        	rx->bufftop--;
        }
	
	
Out:
	mutex.unlock();

	return ret;
}

int findRadio(struct playData *rx)
{
    
    std::string argStr;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    std::cout << "Number of Devices Found: " << results.size() << std::endl;
    
    if(results.size() < 1)return 1;
    
    rx->device = NULL;
    
    SoapySDR::Kwargs deviceArgs;
    
    
    
    for(unsigned int k=0;k<results.size();++k){
			std::cout << "SDR device = " << k << "  ";
			deviceArgs = results[k];
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				if (it->first == "label")std::cout << "  " << it->first << " = " << it->second << std::endl;
			}
    }
    
    std::cout << std::endl;

    for(unsigned int k=0;k<results.size();++k){
    
    	if(k == rx->deviceNumber){
        
			deviceArgs = results[k];
		
	
			std::cout << "device = " << k << " selected" << std::endl;
	
	
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				std::cout << it->first << " = " << it->second << std::endl;
				if (it->first == "driver") {
					//dev->setDriver(it->second);
				} else if (it->first == "label" || it->first == "device") {
					//dev->setName(it->second);
				}
			}
		
			// testRadio(rx,deviceArgs);
			
			std::cout << std::endl;

			
			rx->device = SoapySDR::Device::make(deviceArgs);
			
			std::cout << std::endl;
			
			        //query device info
        	std::vector<std::string> names = rx->device->listAntennas(SOAPY_SDR_RX,  rx->channel);
        	std::cout << "Rx antennas: " << std::endl;
        
        	for (std::vector<std::string>::const_iterator ii = names.begin(); ii != names.end(); ++ii){
            
           		 std::cout << (*ii) << std::endl;
            
        	}
			        
        	rx->antennaCount=names.size();
        	rx->antenna=(char **)cMalloc((unsigned long)(rx->antennaCount*sizeof(char *)),8833);
        	for (size_t i=0;i<names.size();++i){
            	rx->antenna[i]=strsave((char *)names[i].c_str(),5555);
        	}

			
			
			std::cout << std::endl;
        	std::cout << "Setting Info: " << std::endl;
			
        	SoapySDR::ArgInfoList args = rx->device->getSettingInfo();
        	if (args.size()) {
            	for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                	SoapySDR::ArgInfo arg = (*args_i);

                	fprintf(stderr,"key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str(),
                       	(int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

            	}
        	}
			std::cout << std::endl;
			
			
			
		
            if(rx->antennaUse){
            	fprintf(stderr,"Use antenna \"%s\"\n",rx->antennaUse);
            	rx->device->setAntenna(SOAPY_SDR_RX, rx->channel, rx->antennaUse);
            }
            
            
            
            if(rx->setcount){          
             	fprintf(stderr,"setcount %d\n",rx->setcount);
           		for(int k=0;k<rx->setcount;++k){
              		fprintf(stderr,"%s %s\n",rx->set[k].c_str(),rx->value[k].c_str());
               		rx->device->writeSetting(rx->set[k],rx->value[k]);
                }
            }
                
                
        	SoapySDR::Range range=rx->device->getGainRange(SOAPY_SDR_RX, rx->channel);
        
            fprintf(stderr,"Gain range RX min %g max %g \n",range.minimum(),range.maximum());
   
            SoapySDR::RangeList rlist=rx->device->getFrequencyRange(SOAPY_SDR_RX, rx->channel);
            
        	for (size_t j = 0; j < rlist.size(); j++)
        	{
         	    fprintf(stderr,"FrequencyRange min %g max %g \n",rlist[j].minimum(),rlist[j].maximum());
        	}

        	std::vector<double> band=rx->device->listBandwidths(SOAPY_SDR_RX, rx->channel);
        	if(band.size()){
                fprintf(stderr,"\nBandwidth MHZ ");  		
				for (size_t j = 0; j <band.size(); j++)
				{
				   fprintf(stderr," %.2f ",band[j]/1.0e6);
				}
				fprintf(stderr,"\n\n");
            }
            

			std::vector<double> rate=rx->device->listSampleRates(SOAPY_SDR_RX, rx->channel);
        	if(rate.size()){
                 fprintf(stderr,"SampleRates MHZ ");
      		}
			for (size_t j = 0; j < rate.size(); j++)
        	{
           		fprintf(stderr," %.2f ",rate[j]/1.0e6);
         	}
            fprintf(stderr,"\n\n");


			rx->device->setSampleRate(SOAPY_SDR_RX, rx->channel, rx->samplerate);
			
			rx->device->setFrequency(SOAPY_SDR_RX, rx->channel, rx->fc);
			
        	std::cout << "rx->samplerate " << rx->samplerate << std::endl;
			
			rx->rxStream = rx->device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);

			rx->device->activateStream(rx->rxStream, 0, 0, 0); 

			std::cout << "getGainMode: " << rx->device->getGainMode(SOAPY_SDR_RX, rx->channel) << std::endl;
			
			int hasFrequencyCorrection= rx->device->hasFrequencyCorrection(SOAPY_SDR_RX, rx->channel);
			
			std::cout << "hasFrequencyCorrection: " << hasFrequencyCorrection<< std::endl;
			
			if(hasFrequencyCorrection && rx->PPM){
			    rx->device->setFrequencyCorrection(SOAPY_SDR_RX, rx->channel,rx->PPM);
			}
			
		
			// std::cout << std::endl;
			
             
		}
    
}
    
    return 0;
    
}
int rxBuffer(void *rxv)
{

	struct playData *rx=(struct playData *)rxv;
	
	while(1)
	{
	     switch(rx->doWhat){
	     case 0:
	     	Sleep2(5);
	        break;
	     case 1:
	        fprintf(stderr,"Exit rxBuffer\n");
	        return 0;
		 case 2:
	       // fprintf(stderr,"rxBuffer case 2\n");
	       	        
        	long long timeNs=0;
           
            float *buff=rx->buff[rx->witch % NUM_DATA_BUFF];
             
            void *buffs[] = {buff};
            
            int toRead=rx->size;
            
            
            int count=0;
                 
            while(rx->doWhat == 2){
				int flags=0;
				
				buffs[0]=buff+2*count;
				
				int iread;
				
				iread=toRead;
				if(iread > 500000)iread=500000;
				
				int ret = rx->device->readStream(rx->rxStream, buffs, iread, flags, timeNs, 100000L);
			 
				 timeNs++;
						   
				if(ret <= 0){
				   fprintf(stderr,"ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f \n", ret, flags, timeNs,buff[0],buff[1]);
				   break;
				}else if(ret < toRead){
                    count += ret;
                    toRead=toRead-ret;
					//fprintf(stderr,"ret=%d, flags=%d, timeNs=%lld b0 %f b1 %f toRead %d witch %d\n", ret, flags, timeNs,buff[0],buff[1],toRead,rx->witch);
				}else{
					break;
				}
            }
	        if(rx->doWhat == 2){
	        	pushBuff(rx->witch,rx);
             	++rx->witch;
	        	//rx->doWhat=0;
	        }
	        break;
		     
	     }
	     
	     //fprintf(stderr,"rx->doWhat %d\n",rx->doWhat);
	}
	return 0;
}

int setFilters(struct playData *rx,struct Filters *f)
{

    // double shift=rx->f-rx->fc;
    
    if(!rx)return 0;
    
    float As = 60.0f;
    
    float ratio=(float)(rx->faudio / rx->samplerate);
    
    liquid_ampmodem_type mode=LIQUID_AMPMODEM_DSB;
    
    int iflag=0;
    
    if(rx->decodemode == MODE_AM){
        rx->bw=10000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NAM){
        rx->bw=5000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NBFM){
        rx->bw=12500.0;
    }else if(rx->decodemode == MODE_FM){
        rx->bw=200000.0;
    }else if(rx->decodemode == MODE_USB){   // Above 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_USB;
        iflag=1;
    }else if(rx->decodemode == MODE_LSB){  // Below 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }else if(rx->decodemode == MODE_CW){  // Below 10 MHZ
        rx->bw=3000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }
    
    rx->Ratio = (float)(rx->bw/ rx->samplerate);
    ratio= (float)(rx->faudio/rx->bw);
    
    f->demod=freqdem_create(0.5);
    
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
 #else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSampler  = msresamp_crcf_create(rx->Ratio, As);
    
    f->iqSampler2 = msresamp_rrrf_create(ratio, As);
    
    //msresamp_crcf_print(f->iqSampler);
    
    
    f->fShift = nco_crcf_create(LIQUID_NCO);
    
    f->amHistory=0;
    
    return 0;
    	
}


static int initPlay(struct playData *rx)
{
//    ALenum error;
    
	rx->doWhat=0;
	
    rx->witch=0;
    
    rx->audioOut=0;
    if(rx->fc != rx->f){
    	float pi;
    	pi=4.0*atan(1.0);
    	rx->dt=1.0/(double)rx->samplerate;
    	rx->sino=0;
    	rx->coso=1;
    	rx->w=2.0*pi*(rx->fc - rx->f);
    	rx->sindt=sin(rx->w*rx->dt);
    	rx->cosdt=cos(rx->w*rx->dt);
    	fprintf(stderr,"fc %f f %f dt %g samplerate %d\n",rx->fc,rx->f,rx->dt,rx->samplerate);
    }
    
    
	return 0;
}

static int stopPlay(struct playData *rx)
{

    rx->doWhat=1;
    
    Sleep2(100);
    
    if(rx->device){
        rx->device->deactivateStream(rx->rxStream, 0, 0);
    
        rx->device->closeStream(rx->rxStream);
    
        SoapySDR::Device::unmake(rx->device);
    }

    
	return 0;
}
double rtime(void)
{
        long milliseconds;
        long Seconds;
        double ret;


        GetTime(&Seconds,&milliseconds);

        ret=(double)Seconds+(double)milliseconds/1000.;

        return ret;

}
int GetTime(long *Seconds,long *milliseconds)
{
        struct timeval curTime;

        gettimeofday(&curTime,(struct timezone *) NULL);
        *Seconds=curTime.tv_sec;
        *milliseconds=curTime.tv_usec/1000;
        return 0;
}

int zerol(unsigned char *s,unsigned long n)
{
    if(!s || (n <= 0))return 1;


	while(n-- > 0)*s++ = 0;
	
	return 0;
}

int testRadio(struct playData *rx,SoapySDR::Kwargs deviceArgs)
{

    try
    {
        rx->device = SoapySDR::Device::make(deviceArgs);
        std::cout << "  driver=" << rx->device->getDriverKey() << std::endl;
        std::cout << "  hardware=" << rx->device->getHardwareKey() << std::endl;
        
        
        SoapySDR::Kwargs it=rx->device->getHardwareInfo();
        
        for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
        {
            std::cout << (*ii).first << ": " << (*ii).second << std::endl;
        }
        

        std::cout << std::endl;
        
        
        rx->rxStream = rx->device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);
        
        rx->device->setSampleRate(SOAPY_SDR_RX, 0, rx->samplerate);

        rx->device->activateStream(rx->rxStream, 0, 0, 0);
       
        
        //query device info
        std::vector<std::string> names = rx->device->listAntennas(SOAPY_SDR_RX, 0);
        std::cout << "Rx antennas: " << std::endl;
        
        for (std::vector<std::string>::const_iterator ii = names.begin(); ii != names.end(); ++ii){
            
            std::cout << (*ii) << std::endl;
            
        }
        
        
        names = rx->device->listGains(SOAPY_SDR_RX, 0);
        std::cout <<  "Rx gains: " << std::endl;
        
        for (int ii = 0; ii < names.size(); ++ii){
            
            if(names[ii] == "ATT"){
                rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 0.0);
                //std::cout << "ATT Set Zero"<< std::endl;
            }
            if(names[ii] == "VGA"){
                //rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 38.0);
                rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 36.0);
                //std::cout << "VGA Set 52"<< std::endl;
            }
            if(names[ii] == "TUNER"){
                //rx->device->setGain(SOAPY_SDR_RX, 0, names[i], 45);
                rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 60);
                //std::cout << "TUNER Set 59"<< std::endl;
            }
            if(names[ii] == "LNAT"){
                //rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 45);
                rx->device->setGain(SOAPY_SDR_RX, 0, names[ii], 60);
                //std::cout << "LNAT Set 59"<< std::endl;
            }
           
            
            std::cout << (names[ii]) << " " << rx->device->getGain(SOAPY_SDR_RX, 0, names[ii]) << " ";
            
        }
        
        std::cout << std::endl;
        
        
        SoapySDR::Range granges = rx->device->getGainRange(SOAPY_SDR_RX, 0);
        std::cout <<  "getGainRange: " << std::endl; 
        
        std::cout << (granges.minimum()) << " ";
        std::cout << (granges.maximum()) << " ";
        
        bool hasGainMode;
        
        hasGainMode=rx->device->hasGainMode(SOAPY_SDR_RX, 0);
        
        std::cout << std::endl;
        
        std::cout << "hasGainMode: " << hasGainMode << " ";
        
        std::cout << std::endl;
        
        
        if(hasGainMode){
            
            // rx->device->setGainMode(SOAPY_SDR_RX, 0, true);
            
            std::cout << "getGainMode: " << rx->device->getGainMode(SOAPY_SDR_RX, 0) << " ";
        
            std::cout << std::endl;
        }
        
        
        SoapySDR::RangeList ranges = rx->device->getFrequencyRange(SOAPY_SDR_RX, 0);
        std::cout <<  "Rx freq ranges: " << std::endl; 
        
        for (std::vector<SoapySDR::Range>::const_iterator ii = ranges.begin(); ii != ranges.end(); ++ii){
            
            std::cout << (ii->minimum()) << " ";
            std::cout << (ii->maximum()) << " ";
            
        }
        
        std::cout << std::endl;
        
        
        //query device info
        std::vector<double> bnames = rx->device->listBandwidths(SOAPY_SDR_RX, 0);
        std::cout <<  "Bandwidths: " << std::endl; 
        
        for (std::vector<double>::const_iterator ii = bnames.begin(); ii != bnames.end(); ++ii){
            
            std::cout << (*ii) << std::endl;
            
        }
      
        
        std::cout << std::endl;
    
        
        std::cout << "hasDCOffset: " << rx->device->hasDCOffset(SOAPY_SDR_RX, 0) << " ";
        
        std::cout << std::endl;
        
        
        bool hasDCOffsetMode=rx->device->hasDCOffsetMode(SOAPY_SDR_RX, 0);
        
        std::cout << "hasDCOffsetMode: " << hasDCOffsetMode << " ";
        
        std::cout << std::endl;
        
        if(hasDCOffsetMode){
            
            bool automatic=true;
            
            rx->device->setDCOffsetMode(SOAPY_SDR_RX, 0,automatic);
            
            std::cout << "getDCOffsetMode: " << rx->device->getDCOffsetMode(SOAPY_SDR_RX, 0) << " ";
            
            std::cout << std::endl;
           
        }
        
        SoapySDR::ArgInfoList args = rx->device->getSettingInfo();
        if (args.size()) {
            for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
                SoapySDR::ArgInfo arg = (*args_i);

                fprintf(stderr,"key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),rx->device->readSetting(arg.key).c_str(),
                       (int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

            }
        }
      
        
        
        rx->device->setSampleRate(SOAPY_SDR_RX, 0, rx->samplerate);
        
        rx->device->setFrequency(SOAPY_SDR_RX, 0, rx->fc);
        
        
        rx->MTU=rx->device->getStreamMTU(rx->rxStream);
        
        std::cout << "rx->MTU: " << rx->MTU << " ";
        
        std::cout << std::endl;
     
        
        double freq=rx->device->getFrequency(SOAPY_SDR_RX, 0);
        
        std::cout << "freq " << freq << std::endl;
        
        double rate=rx->device->getSampleRate(SOAPY_SDR_RX, 0);
        
        std::cout << "rate " << rate << std::endl;
              
    }
    
    catch (const std::exception &ex)
    {
        std::cerr << "Error making device: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }    
    
    std::cout << std::endl;
    
    if(rx->device == NULL){
        std::cerr << "Error : device == NULL" << std::endl;
        return 1;
    }
    
    return 0;

}
int doAudio(float *aBuff,struct playData *rx)
{
	int short *data;
	int audioOut;

	
	mutexo.lock();
	audioOut=rx->audioOut;
	//fprintf(stderr,"audioOut %d\n",audioOut);
	data=rx->buffa[rx->audioOut++ % NUM_ABUFF];
	mutexo.unlock();

	

	double amin=1e30;
	double amax=-1e30;
		
	float *buff=aBuff;

	
	double dmin,dnom,gain;
	
	gain=rx->gain;
	
	if(gain <= 0.0)gain=1.0;
	
	double average=0;
	
	for (int i=0; i<rx->faudio; i++ ) {
		double v;
		v=buff[i];
        average += v;
		if(v < amin)amin=v;
		if(v > amax)amax=v;
	}
	
	average /= rx->faudio;
	
    amin -= average;

    amax -= average;
	

    if(rx->aminGlobal == 0.0)rx->aminGlobal=amin;

    rx->aminGlobal = 0.8*rx->aminGlobal+0.2*amin;

    amin=rx->aminGlobal;

    

    if(rx->amaxGlobal == 0.0)rx->amaxGlobal=amax;

    rx->amaxGlobal = 0.8*rx->amaxGlobal+0.2*amax;

    amax=rx->amaxGlobal;


	//fprintf(stderr,"doAudio size %d amin %f amax %f audioOut %d\n",BLOCK_SIZE,amin,amax,audioOut);
	
	
    if((amax-amin) > 0){

        dnom=65535.0/(amax-amin);
    }else{

        dnom=65535.0;
    }
		
	dmin=amin;

	for(int k=0;k<rx->faudio;++k){
		double v;

        v=buff[k];

		v=gain*((v-average)*dnom);

        if(v < -32765){
            v = -32765;
        }else if(v > 32765){
            v=32765;
        }

		data[k]=(short int)v;
	}	

	pushBuffa(audioOut,rx);


	return 0;
}
int mstrncpy(char *out,char *in,long n)
{
	if(!out || !in || (n <= 0))return 1;
	
	while(n-- > 0){
	    if(*in == 0){
			*out = 0;
			break;
	    }else{
			*out++ = *in++;
	    }
	}
	
	return 0;
}
char *strsave(char *s,int tag)
{
	long length;
	char *p;
	
	if(!s)return NULL;
	
	length=(long)strlen(s)+1;
	
	if((p=(char *)cMalloc(length+1,tag)) != NULL)
		mstrncpy(p,s,length);
	return(p);
}
static void list_audio()
{

    ALboolean enumeration;
    
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE){
        fprintf(stderr,"enumeration not supported\n");
    }else{
        list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
        list_audio_devices(alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));
    }

}
static void list_audio_devices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    fprintf(stdout, "OpenAL Devices list:\n");
    fprintf(stdout, "----------\n");

    while (device && *device != '\0' && next && *next != '\0') {
        fprintf(stdout, "%s\n", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    fprintf(stdout, "----------\n\n");
}

