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

//// IO
static Ret proxySendSignal(u8 cb) {
    BuffSetSize(proxySendNameBuff, lenUsername);
    CHECK(BuffPush(proxySendNameBuff, cb));
    CHECK(BuffPush(proxySendNameBuff, SERVER_CB_NOLOG));
    CHECK(TCPSockSendP(toProxy, BuffData(proxySendNameBuff), BuffSize(proxySendNameBuff), NET_BLOCK_FULL));
    return 0;
FAILED:
    return -1;
}
static Ret proxySendMsg(Buff *buff) {
    BuffSetSize(proxySendBuff, lenPrefix);
    CHECK(BuffPushArr(proxySendBuff,  BuffData(buff), BuffSize(buff)));
    CHECK(BuffPush(proxySendBuff, CHAT_CB_MSG));
    CHECK(BuffPush(proxySendBuff, SERVER_CB_LOG));
    CHECK(TCPSockSendP(toProxy, BuffData(proxySendBuff), BuffSize(proxySendBuff), NET_BLOCK_FULL));
    CHECK(fileAppend(logPath, BuffData(proxySendBuff), BuffSize(proxySendBuff) - 2));
    return 0;
FAILED:
    return -1;
}
static Ret proxySendBlob(Buff *path, u8 cb) {
    for(usize last = BuffSize(path) - 1; BuffData(path)[last] == '\n'; NOP) {
        BuffPop(path);
        if(!last) { break; }
        else { --last; }
    }
    CHECK(BuffPush(path, 0));
    CHECK(BuffLoadFile(ws2, (char *)BuffData(path)));
    CHECK(BuffPush(ws2, cb));
    CHECK(BuffPush(ws2, SERVER_BLOB_LOG_CB));
    CHECK(TCPSockSendP(toProxy, BuffData(ws2), BuffSize(ws2), NET_BLOCK_FULL));
    CHECK(fileAppend(logPath, BLOB_SEND_MSG, BLOB_SEND_MSG_LEN));

    return 0;
FAILED:
    return -1;
}
//// Blob Input
static Ret blobCB(PTask **task) { 
    void *ign = task;

    CHECK(fileLock(fInputPath, 0));
    CHECK(BuffLoadFile(ws, fInputPath));
    if(BuffSize(ws) > 0) { 
        CHECK(filePTrunc(fInputPath, 0)); 
        CHECK(FWatchEmpty(fileWatch));
        CHECK(proxySendBlob(ws, CHAT_CB_FILE));
    }

    CHECK(fileLock(iInputPath, 0));
    CHECK(BuffLoadFile(ws, iInputPath));
    if(BuffSize(ws) > 0) {
        CHECK(filePTrunc(iInputPath, 0)); 
        CHECK(FWatchEmpty(imageWatch));
        CHECK(proxySendBlob(ws, CHAT_CB_IMG));
    }

    Ret ret = PTASK_RET_REPOLL;
CLEAN:
    if(fileUnlock(fInputPath)) {
        DEBUG_LOG("Failed to unlock fInputPath\n");
        return PTASK_RET_ERROR;
    }
    if(fileUnlock(iInputPath)) {
        DEBUG_LOG("Failed to unlock iInputPath\n");
        return PTASK_RET_ERROR;
    }

    return ret;
FAILED:
    CHECK(fileAppend(logPath, BLOB_FAIL_MSG, BLOB_FAIL_MSG_LEN));
    goto CLEAN;
}
static PWhen blobTrigs[3];
static PWhen *blobTrig(PTask **task) { void *ign = task; return blobTrigs; }
static PTask blobTaskFace = {&blobCB, &blobTrig, NULL};
static PTask *blobTask = &blobTaskFace;
////
//// Typing Input
static Ret typingCB(PTask **task) { 
    void *ign = task;
    Ret locked = 0;

    CHECK(fileLock(tInputPath, 0)); locked = 1;
    CHECK(BuffLoadFile(ws, tInputPath));
    if(BuffSize(ws) > 0) { 
        CHECK(filePTrunc(tInputPath, 0)); 
        CHECK(FWatchEmpty(typingWatch));
        currentTime = time(0);
        if(currentTime > lastTypingTime) {
            CHECK(proxySendSignal(CHAT_CB_TYP));
            lastTypingTime = currentTime;
        }
    }

    Ret ret = PTASK_RET_REPOLL;
CLEAN:
    if(locked && fileUnlock(tInputPath)) {
        DEBUG_LOG("Failed to unlock tInputPath\n");
        return PTASK_RET_ERROR;
    }

    return ret;
FAILED:
    ret = PTASK_RET_ERROR;
    goto CLEAN;
}
static PWhen *typingTrig(PTask **task) { void *ign = task; return FWatchPWhen(typingWatch); }
static PTask typingTaskFace = {&typingCB, &typingTrig, NULL};
static PTask *typingTask = &typingTaskFace;
////
//// Message Input
static Ret inputCB(PTask **task) { 
    void *ign = task;
    Ret locked = 0;

    CHECK(fileLock(mInputPath, 0)); locked = 1;
    CHECK(BuffLoadFile(ws, mInputPath));
    if(BuffSize(ws) > 0) { 
        CHECK(filePTrunc(mInputPath, 0)); 
        CHECK(FWatchEmpty(inputWatch));
        CHECK(proxySendMsg(ws));
    }

    Ret ret = PTASK_RET_REPOLL;
CLEAN:
    if(locked && fileUnlock(mInputPath)) {
        DEBUG_LOG("Failed to unlock mInputPath\n");
        return PTASK_RET_ERROR;
    }

    return ret;
FAILED:
    ret = PTASK_RET_ERROR;
    goto CLEAN;
}
static PWhen *inputTrig(PTask **task) { void *ign = task; return FWatchPWhen(inputWatch); }
static PTask inputTaskFace = {&inputCB, &inputTrig, NULL};
static PTask *inputTask = &inputTaskFace;
////
//// Proxy task
static Ret proxyCB(PTask **task) {
    void *ign = task;

    CHECK(trackerPassTime());
    if(lastHBTime + HEART_BEAT_SPAN_SECONDS <= currentTime) { 
        lastHBTime = currentTime; 
        CHECK(proxySendSignal(CHAT_CB_HB)); 
    }

    while(1) {        
        Ret tcpRet;
        NEG_CHECK(tcpRet = BuffTCPRecvP(ws, toProxy, NET_BLOCK_POLL));
        if(tcpRet) { break; }
        if(!BuffSize(ws)) {
            DEBUG_LOG("No chat control byte sent by other party?\n");
            break;
        }
        else {
            switch(BuffPop(ws)) {
                case CHAT_CB_MSG: {
                    CHECK(fileAppend(logPath, BuffData(ws), BuffSize(ws)));
                    // rm typing awkward due to no name separation
                } break;
                case CHAT_CB_HI: {
                    CHECK(fileAppend(logPath, BuffData(ws), BuffSize(ws)));
                    CHECK(fileAppend(logPath, CB_HI_MSG, CB_HI_LEN));
                    CHECK(proxySendSignal(CHAT_CB_HB));
                    CHECK(addPresent(ws));
                } break;
                case CHAT_CB_BYE: {
                    CHECK(fileAppend(logPath, BuffData(ws), BuffSize(ws)));
                    CHECK(fileAppend(logPath, CB_BYE_MSG, CB_BYE_LEN));
                    CHECK(rmPresent(ws));
                } break;
                case CHAT_CB_HB: {
                    CHECK(addPresent(ws));
                } break;
                case CHAT_CB_TYP: {
                    CHECK(addTyping(ws));
                } break;
                case CHAT_CB_FILE: {
                    CHECK(fileSave(DOWNLOAD_PATH, BuffData(ws), BuffSize(ws)));
                    CHECK(fileAppend(logPath, CB_FILE_MSG, CB_FILE_LEN));
                } break;
                case CHAT_CB_IMG: {
                    CHECK(fileSave(DOWNLOAD_PATH, BuffData(ws), BuffSize(ws)));
                    CHECK(fileAppend(logPath, CB_IMG_MSG, CB_IMG_LEN));
                    
                    { OPTIONAL_IMAGE_DISPLAY_LINE(); }

                } break;
                default: { DEBUG_LOG("Got wierd chat CB from server\n"); }
            }
        }
    }

    return PTASK_RET_REPOLL;
FAILED:
    return PTASK_RET_ERROR;
}
static PWhen proxyTrigs[2];
static PWhen *proxyTrig(PTask **task) { void *ign = task; return proxyTrigs; }
static PTask proxyTaskFace = {&proxyCB, &proxyTrig, NULL};
static PTask *proxyTask = &proxyTaskFace;
////
static void sendEndingSignals(void) {
    if(proxySendSignal(CHAT_CB_BYE)) { DEBUG_LOG("Failed to send leaving signal\n"); }
    if(logPath) { filePTrunc(logPath, 0); }
    if(typingPath) { filePTrunc(typingPath, 0); }
    if(presentPath) { filePTrunc(presentPath, 0); }
    if(fileAppend(logPath, CHAT_SHUTDOWN_MSG, CHAT_SHUTDOWN_MSG_LEN)) { DEBUG_LOG("Failed to warn UI of shutdown\n"); }
}
static void startingSignals() {
    if(proxySendSignal(CHAT_CB_HI)) { exitFailure("Failed to send", "arrival", "msg"); }
    if(atexit(sendEndingSignals)) { exitFailure("Failed to set", "ending exit callback", ""); }
    if(fileAppend(logPath, CHAT_STARTUP_MSG, CHAT_STARTUP_MSG_LEN)) { DEBUG_LOG("Failed to warn UI of startup\n"); }
}

