import matplotlib.pyplot as plt
import numpy 

method = input("Utility plot using which method, 1/2/3/4: ")
file = "./out_images/out_method_" + method + ".txt"
parameter = []
time = []
utility = []
f = open(file)
for line in f:
    tup = line.split(",")
    parameter.append(int(tup[0]))
    time.append(float(tup[1]))
    utility.append(100*(1-float(tup[2])))
f.close()

#plotting time versus parameter plot
plt.plot(parameter, time, label = "Parameter-Time Plot", marker='o', color='r')
if(method == "1"):
    plt.xlabel("X-parameter, no of frames skipped")
elif(method == "2"):
    plt.xlabel("Resolution, XxY")
elif(method == "3" or method == "4"):
    plt.xlabel("Number of Threads")
plt.ylabel("Time in seconds")
plt.title("Parameter-Time Plot")
plt.legend()
plt.savefig("./out_images/method_" + method + "_time.png")
plt.close()

#plotting utility versus time plot
plt.plot(time, utility, label = "Utility-Time Tradeoff Plot")
plt.xlabel("Time in seconds")
plt.ylabel("Utility as a percentage")
plt.title("Utility-Time Tradeoff Plot")
plt.legend()
plt.savefig("./out_images/method_" + method + "_utility.png")
plt.close()
