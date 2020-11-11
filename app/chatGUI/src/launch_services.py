import os
import subprocess
import time

def go(callback):
    os.chdir('chat'); chat = subprocess.Popen(['./chat.elf']); os.chdir('..')
    callback()
    chat.terminate()