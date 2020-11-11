//// Constants
const PROXY_PORT = 44444 // much match proxy.conf
const WEBSERVER_PORT = 41004
const CHATSERVER_PORT = 41005 // duplicated in website

//// Proxy server
const child_process = require('child_process')
const net = require('net')
const proxyServer = child_process.exec('./proxy.elf')
let toServer = new net.Socket()

//// Webserver
const express = require('express')
let expressApp = express()
expressApp.use(express.static('public'))
var webServer = expressApp.listen(WEBSERVER_PORT, () => {})

//// WS Server
const ws = require('ws')
let wsServer = new ws.Server({port: CHATSERVER_PORT})
let toClient = null

const util = require("util")
let encoder = new util.TextEncoder("utf-8")
let decoder = new util.TextDecoder("utf-8")
let incPacketSizeOffset = 0
let incPacketSize = []
let incPacketData = null

wsServer.on('connection', (ws) => {
    toClient = ws 
    toClient.on('close', () => { process.exit(0) })

    toServer.connect(PROXY_PORT, '127.0.0.1', () => {        
        toServer.on('close', () => { process.exit(0) } )

        toServer.on('data', (data) => { proxyRecvPacket(data)})
        toClient.on('message', (msg) => { proxySendPacket(msg) })
    })
})

//// Proxy Adapter
function proxySendPacket(msg) {
    let temp = Buffer.allocUnsafe(8)
    temp.writeInt32BE(0, 0)
    temp.writeInt32BE(msg.length, 4)
    let lenBytes = new Uint8Array(temp)

    let dataBytes = encoder.encode(msg)

    toServer.write(lenBytes)
    toServer.write(dataBytes)
}

function proxyRecvPacket(data) {
    let offset = 0, end = data.length

    while(incPacketSizeOffset < 8 && offset < end) {
        incPacketSize.push(data[offset])
        ++incPacketSizeOffset
        ++offset
    
        if(8 == incPacketSizeOffset) {
            incPacketSize = Buffer.from(incPacketSize, 8).readUInt32BE(4)
            incPacketData = []
        }
    }

    if(incPacketData !== null) {
        while(incPacketSize && offset < end) {
            incPacketData.push(data[offset])
    
            --incPacketSize;
            ++offset;

            if(0 == incPacketSize) {
                incPacketSizeOffset = 0
                incPacketSize = []
    
                incPacketData = decoder.decode(Buffer.from(incPacketData))

                toClient.send(incPacketData)
                incPacketData = null
    
                proxyRecvPacket(data.slice(offset)); return
            }
        }
    }
}