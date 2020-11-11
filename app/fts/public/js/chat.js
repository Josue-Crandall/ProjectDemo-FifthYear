const SERVER_PASSWORD = "SERVER_LAYER_PASSWORD_GOES_HERE!!"
const CHATSERVER_PORT = 12346
const DEFAULT_NAME = "Alice"
const TITLE = "File Transfer Server Chat"
const BLINK_RATE = 1500
const BLINK_MSG = "New Messages!"
const BLUR_DELAY_TIME = 2000
const ENTER_KEY = 13
const TYPING_MESSAGE_DURATION = 1250
const PRESENT_DURATION_MILISECONDS = 45 * 1000
const CHAT_DEFAULT_HANDLE = "Alice"
const CHAT_DEFAULT_ROOM = "DEFAULT_ROOM_PASSWORD_GOES_HERE!!"
const HEART_BEAT_MILISECONDS = 23 * 1000
const LOGIN_MESSAGE = 'Login successful.'

// Init
document.title = TITLE

// globals
let chatLog = document.getElementById('chatLog')
let userNameInput = document.getElementById("userNameInput")
let aesPhraseInput = document.getElementById("aesPhraseInput")
let loginButton = document.getElementById('loginButton')
let userChatInput = document.getElementById('userChatInput')
let feedback = document.getElementById('feedback')
let userLog = document.getElementById('userLog')

let notificationSound = new Audio('./img/steamMessageSound.mp3')

let focused = true
let blinkingInterval = null
let blinkingReady = false

// wss ?
let handle, room, socket = new WebSocket(`ws://${window.location.hostname}:${CHATSERVER_PORT}`)

let feedbackTrackerObject = {}, userTrackerObject = {}
let sendImageFunction = null

//// Focus handler
window.onblur = () => {
    setTimeout(() => { if(!focused) { blinkingReady = true } }, BLUR_DELAY_TIME)
    focused = false
} 
window.onfocus = () => {
    blinkingReady = false
    focused = true
    
    if(blinkingInterval !== null) { 
        clearInterval(blinkingInterval)
        blinkingInterval = null;
        document.title = TITLE
    }
}
function startBlinking(){
    if(blinkingReady && blinkingInterval == null) {
        blinkingInterval = setInterval(() => {
            if(document.title == TITLE) document.title = BLINK_MSG
            else document.title = TITLE
        }, BLINK_RATE)
    }
}
function alertThatMessageHasBeenRecieved() {
    if(!focused && blinkingReady) {
        startBlinking()
        notificationSound.play()
    }
}

//// Drag handlers
function handleBodyDragOver(event) {
    event.stopPropagation()
    event.preventDefault()
    return false
}
function handleBodyDrop(event) {
    if(sendImageFunction) {
        event.stopPropagation()
        event.preventDefault()
        
        var data = event.dataTransfer.items
        for (let i = 0; i < data.length; ++i) {
            if(data[i].kind == 'file' && 
                (data[i].type == 'image/jpeg' || data[i].type == 'image/png' || data[i].type == 'image/gif' || data[i].type == 'image/webp')) {   
                    let fileReader = new FileReader()
                    fileReader.onload = e => sendImageFunction(e.target.result)
                    fileReader.readAsDataURL(data[i].getAsFile())
            }
        }

        return false
    }
}

//// login handler
window.onkeypress = (e) => {
    if(e.keyCode == ENTER_KEY) {
        login()
        window.onkeypress = null
    }
}

//// Crypto
function encryptFunction(msg) { return sjcl.encrypt(SERVER_PASSWORD, sjcl.encrypt(room, msg)) }
function decryptFunction(msg) { return sjcl.decrypt(room, sjcl.decrypt(SERVER_PASSWORD, msg)) }
function socketSend(obj) { socket.send(encryptFunction(JSON.stringify(obj))) }
function socketRecv(msg) { return JSON.parse(decryptFunction(msg)) }

