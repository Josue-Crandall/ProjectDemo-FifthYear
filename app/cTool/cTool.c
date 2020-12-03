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

#include "../../lib/macros/macros.h"
#include "../../lib/macros/term.h"
#include "../../lib/conf/conf.h"
#include "../../lib/file/file.h"
#include "../../lib/nacl/nacl.h"
#include "../../lib/oprot/oprot.h"
#include "../../lib/rprot/rprot.h"
#include "../../lib/api/api.h"

#include <stdio.h>

static char *confPath = "./cTool.conf";
static char *function;
static Conf conf;
static Smem buff1, buff2, buff3, buff4;
static Oprot oprot;
static Rprot rprot;  char *rprotDestroyPath;

static void cleanupStatics(void) {
    SmemDe(&buff1);SmemDe(&buff2);SmemDe(&buff3);SmemDe(&buff4);
    OprotDe(&oprot);RprotDe(&rprot, rprotDestroyPath);
}
static void exitFailure(char * R() start, char * R() highlight, char * R() end) {
    printf("%sERROR:%s %s%s%s%s%s%s%s%s%s\n", ANSI_COLOR_RED, ANSI_COLOR_DEFAULT, 
           ANSI_COLOR_DEFAULT, start, ANSI_COLOR_DEFAULT,
           ANSI_COLOR_BOLD_CYAN, highlight, ANSI_COLOR_DEFAULT,
           ANSI_COLOR_DEFAULT, end, ANSI_COLOR_DEFAULT);
    cleanupStatics();
    exit(0);
}
/////////
static char * parseKey(char *key) {
    char *R() res = ConfGet(&conf, key);
    if(!res) { exitFailure("Failed to parse ", key, " from configuration."); }
    return res;
}
static void loadBinFile(char *path, Smem *buff) {
    if(SmemLoadFile(buff, path)) { exitFailure("Failed to load file ", path, ""); }
}
static void loadB64File(char *path, Smem *buff) {
    if(SmemLoadB64File(buff, path, &buff3)) { exitFailure("Failed to load file ", path, ""); }
}
static void saveBinFile(char *path, Smem *buff) {
    if(fileSave(path, SmemData(buff), SmemSize(buff))) {
        exitFailure("Failed to save file ", path, "");
    }
}
static void saveB64File(char *path, Smem *buff) {
    if(SmemSaveB64File(buff, path, &buff3)) {
        exitFailure("Failed to save b64 file ", path, "");
    }
}
static void loadKey(char *path, Smem *buff, usize size) {
    loadB64File(path, buff);
    if(SmemSize(buff) != size) {
        exitFailure("Key file ", path, " was not the correct size.");
    }
}
static void safeReserve(Smem *buff, usize bytes) {
    if(SmemReserve(buff, bytes)) {
        exitFailure("Failed to ", "reserve memory", ".");
    }
    SmemSetSize(buff, bytes);
}
static void saveEncodedFile(char *path, Smem *buff) {
    if(ConfGet(&conf, "binaryEncoding")) { saveBinFile(path, buff); }
    else { saveB64File(path, buff); }
}
static void loadEncodedFile(char *path, Smem *buff) {
    if(ConfGet(&conf, "binaryEncoding")) { loadBinFile(path, buff); }
    else { loadB64File(path, buff); }
}
////////
static void encodeBase64fn() {
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");
    loadBinFile(inPath, &buff1);
    saveB64File(outPath, &buff1);
}
static void decodeBase64fn() {
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");
    loadB64File(inPath, &buff1);
    saveBinFile(outPath, &buff1);
}
static void symmetricGenfn() {
    char *R() outPath = parseKey("outputFile");
    safeReserve(&buff1, NACLS_KEY_BYTES);
    randombytes(SmemData(&buff1), NACLS_KEY_BYTES);
    saveB64File(outPath, &buff1);
}
static void symmetricEncfn() {    
    char *R() keyPath = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    loadKey(keyPath, &buff1, NACLS_KEY_BYTES);
    loadBinFile(inPath,&buff2);
    if(SmemSEncrypt(&buff2, SmemData(&buff1), &buff3)) {
        exitFailure("Encrypt failed ", "?allocation?", ".");
    }

    saveEncodedFile(outPath, &buff2);
}
static void symmetricDecfn() {
    char *R() keyPath = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    loadKey(keyPath, &buff1, NACLS_KEY_BYTES);
    loadEncodedFile(inPath,&buff2);
    if(SmemSDecrypt(&buff2, SmemData(&buff1), &buff3)) {
        exitFailure("Authentication failure while decrypting ", inPath, ".");
    }
    saveBinFile(outPath, &buff2);
}

