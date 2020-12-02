//// Electron forge's code

const { app, BrowserWindow } = require('electron');
const path = require('path');

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (require('electron-squirrel-startup')) { // eslint-disable-line global-require
  app.quit();
}

const createWindow = () => {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 1200,
    height: 1000,
    webPreferences: {
      nodeIntegration: true
    },
  });
  mainWindow.removeMenu();

  // and load the index.html of the app.
  mainWindow.loadFile(path.join(__dirname, 'public/index.html'));

  // Open the DevTools.
  //mainWindow.webContents.openDevTools();
};

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow);

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and import them here.

//// ~Electron forge's code

//////// Chat code

// WARNING!: Proxy adapter has undefined behavior and breaks on >4gb sends.

//// Constants
const PROXY_PORT = 44444 // much match proxy.conf
const CHATSERVER_PORT = 41005 // duplicated in website

//// Proxy server
const os = require('os')
const process = require('process')
const child_process = require('child_process')
const net = require('net')

process.chdir(`${os.homedir()}/proxy/`)
const proxyServer = child_process.exec('./proxy.elf', (e, cout, cerr) => {
    if(e) { console.log(`Proxy Error: ${e}`) }
    else { console.log(`Proxy (stdout): ${cout}`); console.log(`Proxy (stderr): ${cerr}`) }
})
let toServer = new net.Socket()

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
//////// ~Chat code

