import random

file = open("testfile.txt", "w")
for i in range(0, 3000):
	file.write(str(random.randint(0, 3000))+" ")
file.close()
