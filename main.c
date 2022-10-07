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
#include <string.h>  // For strlen()

#define PROGRAM_NAME "ee469_lab01_dtmf_wav_gen"
#define FILENAME     "/home/mark/src/tmp/blob.wav"

#define CHANNELS            1     /* 1 for mono   2 for stereo         */
#define SAMPLE_RATE      8000     /* Samples per second                */
#define BYTES_PER_SAMPLE    1     /* (BitsPerSample * Channels) / 8    *
                                   *   1 - 8 bit mono                  *
                                   *   2 - 8 bit stereo/16 bit mono    *
                                   *   4 - 16 bit stereo               */
#define BITS_PER_SAMPLE     8     /* 256 possible values               */
#define DURATION_IN_MS    200     /* Duration in milliseconds          */
#define SILENCE_IN_MS     100     /* The pause between each DTMF tone  */
#define AMPLITUDE           0.5   /* Max amplitude of signal, relative *
                                   * to the maximum scale              */

#define PCM_8_BIT_SILENCE 128     /* Silence is 128                    */

#define DTMF_ROW_1        697
#define DTMF_ROW_2        770
#define DTMF_ROW_3        852
#define DTMF_ROW_4        941

#define DTMF_COL_1       1209
#define DTMF_COL_2       1336
#define DTMF_COL_3       1477
#define DTMF_COL_4       1633


FILE *gFile = NULL;               /* Global file pointer to FILENAME */

uint32_t gPCM_data_size = 0;      /* Global counter of the bytes written */


/// Wrap the fwrite() function with error checking
size_t fwrite_ex( const void *ptr, size_t size, size_t members, FILE *stream ) {
   size_t expected_size = size * members;

   size_t return_value = fwrite( ptr, size, members, stream );

   if( expected_size != return_value ) {
      printf( PROGRAM_NAME ": Unable to stream PCM to [%s].  Exiting.\n", FILENAME );
      exit( EXIT_FAILURE );
   }

   return return_value;
}


/// Open the .wav file and write the header.
///
/// There will be fields in the header that will be updated when the file
/// is closed.
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
   fwrite_ex( "RIFF", 1, 4, gFile );

   uint32_t file_size = 44 + gPCM_data_size;  // 44 is total size of the entire header
                                              // This gets overwritten later

   fwrite_ex( &file_size, 1, sizeof(uint32_t), gFile );  /// File size

   fwrite_ex( "WAVE", 1, sizeof(uint32_t), gFile );      /// File type header

   fwrite_ex( "fmt ", 1, sizeof(uint32_t), gFile );      /// Format chunk marker (includes trailing space)

   uint32_t header_length = 16;        /// The length of the RIFF header
   fwrite_ex( &header_length, 1, sizeof(uint32_t), gFile );

   uint16_t format_code = 1;           /// 1==PCM
   fwrite_ex( &format_code, 1, sizeof(uint16_t), gFile );

   uint16_t channels = CHANNELS;
   fwrite_ex( &channels, 1, sizeof(uint16_t), gFile );

   uint32_t sample_rate = SAMPLE_RATE;
   fwrite_ex( &sample_rate, 1, sizeof(uint32_t), gFile );

   uint32_t stream_rate = ( SAMPLE_RATE * BITS_PER_SAMPLE * CHANNELS) / 8;  /// (Sample Rate * BitsPerSample * Channels) / 8
   fwrite_ex( &stream_rate, 1, sizeof(uint32_t), gFile );

   uint16_t bytes_per_sample = BYTES_PER_SAMPLE;
   fwrite_ex( &bytes_per_sample, 1, sizeof(uint16_t), gFile );

   uint16_t bits_per_sample = BITS_PER_SAMPLE;
   fwrite_ex( &bits_per_sample, 1, sizeof(uint16_t), gFile );

   fwrite_ex( "data", 1, 4, gFile );   /// Marks the beginning of the data section

   fwrite_ex( &gPCM_data_size, 1, sizeof(uint32_t), gFile );
}


