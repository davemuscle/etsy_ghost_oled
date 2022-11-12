#!/bin/python

from PIL import Image
import numpy as np

file = 'Cropped_000'
img = np.array(Image.open('bitmaps/Cropped_000.bmp'))
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

for b in bmp_bytes:
    if((b == bc):
        count += 1
    else:
        cmd_bytes.append((count, bc))
        count = 1
    bc = b
cmd_bytes.append((count, bc))

print("void draw_" + file + "(void){")
for c in cmd_bytes:
    print("    pixel_write" + str(c) + ";")
print("}");
#total = 0
#for x in cmd_bytes:
#    total = total + x[0]

