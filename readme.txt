SMS Library Documentation
====================
version 0.2 -- June 20, 2008 

libsms in an open source C library that implements SMS techniques for the analysis,
transformation and synthesis of musical sounds based on a sinusoidal plus residual model.
It is derived from the original code of Xavier Serra, as part of his PhD thesis.  You can read
about this and many things related to SMS at the sms homepage:
http://mtg.upf.edu/technologies/sms/

Since Janurary 2009, the code Serra wrote, originally for NextStep, has undergone changes to make
it useful on modern day platforms.  The goal of this library is to be usable in real-time audio
applications for performing high-fidelity synthesis of sound models. It should work on most 
platforms available, although Linux is the only one tested so far.  If you would like to read about the
progress I have been making in achieving these goals, read the "log.org" file in the main
directory.  As you will see, there are many issues still at hand, but the library is very functional.
          - Richard Thomas Eakin

-Overview of subdirectories

src                      - library source files used by sms programs.
tools                   - small sms programs for analyzing, synthesizing and viewing data.
         smsAnal             - computes a .sms file from a .snd file. 
         smsSynth           - synthesizes a soundfile from a .sms file using IFFT.
         smsClean	    - program to clean the .sms files.
         smsPrint             - prints the contents of a .sms file.
         smsToYaml        - writes a .sms file to YAML text format.
         smsResample     - supports frame rate decimation in a .sms file.
man                    - man pages for the tools
test                     - various python scripts and files for testing
pd                      - pd externals (currently only 1) that use the sms library

- Standard way of using sms programs

soundfile   -------->   .sms  ------------------> .aiff
        smsAnal            smsMk, smsSynth

        The sound file can be anything supporded by libsndfile

-How to install

     libsms uses the scons build system.  Go to the main directory (this one) and type
     "scons" for a default build.  Here are some things you can do with scons:
         "scons -h": prints compile options
         "scons install": copies the programs to your system wide folders (you need permissions)
         "scons -c": removes object files and binary files
         "scons -c install": same as above, but removes system wide files too

     There is an external plugin for Miller Puckette's Pure Data in the folder 'pd'.  It can be made by
     going to that directory and typing "scons".  It will open a pre-analyzed .sms file and synthesize
     it in real-time, allowing you to scan back and forth across the models frames.

-Description of the SMS file format (*** Note: these are outdated, please read the sms.h header file
             for the current struct definitions ***)

The SMS file includes a header of variable length and a set of records, each one of the same size.

The header is defined by the following structure:

typedef struct 
{
	/* fix part */
	int iSmsMagic;         /* magic number for SMS data file */
	int iHeadBSize;        /* size in bytes of header */
	int nRecords;	         /* number of data records */
	int iRecordBSize;      /* size in bytes of data record */
	int iFormat;           /* type of data format */
	int iFrameRate;        /* rate in Hz of data records */
	int iStochasticType;   /* representation of stochastic coefficients */
	int nTrajectories;     /* number of trajectoires in each record */
	int nStochasticCoeff;  /* number of stochastic coefficients in each	
 	                          record */
	float fAmplitude;      /* average amplitude of represented sound */
	float fFrequency;      /* average fundamental frequency */
	float fOriginalSRate;  /* sampling rate of original sound */
	int iBegSteadyState;   /* record number of begining of steady state */
	int iEndSteadyState;   /* record number of end of steady state */
	float fResidualPerc;   /* percentage of the residual with respect to the 
	                          original */
	int nLoopRecords;      /* number of loop records specified */
	int nSpecEnvelopePoints; /* number of breakpoints in spectral envelope */
	int nTextCharacters;   /* number of text characters */
	/* variable part */
	int *pILoopRecords;    /* array of record numbers of loop points */
	float *pFSpectralEnvelope; /* spectral envelope of partials */
	char *pChTextCharacters; /* Textual information relating to the sound */
	char *pChDataRecords;   /* pointer to data records */
} SMSHeader;


The header has two parts, one of fix length and another of variable length. The actual length of the variable part is specified in the fixed part.
The file sms.h has the macros that define some of the header parameters, such as the magic number of the different data formats. The file smsIO.c in the library has the functions that read and write this header structure.

After the header the file has the actual SMS data as a set of records. Each record includes the deterministic and stochastic representation of a given frame. The function setSmsRecord puts the data of a record into the following structure:

typedef struct 
{
	float *pFFreqTraj;         /* frequency of sinusoids */
	float *pFMagTraj;          /* magnitude of sinusoids */
	float *pFPhaTraj;          /* phase of sinusoids */
	int nTraj;                 /* number of sinusoids */
	float *pFStocGain;         /* gain of stochastic component */
	float *pFStocCoeff;        /* filter coefficients for stochastic component */
	int nCoeff;                /* number of filter coefficients */
} SMS_DATA;

This structure is the one generated by the analysis program and the one used by the synthesis program to generate a sound. This assumes equally spaced data. The data stored in the file is in a more compact form. In the file there is no need to store the number of trajectories and the number of coefficients in every record, since they are the same for every record in the file. There is also no need to store the pointers to the data since the arrays are stored in order. Thus when the file is actually used in the program the SMS_DATA structure is set to point to the appropiate places in the data record to be used.
