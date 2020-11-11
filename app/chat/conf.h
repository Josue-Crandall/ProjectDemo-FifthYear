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

void confInit() {
    if(ConfInit(conf, CONF_PATH)) { exitFailure("Failed to parse conf at", CONF_PATH, ""); }
    if(!(proxyIP = ConfGet(conf, "proxyIP"))) { exitFailure("Failed to parse", "proxyIP", ""); }
    if(!(proxyPort = ConfGet(conf, "proxyPort"))) { exitFailure("Failed to parse", "proxyPort", ""); }

    if(!(logPath = ConfGet(conf, "log"))) { exitFailure("Failed to parse", "log", ""); }
    if(!(typingPath = ConfGet(conf, "typing"))) { exitFailure("Failed to parse", "typing", ""); }
    if(!(presentPath = ConfGet(conf, "present"))) { exitFailure("Failed to parse", "present", ""); }
    if(!(mInputPath = ConfGet(conf, "minput"))) { exitFailure("Failed to parse", "minput", ""); }
    if(!(fInputPath = ConfGet(conf, "finput"))) { exitFailure("Failed to parse", "finput", ""); }
    if(!(iInputPath = ConfGet(conf, "iinput"))) { exitFailure("Failed to parse", "iinput", ""); }
    if(!(tInputPath = ConfGet(conf, "tinput"))) { exitFailure("Failed to parse", "tinput", ""); }

    if(!(username = ConfGet(conf, "name"))) { exitFailure("Failed to parse", "name", ""); }
    lenUsername = strlen(username);
    lenPrefix = lenUsername; if(lenPrefix + 2 < lenPrefix) { exitFailure("username is", "too long", ""); }
    lenPrefix += 2;

    if(filePTrunc(presentPath, 0)) { exitFailure("Failed to reset", "present", "file"); }
    if(filePTrunc(mInputPath, 0)) { exitFailure("Failed to reset", "message input", "file"); }
    if(filePTrunc(fInputPath, 0)) { exitFailure("Failed to reset", "file input", "file"); }
    if(filePTrunc(iInputPath, 0)) { exitFailure("Failed to reset", "image input", "file"); }
    if(filePTrunc(tInputPath, 0)) { exitFailure("Failed to reset", "typing input", "file"); }
}