/// At time index, return a raw tone sample at a given frequency
///
/// @param frequency The tone's frequency in Hz
/// @param index The time reference
///
/// @returns A value from -1 to 1
double generate_tone( uint32_t index, uint32_t frequency ) {
   assert( frequency != 0 );

   double cadence = (double) SAMPLE_RATE / frequency / 2.0 / M_PI;

   return sin( index / cadence );  /// % of duty cycle
}


/// Mix the two tones by averaging them together.  In linear digital audio,
/// this has the effect of attenuating the volume of each tone by 1/2.
///
/// @param f1 The value of the 1st tone (from -1 to 1)
/// @param f2 The value of the 2nd tone (from -1 to 1)

/// @return The mixed signal (from -1 to 1)
double mix_tones( double f1, double f2 ) {
   assert( f1 >= -1 && f1 <= 1);
   assert( f2 >= -1 && f2 <= 1);

   return ( (f1 + f2) / 2 );  /// Average the two samples
}


/// Generate a DTMF signal for DURATION_IN_MS and write it to the .wav file
///
/// @param DTMF_digit as an ASII character.  Valid values are:
///        0 through 9, *, # and a through d (case insensitive)
void write_DTMF_tone( char DTMF_digit ) {
   assert( gFile != NULL );   /// Assume gFile is open

   uint32_t DTMF_row = 0;
   uint32_t DTMF_column = 0;

   switch( DTMF_digit ) {
      case '0':
         DTMF_row = DTMF_ROW_4;
         DTMF_column = DTMF_COL_2;
         break;
      case '1':
         DTMF_row = DTMF_ROW_1;
         DTMF_column = DTMF_COL_1;
         break;
      case '2':
         DTMF_row = DTMF_ROW_1;
         DTMF_column = DTMF_COL_2;
         break;
      case '3':
         DTMF_row = DTMF_ROW_1;
         DTMF_column = DTMF_COL_3;
         break;
      case '4':
         DTMF_row = DTMF_ROW_2;
         DTMF_column = DTMF_COL_1;
         break;
      case '5':
         DTMF_row = DTMF_ROW_2;
         DTMF_column = DTMF_COL_2;
         break;
      case '6':
         DTMF_row = DTMF_ROW_2;
         DTMF_column = DTMF_COL_3;
         break;
      case '7':
         DTMF_row = DTMF_ROW_3;
         DTMF_column = DTMF_COL_1;
         break;
      case '8':
         DTMF_row = DTMF_ROW_3;
         DTMF_column = DTMF_COL_2;
         break;
      case '9':
         DTMF_row = DTMF_ROW_3;
         DTMF_column = DTMF_COL_3;
         break;
      case '*':
         DTMF_row = DTMF_ROW_4;
         DTMF_column = DTMF_COL_1;
         break;
      case '#':
         DTMF_row = DTMF_ROW_4;
         DTMF_column = DTMF_COL_3;
         break;
      case 'A':
      case 'a':
         DTMF_row = DTMF_ROW_1;
         DTMF_column = DTMF_COL_4;
         break;
      case 'B':
      case 'b':
         DTMF_row = DTMF_ROW_2;
         DTMF_column = DTMF_COL_4;
         break;
      case 'C':
      case 'c':
         DTMF_row = DTMF_ROW_3;
         DTMF_column = DTMF_COL_4;
         break;
      case 'D':
      case 'd':
         DTMF_row = DTMF_ROW_4;
         DTMF_column = DTMF_COL_4;
         break;
      // No default case statement... The next statement does the error checking
   }

   if( DTMF_row == 0 || DTMF_column == 0) {
      printf( PROGRAM_NAME ": Unknown DTMF tone character [%c].  Skipping.\n", DTMF_digit );
      return;
   }

   assert( DTMF_row > 0 );
   assert( DTMF_column > 0 );

   uint32_t index = 0;
   uint32_t samples = (uint32_t) (( (double) DURATION_IN_MS / 1000.0) * SAMPLE_RATE);

   while( index < samples ) {
      double s;  // Raw sound as -1 to 1
      s = mix_tones( generate_tone( index, DTMF_row ), generate_tone( index, DTMF_column ) );

      // Convert -1 to 1 into a linear PCM representation
      uint8_t PcmSample = (uint8_t) (PCM_8_BIT_SILENCE + ( s * PCM_8_BIT_SILENCE * AMPLITUDE ));

      // Write PcmSample to the .wav file
      fwrite_ex( &PcmSample, 1, 1, gFile );

      gPCM_data_size++;
      index++;
   }

   printf( PROGRAM_NAME ": Generated DTMF digit [%c] at tones [%d] and [%d].\n", DTMF_digit, DTMF_row, DTMF_column );
}


