# GPIO Tool

## Help

```
USAGE
   -d: device node, default is /dev/gpio_mio
   -m: mode
       i: input mode
       o: output mode
       t: test mode
   -i: pin index
   -p: part, this is hardware interface
       default: mio
   -v: value, this just for output mode
RETURE:
    I. input mode
        1: pin is high
        0: pin is low
        -1: pin index error
    II: output mode
        0: operation is success
        -1: pin index error
    III: test mode
        0: operation is success
        -1: operation is failed
EXAMPLE:
    [buildroot@root ~]#  gpiotool -m t
    0
    [buildroot@root ~]#  gpiotool -m i -i 0
    1
    [buildroot@root ~]#  gpiotool -m o -i 0 -v 0
    0
    [buildroot@root ~]#  gpiotool -m i -i 0
    0
    [buildroot@root ~]#  gpiotool -m o -i 0 -v 1
    0
    [buildroot@root ~]#  gpiotool -m i -i 0
    1
    [buildroot@root ~]#

```
