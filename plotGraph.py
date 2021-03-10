import matplotlib.pyplot as plt
import numpy 

def movingaverage(interval, window_size):
    window = numpy.ones(int(window_size))/float(window_size)
    return numpy.convolve(interval, window, 'same')
    pass

file_name = input("Name of the file containing the output points: ")
file = "./out_images/"+ file_name + ".txt"
frame = []
static = []
dynamic = []
f = open(file)
for line in f:
    tup = line.split(", ")
    frame.append(int(tup[0]))
    static.append(int(tup[1]))
    dynamic.append(int(tup[2])/2)
f.close()
static_av = movingaverage(static, 20)
dynamic_av = movingaverage(dynamic, 20)
plt.plot(frame, static_av, label = "Queue density")
plt.plot(frame, dynamic_av, label = "Dynamic density")
plt.xlabel("frame")
plt.ylabel("density")
plt.title("Traffic Density")
plt.legend()
plt.savefig("./out_images/" + file_name + "Graph.png")
plt.show()
