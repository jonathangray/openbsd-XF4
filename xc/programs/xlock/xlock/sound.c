#if !defined( lint ) && !defined( SABER )
static const char sccsid[] = "@(#)sound.c	4.00 97/01/01 xlockmore";

#endif

/*-
 * sound.c - xlock.c and vms_play.c
 *
 * See xlock.c for copying information.
 *
 */

#include "xlock.h"

#ifdef USE_RPLAY
/*-
 * The stuff below does not appear to _compile_ on Solaris>=2.6 with gcc
 * -- xlock maintainer
 */
#include <rplay.h>

void
play_sound(char *string)
{
	int         rplay_fd = rplay_open_default();

	if (rplay_fd >= 0) {
		rplay_sound(rplay_fd, string);
		rplay_close(rplay_fd);
	}
}
#endif

#ifdef USE_NAS
/* Gives me lots of errors when I compile nas-1.2p5  -- xlock maintainer */

/*-
 * Connect each time, because it might be that the server was not running
 * when xlock first started, but is when next nas_play is called
 */

#include <audio/audio.h>
#include <audio/audiolib.h>

extern Display *dsp;

void
play_sound(char *string)
{
	char       *auservername = DisplayString(dsp);
	char       *fname = string;
	AuServer   *aud;	/* audio server connection */

	if (!(aud = AuOpenServer(auservername, 0, NULL, 0, NULL, NULL)))
		return;		/*cannot connect - no server? */
	/*
	 * now play the file at recorded volume (3rd arg is a percentage),
	 * synchronously
	 */
	AuSoundPlaySynchronousFromFile(aud, fname, 100);
	AuCloseServer(aud);
}
#endif

#ifdef USE_XAUDIO
  /* Anybody ever get this working? XAudio.007 */
#endif

#ifdef USE_VMSPLAY
/*-
 * Jouk Jansen <joukj@hrem.stm.tudelft.nl> contributed this
 * which he found at http://axp616.gsi.de:8080/www/vms/mzsw.html
 *
 * quick hack for sounds in xlockmore on VMS
 * with a the above AUDIO package slightly modified
 */
#include <file.h>
#include <unixio.h>
#include <iodef.h>
#include "vms_amd.h"

void
play_sound(char *filename)
{
	int         i, j, status;
	char        buffer[2048];
	int         volume = 65;	/* volume is default to 65% */

	/* use the internal speaker(s) */
	int         speaker = SO_INTERNAL /*SO_EXTERNAL */ ;
	int         fp;

	status = AmdInitialize("SO:", volume);	/* Initialize access to AMD */
	AmdSelect(speaker);	/* Select which speaker */
	fp = open(filename, O_RDONLY, 0777);	/* Open the file */
	if (!(fp == -1)) {
		/* Read through it */
		i = read(fp, buffer, 2048);
		while (i) {
			status = AmdWrite(buffer, i);
			if (!(status & 1))
				exit(status);
			i = read(fp, buffer, 1024);
		}
	}
	(void) close(fp);
}

#endif

#ifdef DEF_PLAY
void
play_sound(char *string)
{
	char        *progrun = NULL;

	if ((progrun = (char *) malloc(strlen(DEF_PLAY) + strlen(string) + 10)) != NULL) {
		(void) sprintf(progrun, "( %s%s ) 2>&1", DEF_PLAY, string);
		/*(void) printf("%s\n", progrun); */
		(void) system(progrun);
		(void) free((void *) progrun);
	}
}

#endif

#ifdef USE_ESOUND

#ifndef DEFAULT_SOUND_DIR
#define DEFAULT_SOUND_DIR "/usr/lib/sounds/xlockmore/"
#endif

#ifdef HAVE_LIBESD

#include <sys/stat.h>
#include <sys/param.h>
#include <esd.h>

#else

#error Sorry, but you cannot use ESD without ESD !!!?

#endif

typedef struct _esd_sample
{
    char               *file;
    int                 rate;
    int                 format;
    int                 samples;
    unsigned char      *data;
    int                 id;
    struct _esd_sample *next;
} EsdSample_t;

typedef EsdSample_t    *EsdSample_ptr;

static EsdSample_ptr 	EsdSamplesList =(EsdSample_ptr)NULL;
static int	     	sound_fd = -1;

static EsdSample_ptr   	sound_esd_load_sample(char *file);
static void 	     	sound_esd_play(EsdSample_ptr s);
static void	     	sound_esd_destroy_sample(EsdSample_ptr s);
static int	     	sound_esd_init(void);
static void	     	sound_esd_shutdown(void);


/*
 * Public ESOUND sound functions
 * =============================
 */

void
play_sound(char *file)
{
#ifdef DEBUG
    (void) fprintf( stderr, "play_sound %s\n", file );
#endif
    if ( file && *file )
      sound_esd_play( sound_esd_load_sample( file ) );
}

int init_sound(void)
{
    return( sound_esd_init() );
}

void shutdown_sound(void)
{
    sound_esd_shutdown();
}

/*
 * Private ESOUND sound functions
 * ==============================
 */

