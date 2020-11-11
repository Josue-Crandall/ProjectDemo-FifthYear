# Note: I've never written python before, or used tkintr.
# The goal here is to finish in 3-4 hours. Woo!

import tkinter as tk
import os

INPUT_TEMP = "./CRYPT_GUI_INPUT_TEMP"
OUTPUT_TEMP = "./CRYPT_GUI_OUTPUT_TEMP"

mode = 1
genMode = False

def rmFileIfExists(path):
    if os.path.exists(path):
        os.remove(path)
def callTools(line):
    rmFileIfExists("cTool.conf")
    file = open("cTool.conf", "w+")
    file.write(line)
    file.close()
    os.system("./cTool.elf")
    rmFileIfExists("cTool.conf")
def writeInputThenCallTools(textWidget, line):
    rmFileIfExists(INPUT_TEMP)
    file = open(INPUT_TEMP, "w+")
    file.write(textWidget.get("1.0", tk.END))
    textWidget.delete("1.0",tk.END)
    file.close()
    callTools(line)
    rmFileIfExists(INPUT_TEMP)
def unsetBRel():
    global genMode
    genMode = False
    modeButton1["relief"] = tk.RAISED
    modeButton2["relief"] = tk.RAISED
    modeButton3["relief"] = tk.RAISED
    modeButton4["relief"] = tk.RAISED
    invisKey2()
    visEButton()
    ctEntry.delete("1.0",tk.END)
def mode1():
    global mode
    unsetBRel()
    key1Label["text"] = "Symmetric Key Path"
    modeButton1["relief"] = tk.SUNKEN
    mode = 1
def mode2():
    global mode
    unsetBRel()
    modeButton2["relief"] = tk.SUNKEN
    key1Label["text"] = "Secret Key Path"
    visKey2()
    key2Label["text"] = "Public Key Path"
    mode = 2
def mode3():
    global mode
    unsetBRel()
    key1Label["text"] = "Ratchet Key Path"
    modeButton3["relief"] = tk.SUNKEN
    mode = 3
def mode4():
    global mode
    unsetBRel()
    key1Label["text"] = "Otp Key Path"
    modeButton4["relief"] = tk.SUNKEN
    mode = 4
def keyGen():
    global genMode
    if not genMode:
        unsetBRel()
        if mode == 1:
            key1Label["text"] = "Symmetric key file path"
            key1Entry.delete(0,tk.END)
        elif mode == 2:
            key1Label["text"] = "Private key file path"
            key1Entry.delete(0,tk.END)
            visKey2()
            key2Label["text"] = "Public key file path"
            key2Entry.delete(0,tk.END)
        elif mode == 3:
            key1Label["text"] = "Ratchet Key file path"
            key1Entry.delete(0,tk.END)
            visKey2()
            key2Label["text"] = "Matching Key file path"
            key2Entry.delete(0,tk.END)
        elif mode == 4:
            key1Label["text"] = "Otp Key file path"
            key1Entry.delete(0,tk.END)
            visKey2()
            key2Label["text"] = "Matching Key file path"
            key2Entry.delete(0,tk.END)
        invisEButton()
        genMode = True
    else:
        switch = {
            1: f"\"function\" = \"symmetricGen\" \"outputFile\" = \"{key1Entry.get()}\"",
            2: f"\"function\" = \"asymmetricGen\" \"privateKeyPath\" = \"{key1Entry.get()}\" \"publicKeyPath\" = \"{key2Entry.get()}\"",
            3: f"\"function\" = \"dratchetGen\" \"outputFile1\" = \"{key1Entry.get()}\" \"outputFile2\" = \"{key2Entry.get()}\"",
            4: f"\"function\" = \"otpGen\" \"outputFile1\" = \"{key1Entry.get()}\" \"outputFile2\" = \"{key2Entry.get()}\" \"len\" = \"50000\"",
        }
        callTools(switch.get(mode))
        ctEntry.delete("1.0",tk.END)
        ctEntry.insert(tk.END,"Attempted key creation!\n\n(see console output for detail)")
def encryptAttempt():
    switch = {
        1: f"\"function\" = \"symmetricEnc\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        2: f"\"function\" = \"asymmetricEnc\" \"publicKeyPath\" = \"{key2Entry.get()}\" \"privateKeyPath\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        3: f"\"function\" = \"dratchetEnc\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        4: f"\"function\" = \"otpEnc\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
    }
    writeInputThenCallTools(ptEntry, switch.get(mode))
    file = open(OUTPUT_TEMP, "r")
    ptEntry.insert(tk.END, file.read())
    file.close()
    rmFileIfExists(OUTPUT_TEMP)
