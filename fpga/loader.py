#!/bin/python

import sys

# bytes
FLASH_DEFAULT = 255
FLASH_SIZE = 8192
PAGE_SIZE = 64
PAGES = int(FLASH_SIZE/PAGE_SIZE)

LOAD_PROG_HIGH = 0x48
LOAD_PROG_LOW = 0x40
WRITE_PROG_PAGE = 0x4C
CHIP_ERASE = 0xAC80
PROG_ENABLE = 0xAC53

DELAY_MS = 10
ERASE_DELAY_MS = 100


flash = [FLASH_DEFAULT]*FLASH_SIZE
dirty = [0]*PAGES

infile = sys.argv[1]

# read hex
fd = open(infile, "r")
lines = fd.readlines()
fd.close()

# add to flash array
idx = 0
addr = 0
for line in lines:
    line = line.strip()
    myint = int(line, 16)
    if(idx==0):
        addr = myint & (FLASH_SIZE-1)
    else:
        flash[addr] = myint
    idx = ~idx

# dirty tags
for i in range(0, PAGES):
    for j in range(0, PAGE_SIZE):
        if(flash[(i*PAGE_SIZE)+j] != FLASH_DEFAULT):
            dirty[i] = 1
            break

# print writes
print("master_write_32 $master 0x0 0x" +
      format(PROG_ENABLE, '04x') +
      format(0, '04x'))
print("after " + str(ERASE_DELAY_MS))
print("master_write_32 $master 0x0 0x" +
      format(CHIP_ERASE, '04x') +
      format(0, '04x'))
print("after " + str(ERASE_DELAY_MS))
for i in range(0, PAGES):
    if(dirty[i] == 0):
        continue
    for j in range(0, PAGE_SIZE):
        addr = (i*PAGE_SIZE) + j
        addr_hex = format(int(addr/2) & 65535, '04x')
        if((addr & 1) == 1):
            cmd_hex = format(LOAD_PROG_HIGH, '02x')
        else:
            cmd_hex = format(LOAD_PROG_LOW, '02x')
        data_hex = format(flash[addr] & 255, '02x')
        print("master_write_32 $master 0x0 0x" +
            str(cmd_hex) +
            str(addr_hex) +
            str(data_hex))
    print("after " + str(DELAY_MS))
    addr = int(i*PAGE_SIZE/2) & 65535
    addr_hex = format(addr, '04x')
    cmd_hex = format(WRITE_PROG_PAGE, '02x')
    print("master_write_32 $master 0x0 0x" +
        str(cmd_hex) +
        str(addr_hex) +
        str(format(0, '02x')))
    print("after " + str(DELAY_MS))
            
               



#for b in flash:
#    print(str(hex(b)), end=" ")
