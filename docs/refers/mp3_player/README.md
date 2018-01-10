Via Buildroot, you can build your own mp3 player.

##Branch

There are two branches:

* No\_Lcd\_Version  
  "no-lcd-version" can play music but no gpio output,
* master  
  "master" branch can show the name of the song which is playing, but you have to prepare a set of 16X2 LCD

##Detail

I writed all the detail about this mp3 in my blog and gitbook,
https://www.gitbook.com/book/hugh712/raspberry\_mp3/details

only chinese version,if there is someone need Eng. version, just let me know.

##Build

only thing you have to do is:

```
 cd mp3\_player 
 make
```

##Image to SDCard

The result is a image file, assume your sdcard path is "/dev/sdd", only thing you have to do is:
```
 sudo dd if=/image/sdcard.img of=/dev/sdd
```
## Power On

When you power on your pi2 with a speaker coneected on it,
then plug in your USB stick with mp3 in it, your pi2 will play songs soon.

	