def decryptAttempt():
    switch = {
        1: f"\"function\" = \"symmetricDec\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        2: f"\"function\" = \"asymmetricDec\" \"publicKeyPath\" = \"{key2Entry.get()}\" \"privateKeyPath\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        3: f"\"function\" = \"dratchetDec\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
        4: f"\"function\" = \"otpDec\" \"keyFile\" = \"{key1Entry.get()}\" \"inputFile\" = \"{INPUT_TEMP}\" \"outputFile\" = \"{OUTPUT_TEMP}\"",
    }
    writeInputThenCallTools(ctEntry, switch.get(mode))
    file = open(OUTPUT_TEMP, "r")
    ctEntry.insert(tk.END, file.read())
    file.close()
    rmFileIfExists(OUTPUT_TEMP)
def visKey2():
    key2Label.grid(row=7,column=0,padx=5,pady=5,stick="nesw")
    key2Entry.grid(row=8,column=0,padx=5,pady=5,stick="nesw")
def invisKey2():
    key2Label.grid_forget()
    key2Entry.grid_forget()
def visEButton():
    eButton.grid(row=10,column=0,padx=5,pady=5,stick="nesw")
    dButton.grid(row=11,column=0,padx=5,pady=5,stick="nesw")
def invisEButton():
    eButton.grid_forget()
    dButton.grid_forget()

window = tk.Tk()
window.title("cryptGUI.py")
window.option_add("*Font","Conolas 16")
window.resizable(width=False,height=False)

modeFrame = tk.Frame(relief=tk.RAISED,master=window)
modeButton1 = tk.Button(master=modeFrame,text="symmetric", command=mode1,relief=tk.SUNKEN)
modeButton2 = tk.Button(master=modeFrame,text="asymmetric", command=mode2)
modeButton3 = tk.Button(master=modeFrame,text="dratchet", command=mode3)
modeButton4 = tk.Button(master=modeFrame,text="otp", command=mode4)
key1Label = tk.Label(master=modeFrame,text="Symmetric Key Path")
key1Entry = tk.Entry(master=modeFrame); key1Entry.insert(tk.END, "./key.txt")
key2Label = tk.Label(master=modeFrame)
key2Entry = tk.Entry(master=modeFrame)
genLabel = tk.Label(master=modeFrame,text="WARNING overwrites:")
genButton = tk.Button(master=modeFrame,text="Generate new key", command=keyGen)
emptyLabel1 = tk.Label(master=modeFrame,text="    ")
emptyLabel2 = tk.Label(master=modeFrame,text="    ")
emptyLabel3 = tk.Label(master=modeFrame,text="    ")
eButton = tk.Button(master=modeFrame,text="Encrypt", command=encryptAttempt)
dButton = tk.Button(master=modeFrame,text="Decrypt", command=decryptAttempt)
textFrame = tk.Frame(relief=tk.GROOVE,master=window)
ptEntry = tk.Text(master=textFrame,height=13); ptEntry.insert(tk.END, "plain text goes here")
ctEntry = tk.Text(master=textFrame,height=13); ctEntry.insert(tk.END, "cipher text goes here\n\n(Feedback messages may show up here)")

window.columnconfigure(0,weight=5)
window.columnconfigure(1,weight=1)
modeFrame.grid(row=0,column=1,padx=5,pady=5,sticky="nesw")
textFrame.grid(row=0,column=0,padx=5,pady=5,sticky="nesw")
genLabel.grid(row=0,column=0,padx=5,pady=5,stick="nesw")
genButton.grid(row=0,column=1,padx=5,pady=5,stick="nesw")
emptyLabel1.grid(row=1,column=1,padx=5,pady=5,stick="nesw")
modeButton1.grid(row=2,column=0,padx=5,pady=5,stick="nesw")
modeButton2.grid(row=2,column=1,padx=5,pady=5,stick="nesw")
modeButton3.grid(row=3,column=0,padx=5,pady=5,stick="nesw")
modeButton4.grid(row=3,column=1,padx=5,pady=5,stick="nesw")
emptyLabel2.grid(row=4,column=1,padx=5,pady=5,stick="nesw")
key1Label.grid(row=5,column=0,padx=5,pady=5,stick="nesw")
key1Entry.grid(row=6,column=0,padx=5,pady=5,stick="nesw")
emptyLabel3.grid(row=9,column=0,padx=5,pady=5,stick="nesw")
visEButton()
ptEntry.grid(row=0,column=0,padx=5,pady=5,stick="nesw")
ctEntry.grid(row=1,column=0,padx=5,pady=5,stick="nesw")

window.mainloop()