/// Write silence for SILENCE_IN_MS to the .wav file
void write_silence() {
   uint32_t index = 0;
   uint32_t samples = ( SILENCE_IN_MS / 1000.0 ) * SAMPLE_RATE;

   while( index < samples ) {
      uint8_t PcmSample = (uint8_t) PCM_8_BIT_SILENCE;  // Silence

      fwrite_ex( &PcmSample, 1, 1, gFile );

      gPCM_data_size++;
      index++;
   }
}


/// Write a sawtooth signal (0, 1, 2, ... 254, 255, 0, 1, 2...) to the .wav file
///
/// It's a fixed frequency that depends on the sampling rate and bit depth
///
/// This is mostly used for diagnostics
///
/// @param duration_in_ms Milliseconds of signal
///
void write_sawtooth_tone( uint32_t duration_in_ms ) {
   assert( gFile != NULL );

   uint32_t index = 0;
   uint32_t samples = (uint32_t) (((float) duration_in_ms / 1000.0) * SAMPLE_RATE);

   while( index < samples ) {
      uint8_t PcmSample = index % 256;

      fwrite_ex( &PcmSample, 1, 1, gFile );

      gPCM_data_size++;
      index++;
   }
}


/// Write a sinusoidal signal to the .wav file
///
/// This is mostly used for diagnostics
///
/// @param frequency      Frequency of the tone
/// @param duration_in_ms Duration of the tone in milliseconds
void write_sinwave_tone( uint32_t frequency, uint32_t duration_in_ms ) {
   assert( gFile != NULL );
   assert( frequency != 0 );

   uint32_t index = 0;
   uint32_t samples = (uint32_t) (((double) duration_in_ms / 1000.0) * SAMPLE_RATE);

   while( index < samples ) {
      double s = generate_tone( index, frequency );  // Raw sound
      uint8_t PcmSample = (uint8_t) (PCM_8_BIT_SILENCE + ( s * PCM_8_BIT_SILENCE * AMPLITUDE ));

      fwrite_ex( &PcmSample, 1, sizeof(uint8_t), gFile );

      gPCM_data_size++;
      index++;
   }
}


/// Close the .wav file
///
/// Seek back to the header and update 2 fields
void close_audio_file() {
   assert( gFile != NULL );

   fseek( gFile, 4, SEEK_SET );  /// Seek to the File Size field

   uint32_t file_size = 44 + gPCM_data_size;  // 44 is the actual size of the header

   fwrite_ex( &file_size, 1, sizeof(uint32_t), gFile );         /// File size

   fseek( gFile, 42, SEEK_SET );  /// Seek to the Size of the Data Section field

   fwrite_ex( &gPCM_data_size, 1, sizeof(uint32_t), gFile );

   if( gFile != NULL ) {
      fclose( gFile );
      gFile = NULL;
   }
}


/// Process dtmf_string and write the digits to a .wav file.  Separate each
/// digit by some silence.
void do_dtmf_digits( char* dtmf_string ) {
   if( dtmf_string == NULL ) {
      printf( PROGRAM_NAME ": Empty DTMF String.  Nothing to do.\n" );
      return;
   }

   for( int i = 0 ; i < strlen( dtmf_string ) ; i++ ) {
      write_DTMF_tone( dtmf_string[i] );
      write_silence();
   }
}


/// Program entry point
int main() {
   printf( PROGRAM_NAME ": Starting.  Writing to [%s]\n", FILENAME );

   open_audio_file();

   do_dtmf_digits( "0123456789*#abcd" );
   //do_dtmf_digits( "1111" );

   //write_sinwave_tone( 500, 2000 );  // 500Hz tone for 2 seconds
   //write_sawtooth_tone( 2000 );

   close_audio_file();

   printf( PROGRAM_NAME ": Ends successfully\n" );
   return EXIT_SUCCESS;
}
