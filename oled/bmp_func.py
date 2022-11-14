#!/bin/python

from PIL import Image
import numpy as np
import sys

path = sys.argv[1]
file = sys.argv[2]

img = np.array(Image.open(path + '/' + file + '.bmp'))
(rows, cols) = img.shape
# cols should be divisible by 8

bmp_bytes = []

for i in range(0, rows):
    for j in range(0, cols, 8):
        b = 0
        for k in range(0, 8):
            #print("i = " + str(i) + " j = ", str(j+k), "b = ", + img[i][j+k])
            if(img[i][j+k] == True):
                b = b + (1 << k)
        #print(str(hex(b)))
        bmp_bytes.append(b)

cmd_bytes = []

bc = bmp_bytes[0]
count = 0

LIMIT = 256

for b in bmp_bytes:
    if((b == bc) and (count != LIMIT)):
        count += 1
    elif(((b == bc) and (count == LIMIT)) or (b != bc)):
        cmd_bytes.append((count-1, bc))
        count = 1
    bc = b
cmd_bytes.append((count-1, bc))

#print(len(cmd_bytes))

#print("void draw_" + file + "(void){")
#for c in cmd_bytes:
#    print("    pixel_write" + str(c) + ";")
#print("}");

print("const uint8_t " + file + " [" + str(2*len(cmd_bytes)) + "] PROGMEM = {", end = "")
for j in range(0, len(cmd_bytes)-1):
    print(str(cmd_bytes[j][0]), end = ",")
    print(str(cmd_bytes[j][1]), end = ",")
print(str(cmd_bytes[len(cmd_bytes)-1][0]), end = ",")
print(str(cmd_bytes[len(cmd_bytes)-1][1]), end = "};")
print("");


#for x in cmd_bytes:
#    print(x)

#total = 0
#for x in cmd_bytes:
#    total = total + x[0]+1
#print(str(total))

