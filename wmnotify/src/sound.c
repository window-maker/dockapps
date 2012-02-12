/*
 * sound.c -- Plays sound from file.
 *
 * Copyright (C) 2003 Hugo Villeneuve <hugo@hugovil.com>
 * Based on the 'sndfile-play' demo program from 'libsndfile'
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(HAVE_SNDFILE)

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#if defined(__linux__)
#  include <fcntl.h>
#  include <sys/ioctl.h>
#  include <sys/soundcard.h>
#elif( defined(sun) && defined(unix) )
#  include <fcntl.h>
#  include <sys/ioctl.h>
#  include <sys/audioio.h>
#endif

#include <sndfile.h>

#include "common.h"
#include "wmnotify.h"
#include "sound.h"


#define BUFFER_LEN ((sf_count_t) 2048)


static int
OpenDspDevice( int channels, int srate )
{
  int fd, status;
#if defined (__linux__)
  int stereo, temp;
  const char audio_device[] = "/dev/dsp";
#elif( defined(sun) && defined(unix) )
  audio_info_t audio_info;
  const char audio_device[] = "/dev/audio";
#endif
  
#if defined (__linux__)
  fd = open( audio_device, O_WRONLY, 0 );
#elif( defined(sun) && defined(unix) )
  /* Open the audio device - write only, non-blocking */
  fd = open( audio_device, O_WRONLY | O_NONBLOCK );
#endif
  
  if( fd < 0 ) {
    fprintf( stderr, "%s: open() failed trying to open device '%s':\n", PACKAGE,
	     audio_device );
    fprintf( stderr, "  %s\n", strerror( errno ) );
    fprintf( stderr,
	     "  Check if device file exists and has correct write permissions.\n" );
    ErrorLocation( __FILE__, __LINE__ );
    return -1;
  }
  
