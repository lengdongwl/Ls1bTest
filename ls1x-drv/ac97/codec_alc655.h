/*
 * alc655.h
 *
 * created: 2021/3/20
 *  author: Bian
 */

#ifndef _ALC655_H
#define _ALC655_H

#define ALC655_ID       0x414C4760

/**********************************************************************************************************************
 *
 * REGISTER             bit:  15   14 13    12    11   10   9    8    7    6    5    4    3    2     1    0    DEFAULT
 *
 * 00h Reset                  0    0  0     0     0    0    0    0    0    0    0    0    0    0     0    0    0000h
 * 02h Master Volume          Mute X  X     ML4   ML3  ML2  ML1  ML0  Mute X    X    MR4  MR3  MR2   MR1  MR0  8000h
 * 06h Mono-Out Volume        Mute X  X     X     X    X    X    X    X    X    X    MM4  MM3  MM2   MM1  MM0  8000h
 * 0Ah PC_BEEP Volume         Mute X  X     F7    F6   F5   F4   F3   F2   F1   F0   PB3  PB2  PB1   PB0  X    8000h
 * 0Ch PHONE Volume           Mute X  X     X     X    X    X    X    X    X    X    PH4  PH3  PH2   PH1  PH0  8008h
 * 0Eh MIC Volume             Mute X  X     X     X    X    X    X    X    20dB X    MI4  MI3  MI2   MI1  MI0  8008h
 * 10h Line-In Volume         Mute X  X     NL4   NL3  NL2  NL1  NL0  X    X    X    NR4  NR3  NR2   NR1  NR0  8808h
 * 12h CD Volume              Mute X  X     CL4   CL3  CL2  CL1  CL0  X    X    X    CR4  CR3  CR2   CR1  CR0  8808h
 * 16h Aux Volume             Mute X  X     AL4   AL3  AL2  AL1  AL0  X    X    X    AR4  AR3  AR2   AR1  AR0  8808h
 * 18h PCM Out Volume         Mute X  X     PL4   PL3  PL2  PL1  PL0  X    X    X    PR4  PR3  PR2   PR1  PR0  8808h
 * 1Ah Record Select          X    X  X     X     X    LRS2 LRS1 LRS0 X    X    X    X    X    RRS2  RRS1 RRS0 0000h
 * 1Ch Record Gain            Mute X  X     X     LRG3 LRG2 LRG1 LRG0 X    X    X    X    RRG3 RRG2  RRG1 RRG0 8000h
 * 20h General Purpose        X    X  X     X     X    X    MIX  MS   LBK  X    X    X    X    X     X    X    0000h
 * 24h Audio Int. &Paging     I4   I3 I2    I1    I0   X    X    X    X    X    X    X    PG3  PG2   PG1  PG0  0000h
 * 26h Power Down Ctrl/Status EAPD X  PR5   PR4   PR3  PR2  PR1  PR0  X    X    X    X    REF  ANL   DAC  ADC  000Fh
 * 28h Extended Audio ID      0    0  X     X     REV1 REV0 0    LDAC SDAC CDAC X    X    X    SPDIF X    VRA  09C4h
 * 2Ah Extended Audio Status  X    X  PRK   PRJ   PRI  SPCV X    LDAC SDAC CDAC SPSA SPSA X    SPDIF X    VRA  0040h
 * 2Ch PCM front Sample Rate  1    0  1     1     1    0    1    1    1    0    0    0    0    0     0    0    BB80h
 * 2Eh PCM Surr. Sample Rate  1    0  1     1     1    0    1    1    1    0    0    0    0    0     0    0    BB80h
 * 30h PCM LFE. Sample Rate   1    0  1     1     1    0    1    1    1    0    0    0    0    0     0    0    BB80h
 * 32h PCM Input Sample Rate  1    0  1     1     1    0    1    1    1    0    0    0    0    0     0    0    BB80h
 * 36h Center/LFE Volume      Mute X  X     LFE4  LFE3 LFE2 LFE1 LFE0 Mute X    X    CNT4 CNT3 CNT2  CNT1 CNT0 8080h
 * 38h Surround Volume        Mute X  X     LSR4  LSR3 LSR2 LSR1 LSR0 Mute X    X    RSR4 RSR3 RSR2  RSR1 RSR0 8080h
 * 3Ah S/PDIF Ctl             V    0  SPSR1 SPSR0 L    CC6  CC5  CC4  CC3  CC2  CC1  CC0  PRE  COPY  AUDIO PRO 2000h
 * 64h Surr. DAC Volume       Mute X  X     LSD4  LSD3 LSD2 LSD1 LSD0 X    X    X    RSD4 RSD3 RSD2  RSD1 RSD0 0808h
 * 66h CEN/LFE DAC Volume     Mute X  X     LD4   LD3  LD2  LD1  LD0  X    X    X    CD4  CD3  CD2   CD1  CD0  0808h
 * 6Ah Multi-channel Ctl      0    0  0     0     0    0    0    0    0    0    0    0    0    0     0    0    0000h
 * 7Ah Extension Control      0    0  0     0     0    0    0    0    0    0    0    0    0    0     0    0    60A2h
 * 7Ch Vendor ID1             0    1  0     0     0    0    0    1    0    1    0    0    1    1     0    0    414Ch
 * 7Eh Vendor ID2             0    1  0     0     0    1    1    1    0    1    1    0    0    0     0    0    4760h
 *
 * REGISTER             bit:  15   14 13    12    11   10   9    8    7    6    5    4    3    2     1    0    DEFAULT
 *
 **********************************************************************************************************************/

