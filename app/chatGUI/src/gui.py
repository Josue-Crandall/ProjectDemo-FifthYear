#libs
import tkinter as tk
SHIFT_DOWN_MASK = 1
BACK_SPACE = '\x08'
import time

#scripts
import src.procs as procs

#const
REFRESH_TIMER_MILISECONDS = 125
ALERT_TIMER_SECONDS = 10
ALERT_COOLDOWN = 5
LOG_PATH = 'chat/log.txt'
LOG_TEMP_PATH = 'chat/logTEMP.txt'
TYPING_PATH = 'chat/typing.txt'
PRESENT_PATH = 'chat/present.txt'
OUTPUT_TYPING_PATH = 'chat/tinput.txt'
OUTPUT_MESSAGE_PATH = 'chat/minput.txt'
OUTPUT_FILE_PATH = 'chat/finput.txt'
OUTPUT_IMAGE_PATH = 'chat/iinput.txt'

#windows
window = tk.Tk()

leftFrame = tk.Frame(relief=tk.RAISED,master=window)
rightFrame = tk.Frame(relief=tk.RAISED,master=window)

logLabel = tk.Label(master=leftFrame,text='Chat Log:',height=1)
log = tk.Text(master=leftFrame,height=28,width=24)
typing = tk.Label(master=leftFrame,height=2)
inputLabel = tk.Label(master=leftFrame,text='Chat Input:',height=1)
inputEntry = tk.Text(master=leftFrame,height=4)

presentLabel = tk.Label(master=rightFrame,text='In Room:',height=1)
present = tk.Text(master=rightFrame,height=28,width=8)
fileLabel = tk.Label(master=rightFrame,text='File path:',height=1)
fileEntry = tk.Entry(master=rightFrame)
imageLabel = tk.Label(master=rightFrame,text='Image path:',height=1)
imageEntry = tk.Entry(master=rightFrame)

refreshWidget = tk.Label(master=window)
focusWidget = tk.Label(master=window)

#config
window.title('chatGUI.py')
window.resizable(width=False,height=False)

logLabel.configure(font=("Times New Roman", 16, "bold"))
log.configure(font=("Times New Roman", 16)); #log.config(state=tk.DISABLED)
typing.configure(font=("Times New Roman", 16, "bold"))
inputLabel.configure(font=("Times New Roman", 16, "bold"))
inputEntry.configure(font=("Times New Roman", 16))

presentLabel.configure(font=("Times New Roman", 16, "bold"))
present.configure(font=("Times New Roman", 16)); #present.config(state=tk.DISABLED)
fileLabel.configure(font=("Times New Roman", 16, "bold"))
fileEntry.configure(font=("Times New Roman", 16))
imageLabel.configure(font=("Times New Roman", 16, "bold"))
imageEntry.configure(font=("Times New Roman", 16))

#layout
leftFrame.grid(row=0,column=0,padx=5,pady=5,sticky='nesw')
rightFrame.grid(row=0,column=1,padx=5,pady=5,sticky='nesw')

logLabel.grid(row=0,column=0,padx=5,pady=5,sticky='nw')
log.grid(row=1,column=0,padx=5,pady=5,sticky='nesw')
typing.grid(row=2,column=0,padx=5,pady=5,sticky='w')
inputLabel.grid(row=3,column=0,padx=5,pady=5,sticky='nw')
inputEntry.grid(row=4,column=0,padx=5,pady=5,sticky='nesw')

presentLabel.grid(row=0,column=0,padx=5,pady=5,sticky='nw')
present.grid(row=1,column=0,padx=5,pady=5,sticky='nesw')
fileLabel.grid(row=2,column=0,padx=5,pady=5,sticky='nw')
fileEntry.grid(row=3,column=0,padx=5,pady=5,sticky='nesw')
imageLabel.grid(row=4,column=0,padx=5,pady=5,sticky='nw')
imageEntry.grid(row=5,column=0,padx=5,pady=5,sticky='nesw')

#logic
sendTyping = True
focused = True
lastFocus = time.time()

def repaintLabel(widget, filePath):
    if procs.fileExists(filePath):
        data = procs.fileLoad(filePath)
        widget['text'] = data
def repaintText(widget, filePath, clear):
    if procs.fileExists(filePath):
        #widget.config(state=tk.NORMAL)
        data = procs.fileLoad(filePath)
        if clear:
            widget.delete('1.0', tk.END)
        widget.insert(tk.END, data)
        widget.see("end")
        #widget.config(state=tk.DISABLED)
def repaintLog():
    global focused, lastFocus
    if procs.fileExists(LOG_PATH):
        procs.fileMove(LOG_PATH, LOG_TEMP_PATH)
        repaintText(log, LOG_TEMP_PATH, False)
        currentTime = time.time()
        if not focused and lastFocus + ALERT_TIMER_SECONDS < currentTime:
            procs.playSound('./media/bloop.mp3')
            lastFocus = currentTime - ALERT_COOLDOWN
def refreshGUI():
    global sendTyping
    sendTyping = True
    repaintLog()
    repaintLabel(typing, TYPING_PATH)
    repaintText(present, PRESENT_PATH, True)
    refreshWidget.after(REFRESH_TIMER_MILISECONDS, refreshGUI)
def handle_keypress(event):
    global sendTyping

    if (event.char == '\n' or event.char == '\r') and (0 == (event.state & SHIFT_DOWN_MASK)):
        data = inputEntry.get("1.0",tk.END); data = data[:-1]
        if len(data) > 1:
            procs.fileSave(OUTPUT_MESSAGE_PATH, data)
            inputEntry.delete('1.0',tk.END)
        data = fileEntry.get()
        if len(data) > 0:
            procs.fileSave(OUTPUT_FILE_PATH, data)
            fileEntry.delete(0,tk.END)
        data = imageEntry.get()
        if len(data) > 0:
            procs.fileSave(OUTPUT_IMAGE_PATH, data)
            imageEntry.delete(0,tk.END)
    elif sendTyping and event.char != BACK_SPACE:
        procs.fileSave(OUTPUT_TYPING_PATH, '1')
        sendTyping = False
def handle_focus(event):
    global focused
    focused = True
def handle_focus_out(event):
    global lastFocus, focused
    focused = False
    lastFocus = time.time()

#go
window.bind('<Key>', handle_keypress) # KeyPress KeyRelease Enter FocusIn FocusOut
window.bind('<FocusIn>', handle_focus)
window.bind('<FocusOut>', handle_focus_out)
refreshGUI(); refreshWidget.after(REFRESH_TIMER_MILISECONDS, refreshGUI)


def gui_main():
    window.mainloop()
