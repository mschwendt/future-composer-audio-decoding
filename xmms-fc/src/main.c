/*
 * AMIGA Future Composer music player plugin for XMMS
 * Copyright (C) 2000 Michael Schwendt <mschwendt@users.sf.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <xmms/plugin.h>
#include <xmms/util.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fc14audiodecoder.h>

#include "config.h"
#include "about.h"
#include "configure.h"

struct audioFormat {
    AFormat xmmsAFormat;
    int bits, freq, channels;
    int zeroSample;
} myFormat;

static void ip_init(void);
static int ip_is_valid_file(char *fileName);
static void ip_play_file(char *fileName);
static void ip_stop(void);
static void ip_pause(short p);
static void ip_seek(int t);
static int ip_get_time(void);
static void ip_get_song_info(char *fileName, char **title, int *length);

static pthread_t decode_thread;

InputPlugin iplugin = {
    NULL,
    NULL,
    "Future Composer decoder ",
    ip_init,
	fc_ip_about,
    fc_ip_configure,
    ip_is_valid_file,
    NULL,
    ip_play_file,
    ip_stop,
    ip_pause,
    ip_seek,
    NULL,
    ip_get_time,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ip_get_song_info,
    NULL,
    NULL
};

InputPlugin *get_iplugin_info(void) {
    return &iplugin;
}

static void *decoder = NULL;
static size_t sampleBufSize = 0;
static gpointer *sampleBuf = NULL;

static int playing = 0;
static int jumpToTime = -1;

static void deleteSampleBuf()
{
    if (sampleBuf != NULL)
    {
        g_free(sampleBuf);
        sampleBuf = NULL;
        sampleBufSize = 0;
    }
}

/* Microsecs to sleep when audio driver does not accept new
   sample buffer contents. */
static int sleepVal;

static void *play_loop(void *arg)
{
    int stop = 0;
    while ( playing ) {
        if ( stop ) {
            memset(sampleBuf,0,sampleBufSize);
        }
        if ( jumpToTime != -1 ) {
            fc14dec_seek(decoder,jumpToTime);
            iplugin.output->flush(jumpToTime);
            jumpToTime = -1;
            stop = 0;
        }
        if ( !stop ) {
            fc14dec_buffer_fill(decoder,sampleBuf,sampleBufSize);
            iplugin.add_vis_pcm(iplugin.output->written_time(),
                                myFormat.xmmsAFormat, myFormat.channels,
                                sampleBufSize, sampleBuf);
            while ( iplugin.output->buffer_free() < sampleBufSize && playing )
                xmms_usleep( sleepVal );
            if ( playing && jumpToTime<0 )
                iplugin.output->write_audio(sampleBuf,sampleBufSize);
            if ( fc14dec_song_end(decoder) && jumpToTime<0 ) {
                iplugin.output->buffer_free();
                iplugin.output->buffer_free();
                while ( iplugin.output->buffer_playing() && playing && jumpToTime<0 ) {
                    xmms_usleep(10000);
                }
                stop = 1;
            }
        }
        else {
            xmms_usleep( 10000 );
        }
    }
    pthread_exit(NULL);
}

static void ip_init(void) {
    playing = 0;
    sampleBuf = NULL;
    sampleBufSize = 0;
    
    fc_ip_load_config();
}

static int ip_is_valid_file(char* fileName) {
    void *dec;
	unsigned char magicBuf[5];
    int ret;
    FILE *fd;
    
    fd = fopen(fileName,"r");
    if (!fd) {
        return 1;
    }
    if (5 != fread(magicBuf,1,5,fd)) {
        fclose(fd);
        return 1;
    }
    fclose(fd);
    dec = fc14dec_new();
    ret = fc14dec_detect(dec,magicBuf,5);
    fc14dec_delete(dec);
    return ret;
}

