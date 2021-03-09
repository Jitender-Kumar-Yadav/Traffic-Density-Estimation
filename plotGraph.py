import matplotlib.pyplot as plt

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
    dynamic.append(int(tup[2])/300)
f.close()
plt.plot(frame, static, label = "Queue density")
plt.plot(frame, dynamic, label = "Dynamic density")
plt.xlabel("frame")
plt.ylabel("density")
plt.title("Traffic Density")
plt.legend()
plt.savefig("./out_images/" + file_name + "Graph.png")
plt.show()
