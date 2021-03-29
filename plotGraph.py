import matplotlib.pyplot as plt
import numpy 

def movingaverage(interval, window_size):
    window = numpy.ones(int(window_size))/float(window_size)
    return numpy.convolve(interval, window, 'same')
    pass

file = "./out_images/out1.txt"
frame = []
static = []
dynamic = []
f = open(file)
for line in f:
    tup = line.split(",")
    frame.append(int(tup[0])/15)
    static.append(float(tup[1]))
    dynamic.append(float(tup[2])/2)
f.close()
static_av = movingaverage(static, 20)
dynamic_av = movingaverage(dynamic, 20)
plt.plot(frame, static_av, label = "Queue density")
plt.plot(frame, dynamic_av, label = "Dynamic density")
plt.xlabel("Time in seconds")
plt.ylabel("Traffic density")
plt.title("Traffic Density")
plt.legend()
plt.savefig("./out_images/out_graph.png")
