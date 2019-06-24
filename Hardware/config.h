#ifndef __CONFIG_H
#define __CONFIG_H

#define TEST_MODE 

#define POW_CHECK_INTERVAL	10000 //in ms

#define SIGNAL_FLAG_DEBUG_IRQ		0x8000
#define SIGNAL_FLAG_DECA_IRQ	0x2000
#define SINGAL_FLAG_TDOA	0x4000

#define LOR_FREQ_HB 0xb8
#define LOR_FREQ_MB	0x9d
#define LOR_FREQ_LB 0x8a
#define LOR_SF_5 0x50
#define LOR_SF_6 0x60
#define LOR_SF_8 0x80
#define LOR_BW_1600 0x0a
#define LOR_CR_4_5 0x01
#define LOR_CR_LI_4_5 0x05

#define LOW_ANCHOR_MAX 16
#define LOW_TAG_ID	9
#define LOW_START_MARGIN 2
#define LOW_WAIT_SYNC_TIMEOUT 50
#define LOW_DATA_MAX 10
#define LOW_BUFFER_SIZE	100

#define DEC_POWER_DA	0x00	//0 18 dB£¬111 off
#define DEC_POWER_MIXER	0x1f	//11111 15.5 dB
#define DEC_FRAME_LEN 20

#endif

