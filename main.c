///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          ee469_lab01_dtmf_wav_gen - EE 469 - Fall 2022
//
/// Generate a .wav file containing DTMF dialing tones
///
/// @see https://docs.fileformat.com/audio/wav/
///
/// @file main.c
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   04_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   // For printf(), fopen(), etc.
#include <stdlib.h>  // For EXIT_SUCCESS
#include <assert.h>  // For assert()
#include <stdint.h>  // For fixed-length ints
#include <math.h>    // For sin()

#define PROGRAM_NAME "ee469_lab01_dtmf_wav_gen"
#define FILENAME     "/home/mark/src/tmp/blob.wav"

#define CHANNELS            1     /* 1 for mono   2 for stereo         */
#define SAMPLE_RATE      8000     /* Samples per second                */
#define BYTES_PER_SAMPLE    1     /* (BitsPerSample * Channels) / 8
                                     1 - 8 bit mono
                                     2 - 8 bit stereo/16 bit mono
                                     4 - 16 bit stereo                 */
#define BITS_PER_SAMPLE     8     /* 256 possible values               */
#define DURATION_IN_SECONDS 2     /* Each DTMF tone is 2 seconds long  */
#define AMPLITUDE           0.1   /* Max amplitude of signal           */

#define TONE                500

FILE *gFile = NULL;               /* Global file pointer to FILENAME */

uint32_t gPCM_data_size = 0;      /* Global counter of the bytes written */

void open_audio_file() {
   assert( gFile == NULL );

   gFile = fopen( FILENAME, "w+" );
   if( gFile == NULL ) {
      printf(PROGRAM_NAME ": Could not open file [%s].  Exiting.\n", FILENAME );
      exit( EXIT_FAILURE );
   }

   assert( gFile != NULL );

   gPCM_data_size = 0;

   /// Marks file as a RIFF file
   fwrite( "RIFF", 1, 4, gFile );  /// @todo Add error checking

   uint32_t file_size = 44 + gPCM_data_size;  // 44 is the actual size of the header
                                              // (regardless of what you see below)
                                              // This gets overwritten later
   fwrite( &file_size, 1, 4, gFile );  /// File size

   fwrite( "WAVE", 1, 4, gFile );      /// File type header

   fwrite( "fmt ", 1, 4, gFile );      /// Format chunk marker (includes trailing space)

   uint32_t header_length = 16;     /// The length of the header (above)
   fwrite( &header_length, 1, 4, gFile );

   uint16_t format_code = 1;        /// 1==PCM
   fwrite( &format_code, 1, 2, gFile );

   uint16_t channels = CHANNELS;
   fwrite( &channels, 1, 2, gFile );

   uint32_t sample_rate = SAMPLE_RATE;
   fwrite( &sample_rate, 1, 4, gFile );

   uint32_t stream_rate = ( SAMPLE_RATE * BITS_PER_SAMPLE * CHANNELS) / 8;  /// (Sample Rate * BitsPerSample * Channels) / 8
   fwrite( &stream_rate, 1, 4, gFile );

   uint16_t bytes_per_sample = BYTES_PER_SAMPLE;
   fwrite( &bytes_per_sample, 1, 2, gFile );

   uint16_t bits_per_sample = BITS_PER_SAMPLE;
   fwrite( &bits_per_sample, 1, 2, gFile );

   fwrite( "data", 1, 4, gFile );   /// Marks the beginning of the data section.

   fwrite( &gPCM_data_size, 1, 4, gFile );
}


void write_audio() {
   assert( gFile != NULL );

   uint32_t index = 0;
   uint32_t samples = DURATION_IN_SECONDS * SAMPLE_RATE;

   double cadence = SAMPLE_RATE / TONE / 2.0 / M_PI;

   double s;
   uint8_t sample;

   while( index < samples ) {
      s = sin( index / cadence );
      sample = (uint8_t) 128 + ( s * 256 * AMPLITUDE );

      size_t bytes_written = fwrite( &sample, 1, 1, gFile );
      if( bytes_written != 1 ) {
         printf(PROGRAM_NAME ": Unable to stream PCM to [%s].  Exiting.\n", FILENAME );
         exit( EXIT_FAILURE );
      }

      gPCM_data_size++;
      index++;
   }
}


void close_audio_file() {
   assert( gFile != NULL );

   fseek( gFile, 4, SEEK_SET );  /// Seek to the File Size field

   uint32_t file_size = 44 + gPCM_data_size;  // 44 is the actual size of the header

   fwrite( &file_size, 1, 4, gFile );         /// File size

   fseek( gFile, 42, SEEK_SET );  /// Seek to the Size of the Data Section field

   fwrite( &gPCM_data_size, 1, 4, gFile );

   if( gFile != NULL ) {
      fclose( gFile );
      gFile = NULL;
   }
}


int main() {
   printf( PROGRAM_NAME ": Starting.  Writing to [%s]\n", FILENAME );

   open_audio_file();

   write_audio();

   close_audio_file();

   printf( PROGRAM_NAME ": Ends successfully\n" );
   return EXIT_SUCCESS;
}
