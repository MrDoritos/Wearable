#!/bin/python3
import tkinter as tk
import PIL
from PIL import Image, ImageTk

class DeviceInput:
    def __init__(self, parent):
        pass

class DisplayInterface:
    def __init__(self, parent, width, height, window_width:int=None, window_height:int=None):
        self.window_width = window_width if window_width is not None else width
        self.window_height = window_height if window_height is not None else height
        self.width = width
        self.height = height
        self.buffer = Image.new('L', (width, height))
        self.photoimage = ImageTk.PhotoImage(self.buffer)
        self.label = tk.Label(parent, borderwidth=0, background='black')
        #tk.Canvas.__init__(self, parent, width=self.window_width, height=self.window_height, background='black')
        #tk.PhotoImage.__init__(self, self.buffer, master=parent, height=height, width=width)
        #self.pack()
        self.label.pack()
        self.label.configure(image=self.photoimage)

    def draw(self):
        #self.label.configure(image=self.photoimage)
        #self.label.image
        self.label.pack()

    def test(self):
        for i in range(min(self.width, self.height)):
            self.buffer.putpixel((i,i), i)
        #self.buffer.putpixel()

class Controller(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.bind('<Escape>', lambda x: self.destroy)

if __name__ == "__main__":
    root = Controller()
    disp = DisplayInterface(root, 128, 128)
    disp.test()
    disp.draw()

    root.mainloop()

