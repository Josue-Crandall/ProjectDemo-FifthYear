/*
	Alice v.08 a chat client and server.
	Copyright (C) 2020 Josue Crandall

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

//// conf
#define CONF_PATH "chat.conf"
#define SERVER_CB_LOG   '1'
#define SERVER_CB_NOLOG '0'
#define SERVER_BLOB_LOG_CB (SERVER_CB_NOLOG)
#define DOWNLOAD_PATH "./CHAT_DL_TEMP"
#define OPTIONAL_IMAGE_DISPLAY_LINE()       \
{                                           \
    static char *DISPLAY_ARGS[] = {         \
        "eog", DOWNLOAD_PATH, 0             \
    };                                      \
    NEG_CHECK(forkedExec("eog", DISPLAY_ARGS, 0));  \
}                                           \

#define PROXY_EXEC_PATH "./proxy.elf"

#define HEART_BEAT_SPAN_SECONDS  (23)
#define PRESENT_DURATION_SECONDS (30)
#define TYPING_DURATION_SECONDS  (2)
#define UPDATE_TIMEOUT_MILISECONDS (1050)
#define CHAT_CB_MSG  'M'
#define CHAT_CB_HI   'H'
#define CHAT_CB_BYE  'Y'
#define CHAT_CB_HB   'B'
#define CHAT_CB_TYP  'T'
#define CHAT_CB_IMG  'I'
#define CHAT_CB_FILE 'F'

static char *proxyIP, *proxyPort;

static char *logPath, *typingPath, *presentPath;
static char *mInputPath, *fInputPath, *iInputPath, *tInputPath;

static char *username;
static usize lenUsername;
static usize lenPrefix;

//// presentation
#define CB_HI_MSG ((u8 *)" has arrived.\n")
#define CB_HI_LEN (14)
#define CB_BYE_MSG ((u8 *)" has left.\n")
#define CB_BYE_LEN (11)
#define CB_FILE_MSG ((u8 *)"Got a file.\n")
#define CB_FILE_LEN (12)
#define CB_IMG_MSG  ((u8 *)"Got an image.\n")
#define CB_IMG_LEN  (14)
#define BLOB_FAIL_MSG ((u8 *)"Blob send failed.\n")
#define BLOB_FAIL_MSG_LEN (18)
#define TYPING_MSG ((u8 *)" is typing ...\n")
#define TYPING_MSG_LEN (15)
#define BLOB_SEND_MSG ((u8 *)"Blob sent.\n")
#define BLOB_SEND_MSG_LEN (11)
#define CHAT_SHUTDOWN_MSG ((u8 *)"\n  Disconnected.\n")
#define CHAT_SHUTDOWN_MSG_LEN (17)
#define CHAT_STARTUP_MSG ((u8 *)"\n  Connected!\n\n")
#define CHAT_STARTUP_MSG_LEN (15)


//// data
SPRAI(Conf, conf);
SPRAI(TCPSock, toProxy);
SPRAI(Poller, mpoll);
SPRAI(FWatch, inputWatch);
SPRAI(FWatch, typingWatch);
SPRAI(FWatch, fileWatch);
SPRAI(FWatch, imageWatch);
SPRAI(Buff, ws);
SPRAI(Buff, proxySendBuff);
SPRAI(Buff, proxySendNameBuff);
SPRAI(Buff, ws2);

// WARNING: abusing time_t
static time_t currentTime, lastHBTime, lastTypingTime;

DD_TABLE(Tracker, char *, time_t, strHash(key), res = !strcmp(lhs, rhs), free(key), NOP, STD_ALLOC);
static Tracker ptrackData, ttrackData, *ptrack = &ptrackData, *ttrack = &ttrackData;

void dataDe() {
    ConfDe(conf);
    TCPSockDe(toProxy);
    PollerDe(mpoll);
    FWatchDe(inputWatch);
    FWatchDe(typingWatch);
    FWatchDe(fileWatch);
    FWatchDe(imageWatch);
    BuffDe(ws);
    BuffDe(proxySendBuff);
    BuffDe(proxySendNameBuff);
    BuffDe(ws2);
    TrackerDe(ttrack);
    TrackerDe(ptrack);
}

void dataInit() {
    if(atexit(dataDe)) { exitFailure("Failed to set", "static data", "cleaning"); }
    if(PollerInit(mpoll, UPDATE_TIMEOUT_MILISECONDS)) { exitFailure("Failed to init", "Poller", ""); }
    if(BuffInit(ws, 0)) { exitFailure("Failed to init", "workspace", ""); }
    if(BuffInit(proxySendBuff, 0)) { exitFailure("Failed to init", "proxySendBuff", ""); }
    if(BuffInit(proxySendNameBuff, 0)) { exitFailure("Failed to init", "proxySendNameBuff", ""); }
    if(BuffInit(ws2, 0)) { exitFailure("Failed to init", "second workspace", ""); }
    if(TrackerInit(ttrack, 0)) { exitFailure("Failed to init", "typing tracker", ""); }
    if(TrackerInit(ptrack, 0)) { exitFailure("Failed to init", "present tracker", ""); }
    currentTime = time(0);
}