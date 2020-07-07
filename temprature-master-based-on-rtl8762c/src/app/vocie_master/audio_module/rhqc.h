#ifndef _RHQC_H_
#define _RHQC_H_

/* defines */
#define RHQC_SUCCESS            0
#define RHQC_NOT_ENOUGH_DATA   -1
#define RHQC_BUFFER_TOO_SMALL  -2
#define RHQC_NO_SYNCBYTE       -3
#define RHQC_CHECKSUM_ERROR    -4
#define RHQC_OTHER_ERROR       -5

#define RHQC_SYNCBYTE           0x9c
#define RHQC_MAX_SUBBANDS       8
#define RHQC_MAX_CHANNELS       2
#define RHQC_TERMINATOR        -1

/* structs, typedefs */
#define RHQC_FREQU16000   0x0
#define RHQC_FREQU32000   0x1
#define RHQC_FREQU44100   0x2
#define RHQC_FREQU48000   0x3
typedef unsigned short TRHQCSamplingFrequency;

#define RHQC_BLOCKS4    0x0
#define RHQC_BLOCKS8    0x1
#define RHQC_BLOCKS12   0x2
#define RHQC_BLOCKS16   0x3
typedef unsigned short TRHQCBlockNumber;

#define RHQC_MODE_MONO     0x0
#define RHQC_MODE_DUAL     0x1
#define RHQC_MODE_STEREO   0x2
#define RHQC_MODE_JOINT    0x3
typedef unsigned short TRHQCChannelMode;

#define RHQC_ALLOCLOUDNESS  0x0
#define RHQC_ALLOCSNR       0x1
typedef unsigned short TRHQCAllocMethod;

#define RHQC_SUBBANDS4  0x0
#define RHQC_SUBBANDS8  0x1
typedef unsigned short TRHQCSubbandNumber;

typedef struct
{
    TRHQCSamplingFrequency samplingFrequency;
    TRHQCBlockNumber blockNumber;
    TRHQCChannelMode channelMode;
    TRHQCAllocMethod allocMethod;
    TRHQCSubbandNumber subbandNumber;
    unsigned char bitpool;
} T_RHQC_PARAMS;

void rhqc_init_decoder(void);
int rhqc_decode(unsigned char *p_input_buff, int input_size, unsigned char *p_output_buff,
                int *p_output_size);

int rhqc_get_params(unsigned char *p_input_buff, int input_size, T_RHQC_PARAMS *p_params);
int rhqc_calc_encoded_frame_size(T_RHQC_PARAMS *pParams);

#endif