static void asymmetricGenfn() {
    char *R() pkPath = parseKey("publicKeyPath");
    char *R() skPath = parseKey("privateKeyPath");
    safeReserve(&buff1, NACLA_PUB_BYTES);
    safeReserve(&buff2, NACLA_SEC_BYTES);
    naclKeyPair(SmemData(&buff1), SmemData(&buff2));
    saveB64File(pkPath, &buff1);
    saveB64File(skPath, &buff2);
}
static void asymmetricEncfn() {    
    char *R() pkPath = parseKey("publicKeyPath");
    char *R() skPath = parseKey("privateKeyPath");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    loadKey(pkPath, &buff1, NACLA_PUB_BYTES);
    loadKey(skPath, &buff2, NACLA_SEC_BYTES);
    loadBinFile(inPath, &buff4);
    if(SmemAEncrypt(&buff4, SmemData(&buff1), SmemData(&buff2), &buff3)) {
        exitFailure("Encrypt failed ", "?allocation?", ".");
    }

    saveEncodedFile(outPath, &buff4);
}
static void asymmetricDecfn() {   
    char *R() pkPath = parseKey("publicKeyPath");
    char *R() skPath = parseKey("privateKeyPath");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    loadKey(pkPath, &buff1, NACLA_PUB_BYTES);
    loadKey(skPath, &buff2, NACLA_SEC_BYTES);
    loadEncodedFile(inPath, &buff4);
    if(SmemADecrypt(&buff4, SmemData(&buff1), SmemData(&buff2), &buff3)) {
        exitFailure("Authentication failure while decrypting ", inPath, "");
    }

    saveBinFile(outPath, &buff4);
}
static void dratchetGenfn() {
    char *R() outPath1 = parseKey("outputFile1");
    char *R() outPath2 = parseKey("outputFile2");
    if(RprotMake(outPath1, outPath2)) {
        exitFailure("Generating ", "double ratcheting ", "keys has failed.");
    }
}
static void dratchetEncfn() {    
    char *R() key = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    if(RprotInit(&rprot, key)) {
        exitFailure("Could not parse ", key, " as a double ratchet protocol key.");
    }
    rprotDestroyPath = key;
    loadBinFile(inPath, &buff1);

    if(SmemRPEncrypt(&buff1, &rprot, &buff3, &buff4)) {
        exitFailure("Encrypt failed ", "?allocation?", ".");
    }
    RPEncryptFinish(&rprot, SmemData(&buff3));

    saveEncodedFile(outPath, &buff1);
}
static void dratchetDecfn() {   
    char *R() key = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    if(RprotInit(&rprot, key)) {
        exitFailure("Could not parse ", key, " as a double ratchet protocol key.");
    }
    rprotDestroyPath = key;
    loadEncodedFile(inPath, &buff1);
    if(SmemRPDecrypt(&buff1, &rprot, &buff3, &buff4)) {
        exitFailure("Authentication failure while decrypting ", inPath, "");
    }
    saveBinFile(outPath, &buff1);
}
static void otpGenfn() {
    char *R() outPath1 = parseKey("outputFile1");
    char *R() outPath2 = parseKey("outputFile2");
    char *R() keySize = parseKey("len");
    char *R() rngSrc = ConfGet(&conf, "rngSrc");
    if(OprotMake(outPath1, outPath2, atoll(keySize), rngSrc)) {
        exitFailure("Generating ", "one time pad ", "keys has failed.");
    }
}
static void otpEncfn() {    
    char *R() key = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    if(OprotInit(&oprot, key)) {
        exitFailure("Could not parse ", key, " as a one time pad key.");
    }
    loadBinFile(inPath, &buff1);
    if(SmemOPEncrypt(&buff1, &oprot, &buff3, SmemData(&buff4))) {
        exitFailure("Encrypt failed ", "?allocation?", ".");
    }

    saveEncodedFile(outPath, &buff1);
}
static void otpDecfn() {   
    char *R() key = parseKey("keyFile");
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    if(OprotInit(&oprot, key)) {
        exitFailure("Could not parse ", key, " as a one time pad key.");
    }
    loadEncodedFile(inPath, &buff1);
    if(SmemOPDecrypt(&buff1, &oprot, &buff3, SmemData(&buff4))) {
        exitFailure("Authentication failure while decrypting ", inPath, "");
    }
    
    saveBinFile(outPath, &buff1);
}
static void hashfn() {
    char *R() inPath = parseKey("inputFile");
    char *R() outPath = parseKey("outputFile");

    loadBinFile(inPath, &buff1);
    safeReserve(&buff2, NACLH_BYTES);
    naclHash(SmemData(&buff2), SmemData(&buff1), SmemSize(&buff1));

    saveEncodedFile(outPath, &buff2);
}
static void verifyHashfn() {
    char *R() inPath = parseKey("inputFile");
    char *R() hashPath = parseKey("hashFile");

    loadBinFile(inPath, &buff1);
    safeReserve(&buff2, NACLH_BYTES);
    naclHash(SmemData(&buff2), SmemData(&buff1), SmemSize(&buff1));
    loadEncodedFile(hashPath, &buff4);
    if(SmemSize(&buff4) != NACLH_BYTES) {
        exitFailure("Hash file ", hashPath, " was not a valid length");
    }
    if(memcmp(SmemData(&buff2), SmemData(&buff4), NACLH_BYTES)) {
        printf(" !!! %sWARNING%s: %s%s%s did %sNOT%s match hash !!!\n", 
            ANSI_COLOR_RED, ANSI_COLOR_DEFAULT, ANSI_COLOR_BOLD_RED, inPath, ANSI_COLOR_DEFAULT, 
            ANSI_COLOR_BOLD_MAGENTA, ANSI_COLOR_DEFAULT);
    }
    else {
        printf("Success: %s was verified against hash\n", inPath);
    }
}