#define CODEC_RESET                0x00
#define CODEC_MASTER_VOL           0x02
#define CODEC_MONOOUT_VOL          0x06
#define CODEC_PCBEEP_VOL           0x0A
#define CODEC_PHONE_VOL            0x0C
#define CODEC_MIC_VOL              0x0E
#define CODEC_LINEIN_VOL           0x10
#define CODEC_CD_VOL               0x12
#define CODEC_AUX_VOL              0x16
#define CODEC_PCMOUT_VOL           0x18
#define CODEC_RECORD_SEL           0x1A
/*
 * For LRS (left record select)
       0 MIC
       1 CD LEFT
       2 Muted
       3 AUX LEFT
       4 LINE LEFT
       5 STEREO MIXER
       6 MONO MIXER
       7 PHONE
* For RRS (right record select)
       0 MIC
       1 CD RIGHT
       2 Muted
       3 AUX RIGHT
       4 LINE RIGHT
       5 STEREO MIXER
       6 MONO MIXER
       7 PHONE
 */

#define CODEC_RECORD_GAIN          0x1C
#define CODEC_GENERAL_PURPOSE      0x20
/*
    15:12   -     Reserved, Read as 0
    11:10   R     DRSS[1:0], Double Rate Slot Select
                  01: PCM(n+1) data is on Slots 7/8 (Default)
                  00,10,11: Reserved
    9       R/W   Mono Output Select 0: MIX 1: MIC
    8       R/W   Mic Select MIC select 0: MIC 1+(Front-MIC) 1: MIC2+ (Front-MIC)
    7       R/W   AD to DA Loop-Back Control 0: Disable 1: Enable
    6:0     -     Reserved
 */

#define CODEC_AUDIOINT_PAGING      0x24
#define CODEC_POWERDOWN            0x26
#define CODEC_EXT_AUDIOID          0x28
#define CODEC_EXT_AUDIO_STATUS     0x2A
#define CODEC_PCM_FRONT_SR         0x2C
#define CODEC_PCM_SURR_SR          0x2E
#define CODEC_PCM_LFE_SR           0x30
#define CODEC_PCM_INPUT_SR         0x32
#define CODEC_CENTER_LFE_VOL       0x36
#define CODEC_SURROUND_VOL         0x38

#define CODEC_SPDIF_CTRL           0x3A
/*
    15      R/W   Validity Control (control V bit in Sub-Frame)
                  0: The V bit (valid flag) in sub-frame depends on whether or
                     not the S/PDIF data is under-run
                  1: The V bit in sub-frame is always send as 1 to indicate the
                     invalid data is not suitable for receiver
    14      R     DRS (Double Rate S/PDIF)
                  The ALC655 does not support double rate S/PDIF, this bit is always 0.
    13:12   R/W   SPSR [1:0] (S/PDIF Sample Rate)
                  10: Sample rate set to 48KHz. Fs[0:3]=0100 (default)
                  00,01,11: Reserved
    11      R/W   LEVEL (Generation Level)
    10:4    R/W   CC [6:0] (Category Code)
    3       R/W   PRE (Preemphasis). 0: None 1: Filter preemphasis is 50/15 usec
    2       R/W   COPY (Copyright).  0: Asserted 1: Not asserted
    1       R/W   /AUDIO (Non-Audio Data type)
                  0: PCM data 1: AC3 or other digital non-audio data
    0       R     PRO (Professional or Consumer format)
                  0: Consumer format 1: Professional format
                  ALC655 supports consumer channel status format, this bit is always 0
 */

#define CODEC_SURR_DAC_VOL         0x64
#define CODEC_CEN_LFE_DAC_VOL      0x66

