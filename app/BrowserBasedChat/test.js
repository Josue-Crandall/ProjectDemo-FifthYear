const util = require("util")
let encoder = new util.TextEncoder("utf-8")
let decoder = new util.TextDecoder("utf-8")
let incPacketSizeOffset = 0
let incPacketSize = []
let incPacketData

let msg = "ABC"
let a, b
proxySendPacket(msg)
proxyRecvPacket(a)
if(proxyRecvPacket(b)) { console.log(incPacketData) }