function login() {
    readForm()
    setupWSCallbacks()
}
function readForm() {
    handle = userNameInput.value
    if(handle == "") handle = CHAT_DEFAULT_HANDLE
    room = aesPhraseInput.value
    if(room == "") room = CHAT_DEFAULT_ROOM
}
function setupWSCallbacks() {
    socket.onmessage = (event) => {
        let msg = socketRecv(event.data)
        switch(msg['type']) {
            case "hello": {
                beatHeart()
                trackPresent(msg['handle'])
                alertThatMessageHasBeenRecieved()
            } break;
            case "message": {
                updateChatLog(`${msg['handle']}: ${msg['message']}`)
                deleteFeedbackName(msg['handle'])
                alertThatMessageHasBeenRecieved()
            } break;
            case "typing": {
                trackTyping(msg['handle'])
            } break;
            case "image": {
                updateChatLog(`${msg['handle']} sent:`)
                updateImageToChatLog(msg['message'])
                alertThatMessageHasBeenRecieved()
            } break;
            case "heartbeat": {
                trackPresent(msg['handle'])
            } break;
        }
    }

    userChatInput.onkeypress = (e) => {
        if(e.keyCode == ENTER_KEY && userChatInput.value != '') {
            socketSend({type:"message",handle:handle,message:userChatInput.value})
            updateChatLog(`${handle}: ${userChatInput.value}`)
            userChatInput.value = ""
        }
        else { socketSend({type:"typing",handle:handle}) }
    }

    keepHeartBeating()
    sendImageFunction = sendImageImp
    socketSend({type:"hello",handle:handle})
    displayFeedbackObject()
    displayUserTrackerObject()
    updateChatLog(LOGIN_MESSAGE)
    sceneTransitionLoginToChat()
}
function updateChatLog(msg){
    msg = msg.replace(/</g, "&lt;").replace(/>/g, "&gt;")
    chatLog.innerHTML = `${chatLog.innerHTML}${msg}<br>`
    chatLog.scrollTop = chatLog.scrollHeight
}
function beatHeart() { socketSend({type:"heartbeat",handle:handle}) }
function keepHeartBeating() { setInterval(beatHeart, HEART_BEAT_MILISECONDS) }
function sceneTransitionLoginToChat() {
    document.getElementById("loginTable").style.display = 'none'
    document.getElementById("body").style.backgroundImage = 'none'
    document.getElementById("chatControls").style.display = 'block'
}
function updateImageToChatLog(imageFile) {
    let image = document.createElement('img')
    image.src = imageFile
    image.style.maxWidth = '90%'
    image.style.maxHeight = '525px'
    chatLog.appendChild(image)
    updateChatLog("") // add a line break
    chatLog.scrollTop = chatLog.scrollHeight
}
function deleteFeedbackName(name){
    delete feedbackTrackerObject[name]
    displayFeedbackObject()
}
function deleteUserTrackerName(name){
    delete userTrackerObject[name]
    displayUserTrackerObject()
}
function displayFeedbackObject(){
    feedback.innerHTML = ''
    for(user in feedbackTrackerObject) {
        feedback.innerHTML += '<em>' + user + ' is typing a message...</em><br>'
    }
}
function displayUserTrackerObject() {
    userLog.innerHTML = 'Users in room:\n    ' + handle + '\n'
    for(user in userTrackerObject) { userLog.innerHTML += `    ${user}\n`}
}
function sendImageImp(imageFile) {
    socketSend({type:'image',handle:handle,message:imageFile})
    updateChatLog('Sent image:')
    updateImageToChatLog(imageFile)
}
function trackTyping(name) {
    if(feedbackTrackerObject[name]) { clearTimeout(feedbackTrackerObject[name]) }
    feedbackTrackerObject[name] = setTimeout(() => deleteFeedbackName(name), TYPING_MESSAGE_DURATION)
    displayFeedbackObject()
}
function trackPresent(name) {
    if(userTrackerObject[name]) { clearTimeout(userTrackerObject[name]) }
    userTrackerObject[name] = setTimeout(() => deleteUserTrackerName(name), PRESENT_DURATION_MILISECONDS)
    displayUserTrackerObject()
}
