//// Constants
const WEBSERVER_PORT = 12345
const CHATSERVER_PORT = 12346

const { WSASERVICE_NOT_FOUND } = require('constants')
//// Webserver
let express = require('express')
let expressApp = express()
expressApp.use(express.static('public'))
var webServer = expressApp.listen(WEBSERVER_PORT, () => {})

//// Chatserver
let ws = require('ws')
let chatServer = new ws.Server({port: CHATSERVER_PORT})
let chatClients = {}
let chatTicker = 0
chatServer.on('connection', (ws) => {
    let id = chatTicker++
    chatClients[id] = ws
    ws.on('close', () => { delete chatClients[id] })
    ws.on('message', (msg) => { for(cid in chatClients) { if(id != cid) chatClients[cid].send(msg) } })
})