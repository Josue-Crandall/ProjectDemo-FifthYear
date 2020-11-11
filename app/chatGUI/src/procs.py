import os
import os.path
import vlc #pip3 install python-vlc

def fileLoad(path):
    file = open(path,'r+') 
    data = file.read()
    file.close()
    return data
def fileSave(path, data):
    file = open(path,'r+')
    file.write(data)
    file.close()
def fileMove(path, newPath):
    os.rename(path, newPath)
def fileExists(path):
    return os.path.exists(path)
def playSound(path):
    sound = vlc.MediaPlayer(path)
    sound.play()