////////////////////

int main(int argc, char **argv) {
    if(argc == 2) { confPath = argv[1]; }
    if(ConfInit(&conf, confPath)) {
        exitFailure("Could not parse ", confPath, " as a configuration file.");
    }
    CHECK(SmemInit(&buff1, 0)); CHECK(SmemInit(&buff2, 0));
    CHECK(SmemInit(&buff3, NACLH_BYTES)); CHECK(SmemInit(&buff4, NACLH_BYTES));
    if(0 == (function = ConfGet(&conf, "function"))) {
        exitFailure("Failed to parse ", "function", " from config file.");
    }
#define DO_FN(NAME) if(0 == strcmp(function, #NAME)) { NAME##fn(); }
    DO_FN(encodeBase64); DO_FN(decodeBase64);
    DO_FN(symmetricGen); DO_FN(symmetricEnc); DO_FN(symmetricDec);
    DO_FN(asymmetricGen); DO_FN(asymmetricEnc); DO_FN(asymmetricDec);
    DO_FN(dratchetGen); DO_FN(dratchetEnc); DO_FN(dratchetDec);
    DO_FN(otpGen); DO_FN(otpEnc); DO_FN(otpDec);
    DO_FN(hash); DO_FN(verifyHash);
#undef DO_FN
    printf("Function: %s%s%s completed.\n", ANSI_COLOR_GREEN, function, ANSI_COLOR_DEFAULT);
    cleanupStatics();
    exit(0);
FAILED:
    exitFailure("Failed initialization of ", "buffers", ".");
}