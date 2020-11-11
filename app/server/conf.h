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
    serverIP = ConfGet(conf, "ip");
    serverPort = ConfGet(conf, "port");

    if(SmemInit(confSWS, MAX(NACLA_PUB_BYTES, NACLA_SEC_BYTES))) { exitFailure("Failed to allocate memory for ", "conf workspace", "");}
    
    if(SmemInit(pkData, NACLA_PUB_BYTES)) { exitFailure("Failed to allocate memory for ", "public key", ""); }
    if(SmemInit(skData, NACLA_SEC_BYTES)) { exitFailure("Failed to allocate memory for ", "private key", ""); }
    char *path;
    if(!(path = ConfGet(conf, "pk"))) { exitFailure("Failed to parse", "pk", "from conf"); }
    if(SmemLoadB64File(pkData, path, confSWS) || SmemSize(pkData) != NACLA_PUB_BYTES) { exitFailure("Failed to read", "pk", "from conf path"); }
    clientPK = SmemData(pkData);
    if(!(path = ConfGet(conf, "sk"))) { exitFailure("Failed to parse", "sk", "from conf"); }
    if(SmemLoadB64File(skData, path, confSWS) || SmemSize(pkData) != NACLA_SEC_BYTES) { exitFailure("Failed to read", "sk", "from conf path"); }
    serverSK = SmemData(skData);

    SmemDe(confSWS); memset(confSWS, 0, sizeof(Smem));
}