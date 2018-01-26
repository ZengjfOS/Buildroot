# GPIO Tool

## Help

```
USAGE
   -d: device node, default is /dev/gpio_mio
   -m: mode
       e: enable pwm
       f: set freqency
       d: disable pwm
       ef/fe: set freqency and start pwm
       t: test mode
   -f: freqency for pwm controler
EXAMPLE
    [buildroot@root ~]#  ./buzzertool -m t
    0
    [buildroot@root ~]#  ./buzzertool -m e
    0
    [buildroot@root ~]#  ./buzzertool -m ef -f 3000
    0
    [buildroot@root ~]#  ./buzzertool -m d
    0
    [buildroot@root ~]#
```