static void ip_play_file(char *fileName) {
    FILE *fd;
    void *fileBuf = NULL;
    size_t fileLen;
    int haveModule = 1;
    int audioDriverOK = 1;
    int haveSampleBuf = 1;

    playing = 0;
    jumpToTime = -1;
    if (decoder) {
        fc14dec_delete(decoder);
    }
    decoder = fc14dec_new();

    fd = fopen(fileName,"r");
    if (!fd) {
        return;
    }
    if ( fseek(fd,0,SEEK_END)!=0 ) {
        fclose(fd);
        return;
    }
    fileLen = ftell(fd);
    if ( fseek(fd,0,SEEK_SET)!=0 ) {
        fclose(fd);
        return;
    }
    fileBuf = g_malloc(fileLen);
    if ( !fileBuf ) {
        fclose(fd);
        return;
    }
    if ( fileLen != fread((char*)fileBuf,1,fileLen,fd) ) {
        fclose(fd);
        g_free(fileBuf);
        return;
    }
    fclose(fd);
    haveModule = fc14dec_init(decoder,fileBuf,fileLen);
    g_free(fileBuf);
    if ( !haveModule ) {
        return;
    }

    myFormat.freq = fc_myConfig.frequency;
    myFormat.channels = fc_myConfig.channels;
    if (fc_myConfig.precision == 8) {
        myFormat.xmmsAFormat = FMT_U8;
        myFormat.bits = 8;
        myFormat.zeroSample = 0x80;
    }
    else {
#ifdef WORDS_BIGENDIAN
        myFormat.xmmsAFormat = FMT_S16_BE;
#else
        myFormat.xmmsAFormat = FMT_S16_LE;
#endif
        myFormat.bits = 16;
        myFormat.zeroSample = 0x0000;
    }
    if (myFormat.freq>0 && myFormat.channels>0) {
        audioDriverOK = (iplugin.output->open_audio(myFormat.xmmsAFormat,
                                                    myFormat.freq,
                                                    myFormat.channels) != 0);
    }
    if ( !audioDriverOK ) {
        /* Try some audio configurations regardless of whether the
           output plugin might perform format conversion and/or
           downsampling. */
        struct audioFormat formatList[] = {
#ifdef WORDS_BIGENDIAN
            { FMT_S16_BE, 16, 44100, 1, 0x0000 },
#else
            { FMT_S16_LE, 16, 44100, 1, 0x0000 },
#endif
            { FMT_U8,      8, 44100, 1,   0x80 },
            { FMT_U8,      8, 22050, 1,   0x80 },
            { FMT_U8,      0,     0, 0,      0 }
        };
    
        int i = 0;
        do {
            myFormat = formatList[i];
            if (iplugin.output->open_audio(myFormat.xmmsAFormat,
                                           myFormat.freq,
                                           myFormat.channels) != 0)
            {
                audioDriverOK = 0;
                break;
            }
        }
        while (formatList[++i].bits != 0);
    }
    if ( audioDriverOK ) {
        sampleBufSize = 512*(myFormat.bits/8)*myFormat.channels;
        sampleBuf = g_malloc(sampleBufSize);
        haveSampleBuf = (sampleBuf != NULL);
        fc14dec_mixer_init(decoder,myFormat.freq,myFormat.bits,myFormat.channels,myFormat.zeroSample);
    }

    if ( haveSampleBuf && haveModule ) {
        int msecSongLen = fc14dec_duration(decoder);
        
        int bitsPerSec = myFormat.freq * myFormat.channels * myFormat.bits;
        char* title;
        char* pathSepPos = strrchr( fileName, '/' );
        if ( pathSepPos!=0 && pathSepPos[1]!=0 )  /* found file name */
            title = pathSepPos+1;
        else
            title = fileName;
        iplugin.set_info( title, msecSongLen, bitsPerSec, myFormat.freq, myFormat.channels );

        int usecPerBuf = (sampleBufSize * 1000000) / (myFormat.freq * myFormat.channels * (myFormat.bits/8));
        sleepVal = usecPerBuf;
        
        playing = 1;
        
        pthread_create(&decode_thread, NULL, play_loop, NULL);
    }
    else {
        deleteSampleBuf();
    }
}
    
static void ip_stop(void) {
    if ( playing ) {
        playing = 0;
        pthread_join(decode_thread, NULL);
        iplugin.output->close_audio();
        
        deleteSampleBuf();
        fc14dec_delete(decoder);
        decoder = NULL;
    }
}

static void ip_pause(short p) {
    iplugin.output->pause(p);
}

static void ip_seek(int secs) {
    jumpToTime = secs * 1000;
    
    while (jumpToTime != -1)
        xmms_usleep(10000);
}

static int ip_get_time(void) {
    if (iplugin.output) {
        if ( playing && decoder && fc14dec_song_end(decoder) && !iplugin.output->buffer_playing() )
            return -1;  /* stop and next file */
        else if ( playing )
            return iplugin.output->output_time();
        else
            return -1;
    }
    else
        return -1;
}

static void ip_get_song_info(char *fileName, char **title, int *length) {
    gchar* songTitle;
    gchar* pathSepPos = strrchr( fileName, '/' );
    if ( pathSepPos!=0 && pathSepPos[1]!=0 )  /* found file name */
        songTitle = pathSepPos+1;
    else
        songTitle = fileName;
    *title = g_strdup( songTitle );
    *length = -1;
}
