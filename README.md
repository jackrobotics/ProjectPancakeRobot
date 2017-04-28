# ProjectPancakeRobot
## control robot arm protocol : 
```
0xFF 0xFF 0x08 0x01 [Y step &0xFF00] [Y step &0x00FF] [A step &0xFF00] [A step &0x00FF] [B step &0xFF00] [B step &0x00FF] [checksumm]
  
0xFF 0xFF -> Header
0x08 -> length
0x01 -> instruction
[Y step &0xFF00] = Y step (2 byte)
[Y step &0x00FF] = Y step (2 byte)
[A step &0xFF00] = A step (2 byte)
[A step &0x00FF] = A step (2 byte)
[B step &0xFF00] = B step (2 byte)
[B step &0x00FF] = B step (2 byte)
[checksum] = ~(0x01 + [Y step &0xFF00] + [Y step &0x00FF] + [A step &0xFF00] + [A step &0x00FF] + [B step &0xFF00] + [B step &0x00FF]);
```

## control servo,magnat protocol : 
```
0xFF 0xFF 0x04 0x02 [servo_angle] [magnat_on_off] [checksumm]

0xFF 0xFF -> Header
0x04 -> length
0x02 -> instruction
[servo_angle] = 0-180 to hex
[magnat_on_off] = 1,0 to hex
[checksum] = ~(0x02 + [servo_angle] + [magnat_on_off]);
```

## read protocol : 
```
0xFF 0xFF 0x02 0x03 0xFC

0xFF 0xFF -> Header
0x02 -> length
0x03 -> instruction
0xFC -> checksum
```

## sethome protocol : 
```
0xFF 0xFF 0x02 0x04 0xFB

0xFF 0xFF -> Header
0x02 -> length
0x04 -> instruction
0xFC -> checksum
```

## rotary tray protocol : 
```
0xFF 0xFF 0x04 0x05 [dir] [step] [checksum]

0xFF 0xFF -> Header
0x04 -> length
0x05 -> instruction
[dir] -> 0,1
[step] -> step increment
[checksum] -> checksum
```