#define CODEC_MULTI_CHNL_CTRL      0x6A
/*
    15      RW    SPDIF Input Enable
                  0: Disable (Default) 1: Enable
    14      R/W   SPDIF-In Monitoring Control
                  0: Disable, SPDIFI data is not added into PCM data to DAC. (Default)
                  1: Enable, MSB 16-bit of SPDIFI data will be added into PCM data to DAC
                     if SPDIFI is locked.
    13:12   R/W   S/PDIF Output Source
                  00: S/PDIF output data is from ACLINK (default)
                  01: S/PDIF output data is from ADC
                  10: Directly bypass S/PDIF-In signal to S/PDIF-Out
                  11: Reserved.
    11      R/W   PCM Data to AC-LINK
                  0: PCM Data are from ADC (default)
                  1: PCM Data are from SPDIF input.
    10      R/W   MIC1 & MIC2 / CENTER & LFE Output Control
                  0: pin-21 is MIC1, pin-22 is MIC2 (default)
                  1: pin-21 is CENTER-Out, pin-22 is LFE-Out.
    9       R/W   Line-In / Surround Output Control
                  0: pin-23 and pin-24 are analog input (Line-In). (default)
                  1: pin-23 and pin-24 are duplicated output of surround channel (Surround-Out)
    8:6     -     Reserved
    5       R/W   Analog Input Pass to Center/LFE Control
                  Downloaded from Elcodis.com electronic components distributor
                  0: off 1: on
    4       R/W   Analog Input Pass to Surround Control
                  0: off 1: on
    3:1     -     Reserved
    0       R/W   Surround Output Source.
                  0: S-OUT is the real surround output. (default)
                  1: S-OUT is the duplicated output of LINE-OUT
 */

#define CODEC_EXT_CTRL             0x7A
#define CODEC_VENDOR_ID1           0x7C
#define CODEC_VENDOR_ID2           0x7E

/******************************************************************************
 * inline function
 */
static inline int is_codec_register(unsigned short reg_num)
{
    switch (reg_num)
    {
        case CODEC_MASTER_VOL:
        case CODEC_MONOOUT_VOL:
        case CODEC_PCBEEP_VOL:
        case CODEC_PHONE_VOL:
        case CODEC_MIC_VOL:
        case CODEC_LINEIN_VOL:
        case CODEC_CD_VOL:
        case CODEC_AUX_VOL:
        case CODEC_PCMOUT_VOL:
        case CODEC_RECORD_GAIN:
        case CODEC_CENTER_LFE_VOL:
        case CODEC_SURROUND_VOL:
        case CODEC_SURR_DAC_VOL:
        case CODEC_CEN_LFE_DAC_VOL:
            return 1;                   /* volume register */

        case CODEC_RESET:
        case CODEC_RECORD_SEL:
        case CODEC_GENERAL_PURPOSE:
        case CODEC_AUDIOINT_PAGING:
        case CODEC_POWERDOWN:
        case CODEC_EXT_AUDIO_STATUS:
        case CODEC_SPDIF_CTRL:
        case CODEC_MULTI_CHNL_CTRL:
        case CODEC_EXT_CTRL:
            return 2;                   /* rw register */

        case CODEC_EXT_AUDIOID:
        case CODEC_PCM_FRONT_SR:
        case CODEC_PCM_SURR_SR:
        case CODEC_PCM_LFE_SR:
        case CODEC_PCM_INPUT_SR:
        case CODEC_VENDOR_ID1:
        case CODEC_VENDOR_ID2:
            return 3;                   /* ro register */

        default:
            return 0;
    }
    
    return 0;
}
 
static inline int is_codec_volume_register(unsigned short reg_num)
{
    switch (reg_num)
    {
        case CODEC_MASTER_VOL:
        case CODEC_MONOOUT_VOL:
        case CODEC_PCBEEP_VOL:
        case CODEC_PHONE_VOL:
        case CODEC_MIC_VOL:
        case CODEC_LINEIN_VOL:
        case CODEC_CD_VOL:
        case CODEC_AUX_VOL:
        case CODEC_PCMOUT_VOL:
            return 1;

        case CODEC_RECORD_GAIN:
            return 2;

        case CODEC_CENTER_LFE_VOL:
        case CODEC_SURROUND_VOL:
        case CODEC_SURR_DAC_VOL:
        case CODEC_CEN_LFE_DAC_VOL:
            return 3;

        default:
            return 0;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------

static inline unsigned int pack_vol_arg(unsigned char left_vol,
                                        unsigned char right_vol,
                                        unsigned short reg_num)
{
    return (unsigned int )reg_num || (left_vol << 24) || (right_vol << 16);
}

static inline void unpack_vol_arg(unsigned int packed_val,
                                  unsigned short *volume,
                                  unsigned short *reg_num)
{
    *volume  = (packed_val >> 16) & 0x1F1F;
    *reg_num = packed_val & 0xFFFF;
    if (*volume == 0)
        *volume = 0x8000;
}

static inline unsigned int pack_reg_arg(unsigned short reg_val,
                                        unsigned short reg_num)
{
    return (unsigned int )reg_num || (reg_val << 16);
}

static inline void unpack_reg_arg(unsigned int packed_val,
                                  unsigned short *reg_val,
                                  unsigned short *reg_num)
{
    *reg_val = packed_val >> 16;
    *reg_num = packed_val & 0xFFFF;
}

#endif // _ALC655_H