#if defined (__linux__)
  stereo = 0;
  status = ioctl( fd, SNDCTL_DSP_STEREO, &stereo );
  if( status == -1 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  status = ioctl( fd, SNDCTL_DSP_RESET, 0 );
  if( status > 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
	
  temp = 16;
  status = ioctl( fd, SOUND_PCM_WRITE_BITS, &temp );
  if( status != 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  status = ioctl( fd, SOUND_PCM_WRITE_CHANNELS, &channels );
  if( status != 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
	
  status = ioctl( fd, SOUND_PCM_WRITE_RATE, &srate );
  if( status != 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
	
  status = ioctl( fd, SNDCTL_DSP_SYNC, 0 );
  if( status != 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
#elif( defined(sun) && defined(unix) )
  /* Retrieve standard values. */
  AUDIO_INITINFO( &audio_info );
    
  audio_info.play.sample_rate = sfinfo.samplerate;
  audio_info.play.channels = sfinfo.channels;
  audio_info.play.precision = 16;
  audio_info.play.encoding = AUDIO_ENCODING_LINEAR;
  audio_info.play.gain = AUDIO_MAX_GAIN;
  audio_info.play.balance = AUDIO_MID_BALANCE;
  
  status = ioctl( audio_fd, AUDIO_SETINFO, &audio_info );
  if( status > 0 ) {
    fprintf( stderr, "%s: ioctl() failed: %s\n", PACKAGE, strerror( errno ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
#endif
  
  return fd;
}


void
PlayAudioFile( char *filename, int volume )
{
  static short buffer[BUFFER_LEN];
  SNDFILE *sndfile;
  SF_INFO sfinfo;
  int audio_fd;
  int readcount;
  int status;
#if defined (__linux__)
  int subformat;
  int m;
#elif( defined(sun) && defined(unix) )
  unsigned long	delay_time;
  long start_count, output_count, write_count;
  bool done;
#endif

  if( wmnotify_infos.debug ) {
    printf( "%s: PlayAudioFile() Entry\n", PACKAGE );
  }

  if( filename == NULL ) {
    fprintf( stderr, "%s: No audio file specified.\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  sndfile = sf_open( filename, SFM_READ, &sfinfo );
  
  if( sndfile == NULL ) {
    fprintf( stderr, "%s: sf_open() failed trying to open '%s':\n", PACKAGE, filename );
    fprintf( stderr, "  %s\n", sf_strerror(NULL) );
    fprintf( stderr, "  Check if file exists and has correct read permissions.\n" );
    ErrorLocation( __FILE__, __LINE__ );
    return;
  }
  
  if( sfinfo.channels < 1 || sfinfo.channels > 2 ) {
    fprintf( stderr, "%s: Audio file has %d channel(s), but ", PACKAGE, sfinfo.channels );
    fprintf( stderr, "we support only 1 or 2 channels.\n" );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  audio_fd = OpenDspDevice( sfinfo.channels, sfinfo.samplerate );
  if( audio_fd < 0 ) {
    goto play_audio_file_close_file;
  }

#if( defined(sun) && defined(unix) )
  /* Delay time equal to 1/4 of a buffer in microseconds. */
  delay_time = (BUFFER_LEN * 1000000) / (sfinfo.samplerate * 4);
#endif

  subformat = sfinfo.format & SF_FORMAT_SUBMASK;
    
  if( subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE ) {
    static float flt_buffer[BUFFER_LEN];
    double scale;
    
    status = sf_command( sndfile, SFC_CALC_SIGNAL_MAX, &scale, (int) sizeof(scale) );
    if( status == 0 ) {
      fprintf( stderr, "%s: Warning, sf_command() failed.\n", PACKAGE );
      ErrorLocation( __FILE__, __LINE__ );
      goto play_audio_file_close_audio;
    }

    if (scale < 1e-10) {
      scale = 1.0;
    }
    else {
      scale = 32700.0 / scale;
    }

    while( ( readcount = (int) sf_read_float( sndfile, flt_buffer, BUFFER_LEN ) ) != 0 ) {
    /* Linux/OSS -- FLOAT samples */
#if defined (__linux__)
      for( m = 0 ; m < readcount ; m++ ) {
	/* Float to integer conversion. */
	buffer[m] = (short) ( scale * flt_buffer[m] );
	/* Changing volume */
	buffer[m] = buffer[m] * volume / 100;
      }
      status = (int) write( audio_fd, buffer, readcount * sizeof(short) );
      if( status == -1 ) {
	perror( PACKAGE );
	ErrorLocation( __FILE__, __LINE__ );
	goto play_audio_file_close_audio;
      }
      
      /* Solaris -- FLOAT samples */
#elif( defined(sun) && defined(unix) )
      start_count = 0;
      output_count = read_count * sizeof(short);
      
      while( output_count > 0 ) {
	/* Write as much data as possible */
	for( m = 0 ; m < readcount ; m++ ) {
	  /* Float to integer conversion. */
	  buffer[m] = (short) ( scale * flt_buffer[m] );
	  /* Changing volume */
	  buffer[m] = buffer[m] * volume / 100;
	}
	
	write_count = write( audio_fd, &(buffer[start_count]), output_count );
	if( write_count > 0 ) {
	  output_count -= write_count;
	  start_count += write_count;
	}
	else {
	  /* Give the audio output time to catch up. */
	  usleep( delay_time );
	}
      } /* while( output_count > 0 ) */
#endif
    } /* while( ( readcount... ) */
  }
  else {
    while( ( readcount = (int) sf_read_short( sndfile, buffer, BUFFER_LEN ) ) != 0 ) {
      /* Linux/OSS -- INTEGER samples */
#if defined (__linux__)
      /* Changing volume... */
      for( m = 0 ; m < readcount ; m++ ) {
	buffer[m] = ( buffer[m] * volume ) / 100;
      }
      
      status = (int) write( audio_fd, buffer, readcount * sizeof(short) );
      if( status == -1 ) {
	perror( PACKAGE );
	ErrorLocation( __FILE__, __LINE__ );
	goto play_audio_file_close_audio;
      }
    
      /* Solaris -- INTEGER samples */
#elif( defined(sun) && defined(unix) )
      start_count = 0;
      output_count = read_count * sizeof(short);
      
      while( output_count > 0 ) {
	/* Write as much data as possible */
	
	/* Changing volume. */
	for( m = 0 ; m < read_count ; m++ ) {
	  buffer[m] = ( buffer[m] * volume ) / 100;
	}
	
	write_count = write( audio_fd, &(buffer[start_count]), output_count );
	if( write_count > 0 ) {
	  output_count -= write_count;
	  start_count += write_count;
	}
	else {
	  /* Give the audio output time to catch up. */
	  usleep( delay_time );
	}
      } /* while( output_count > 0 ) */
#endif
    } /* while( ( readcount... ) */
  } /* else */

 play_audio_file_close_audio:

  status = close( audio_fd );
  if( status != 0 ) {
    fprintf( stderr, "%s: Error, close() failed.\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }

 play_audio_file_close_file:

  status = sf_close( sndfile );
  if( status != 0 ) {
    fprintf( stderr, "%s: Error, sf_close() failed.\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  if( wmnotify_infos.debug ) {
    printf( "%s: PlayAudioFile() Exit\n", PACKAGE );
  }

  return;
}

#endif /* HAVE_SNDFILE */
