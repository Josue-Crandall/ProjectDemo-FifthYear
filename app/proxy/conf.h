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

static void confInit(void) {
    if(ConfInit(conf, CONF_PATH)) { exitFailure("Failed to parse conf at", CONF_PATH, ""); }
    if(!(serverIP = ConfGet(conf, "ip"))) { exitFailure("Failed to parse", "ip", ""); }
    if(!(serverPort = ConfGet(conf, "port"))) { exitFailure("Failed to parse", "port", ""); }
    if(!(channel = ConfGet(conf, "channel"))) { exitFailure("Failed to parse", "channel", ""); }
    if( (useProxy = !ConfGet(conf,"skipProxy"))) {
        if(!(proxyIP = ConfGet(conf, "proxyIP"))) { exitFailure("Failed to parse", "proxyIP", ""); }
        if(!(proxyPort = ConfGet(conf, "proxyPort"))) { exitFailure("Failed to parse", "proxyPort", ""); }
    }
    char *path;
    if(!(path = ConfGet(conf, "pk"))) { exitFailure("Failed to parse", "pk", "from conf"); }
    if(SmemLoadB64File(pk, path, sws) || SmemSize(pk) != NACLA_PUB_BYTES) { exitFailure("Failed to read", "pk", "from conf path"); }
    serverPK = SmemData(pk);
    if(!(path = ConfGet(conf, "sk"))) { exitFailure("Failed to parse", "sk", "from conf"); }
    if(SmemLoadB64File(sk, path, sws) || SmemSize(sk) != NACLA_SEC_BYTES) { exitFailure("Failed to read", "sk", "from conf path"); }
    clientSK = SmemData(sk);

    lisIP = ConfGet(conf, "lisIP");
    lisPort = ConfGet(conf, "lisPort");

    if(!(e2ePath = ConfGet(conf, "key"))) { printf("%sWARNING:%s no %ssymmetric%s e2e encryption\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT, ANSI_COLOR_BOLD_CYAN, ANSI_COLOR_DEFAULT); }
    else if(SmemLoadB64File(e2eKey, e2ePath, sws) || SmemSize(e2eKey) != NACLS_KEY_BYTES) { exitFailure("Failed to load", "e2eKey", "from conf"); }
    if(!(rprotPath = ConfGet(conf, "rprot"))) { printf("%sWARNING:%s no %sratcheting%s e2e encryption\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT, ANSI_COLOR_BOLD_CYAN, ANSI_COLOR_DEFAULT); }
    else if(RprotInit(rprot, rprotPath)) { exitFailure("Failed to load", "rprot", "from conf"); }
    if(!(oprotPath = ConfGet(conf, "oprot"))) { printf("%sWARNING:%s no %sone time pad%s e2e encryption\n", ANSI_COLOR_BOLD_RED, ANSI_COLOR_DEFAULT, ANSI_COLOR_BOLD_CYAN, ANSI_COLOR_DEFAULT);  }
    else if(OprotInit(oprot, oprotPath)) { exitFailure("Failed to load", "oprot", "from conf"); }
}