static void taskInit() {
    if(FWatchInit(inputWatch, mInputPath)) { exitFailure("Failed to set", "input file watcher", ""); }
    if(PollerPush(mpoll, &inputTask)) { exitFailure("Failed to set poll", "message input task", ""); };

    if(FWatchInit(typingWatch, tInputPath)) { exitFailure("Failed to set", "typing file watcher", ""); }
    if(PollerPush(mpoll, &typingTask)) { exitFailure("Failed to set poll", "typing input task", ""); };

    if(FWatchInit(fileWatch, fInputPath)) { exitFailure("Failed to set", "file send watcher", ""); }
    if(FWatchInit(imageWatch, iInputPath)) { exitFailure("Failed to set", "image send watcher", ""); }
    blobTrigs[0] = *FWatchPWhen(fileWatch);
    blobTrigs[1] = *FWatchPWhen(imageWatch);
    if(PollerPush(mpoll, &blobTask)) { exitFailure("Failed to set poll", "blob task", ""); };

    proxyTrigs[0].fd = TCPSockGetFD(toProxy);
    proxyTrigs[0].when = PWHEN_IN;
    if(PollerPush(mpoll, &proxyTask)) { exitFailure("Failed to set poll", "proxy task", ""); }

    if(BuffReserve(proxySendBuff, lenPrefix)) { exitFailure("Failed to setup", "proxy send buffer", ""); }
    memcpy(BuffData(proxySendBuff), username, lenUsername); BuffSetSize(proxySendBuff, lenUsername);
    if(BuffPushArr(proxySendBuff, (u8 *R())": ", 2)) { exitFailure("Failed to setup", "proxy send buffer", ""); }
    if(BuffReserve(proxySendNameBuff, lenPrefix)) { exitFailure("Failed to setup", "proxy send name buffer", ""); }
    memcpy(BuffData(proxySendNameBuff), username, lenUsername); BuffSetSize(proxySendBuff, lenUsername);

    startingSignals();
}
