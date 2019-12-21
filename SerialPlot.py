import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import threading

timeList = deque(maxlen=1000)
tempList = deque(maxlen=1000)

s = serial.Serial('COM18')
for i in range(20): s.readline()

def getSerialData ():
    while True:
        global s
        try:
            line = s.readline()
        except:
            return
        try:
            readt, readT = [float(x) for x in str(line, 'utf-8')[:-2].split()]
            timeList.append(readt)
            tempList.append(readT)
        except:
            pass

thread = threading.Thread(target=getSerialData)
thread.start()

def onlinePlot (i, timeList, tempList):
    ax.clear()
    ax.set_title("Temperature Monitor")
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Temperature (° C)")
    ax.plot(timeList, tempList)

print('Conected to %s.' % s.name)

fig = plt.figure()
ax = plt.axes()

ani = animation.FuncAnimation(fig, onlinePlot, fargs=(timeList, tempList), interval=1)
plt.show()

s.close()