static EsdSample_ptr
sound_esd_load_sample(char *file)
{
   AFfilehandle        in_file;
   struct stat	       stbuf;
   EsdSample_ptr       s;
   int                 in_format, in_width, in_channels, frame_count;
   int                 bytes_per_frame, frames_read;
   double              in_rate;
   char 	      *origfile = strdup( file ? file : "" );
   char 	       fullfile[MAXPATHLEN];

#ifdef DEBUG
   (void) fprintf(stderr, "sound_esd_load_sample: %s\n", origfile);
#endif

   s = EsdSamplesList;
   while ( s && strcmp( file, s->file ) )
     s = s->next;
   if ( s && !strcmp( file, s->file ) )
     return s;

#ifdef DEBUG
   (void) fprintf(stderr, "sound_esd_load_sample: sample not loaded: loading ...\n", origfile);
#endif

   if (file[0] != '/')
   {
       (void) sprintf( fullfile, "%s/%s", DEFAULT_SOUND_DIR, file );
       file = fullfile;
   }
   if (stat(file, &stbuf) < 0)
   {
       (void) fprintf( stderr, "Error ! Cannot find the sound file %s\n", file);
       return NULL;
   }
   if ( !( in_file = afOpenFile(file, "r", NULL) ) )
   {
       (void) fprintf( stderr, "Error ! Cannot open sound sample ! Bad format ?\n" );
       return(NULL);
   }
   s = EsdSamplesList;
   if ( s )
   {
       while ( s && s->next )
         s = s->next;
       s->next = malloc(sizeof(EsdSample_t));
       if ( !s->next )
       {
           (void) fprintf( stderr, "Error ! cannot allocate sample data !\n" );
           afCloseFile(in_file);
           return NULL;
       }
       s = s->next;
       s->next = NULL;
   }
   else
   {
       s = malloc(sizeof(EsdSample_t));
       if ( !s )
       {
           (void) fprintf( stderr, "Error ! cannot allocate sample data !\n" );
           afCloseFile(in_file);
           return NULL;
       }
       EsdSamplesList = s;
       s->next = NULL;
   }

   frame_count = afGetFrameCount(in_file, AF_DEFAULT_TRACK);
   in_channels = afGetChannels(in_file, AF_DEFAULT_TRACK);
   in_rate = afGetRate(in_file, AF_DEFAULT_TRACK);
   afGetSampleFormat(in_file, AF_DEFAULT_TRACK, &in_format, &in_width);
#ifdef WORDS_BIGENDIAN
   afSetVirtualByteOrder(in_file, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
   afSetVirtualByteOrder(in_file, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
   s->file = strdup(origfile);
   s->rate = 44100;
   s->format = ESD_STREAM | ESD_PLAY;
   s->samples = 0;
   s->data = NULL;
   s->id = 0;

   if (in_width == 8)
      s->format |= ESD_BITS8;
   else if (in_width == 16)
      s->format |= ESD_BITS16;
   bytes_per_frame = (in_width * in_channels) / 8;
   if (in_channels == 1)
      s->format |= ESD_MONO;
   else if (in_channels == 2)
      s->format |= ESD_STEREO;
   s->rate = (int)in_rate;

   s->samples = frame_count * bytes_per_frame;
   s->data = malloc(frame_count * bytes_per_frame);
   if ( !s->data )
   {
       (void) fprintf( stderr, "Error ! cannot allocate memory for sample !\n" );
       afCloseFile(in_file);
       return NULL;
   }
   frames_read = afReadFrames(in_file, AF_DEFAULT_TRACK, s->data, frame_count);
   afCloseFile(in_file);
   return s;
}

static
void
sound_esd_play(EsdSample_ptr s)
{
   int                 size, confirm = 0;

#ifdef DEBUG
   (void) fprintf( stderr, "sound_esd_play\n" );
#endif

   if ((sound_fd < 0) || (!s))
     return;
   if (!s->id)
     {
	if (sound_fd >= 0)
	  {
	     if (s->data)
	       {
		  size = s->samples;
		  s->id = esd_sample_getid(sound_fd, s->file);
		  if (s->id < 0)
		    {
		       s->id = esd_sample_cache(sound_fd, s->format, s->rate, size, s->file);
		       write(sound_fd, s->data, size);
		       confirm = esd_confirm_sample_cache(sound_fd);
		       if (confirm != s->id)
                         s->id = 0;
		    }
		  free(s->data);
		  s->data = NULL;
	       }
	  }
     }
   if (s->id > 0)
     esd_sample_play(sound_fd, s->id);
   return;
}

static void
sound_esd_destroy_sample(EsdSample_ptr s)
{
#ifdef DEBUG
    (void) fprintf( stderr, "sound_esd_destroy_sample\n" );
#endif

    if ((s->id) && (sound_fd >= 0))
      esd_sample_free(sound_fd, s->id);

    if (s->data)
      free(s->data);
    if (s->file)
      free(s->file);
}

static int
sound_esd_init(void)
{
    int                 fd;

#ifdef DEBUG
    (void) fprintf(stderr, "sound_esd_init\n");
#endif
    if (sound_fd != -1)
      return 0;
    fd = esd_open_sound(NULL);
    if (fd >= 0)
      sound_fd = fd;
    else
    {
	(void) fprintf(stderr, "Error initialising sound\n");
        return -1;
     }
    return 0;
}

static void
sound_esd_shutdown(void)
{
#ifdef DEBUG
    (void) fprintf( stderr, "sound_esd_shutdown\n" );
#endif
    if (sound_fd >= 0)
    {
        EsdSample_ptr	s = EsdSamplesList;
 
        while ( s )
        {
            EsdSamplesList = s->next;
            sound_esd_destroy_sample( s );
            free( s );
            s = EsdSamplesList;
        }
	close(sound_fd);
	sound_fd = -1;
    }
}

#endif
