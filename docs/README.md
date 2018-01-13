# README

## Analysis

* [0004_Freescale_SD_img_Script.md](0004_Freescale_SD_img_Script.md)
* [0003_Custom_Buildroot_Filesystem.md](0003_Custom_Buildroot_Filesystem.md)
* [0002_Freescale_i.MX6.md](0002_Freescale_i.MX6.md)
* [0001_Python3_Config_Hacking.md](0001_Python3_Config_Hacking.md)

## Article

* [Linux OTG当串口、网口、U盘](http://www.cnblogs.com/zengjfgit/p/8270050.html)
* [How do I Add a A Package To Buildroot Which Is Available In A Git Repository?](https://stackoverflow.com/questions/8014991/how-do-i-add-a-a-package-to-buildroot-which-is-available-in-a-git-repository)  
  文章中有描述如何获取仓库分支的方法：`MYPKG_VERSION = some_commit_id_or_tag_or_branch_name`

## Buildroot Patch

使用git管理源代码，生成git Patch，直接用`git diff > pach_name_with_number_at_start.patch`，放在package目录中，不用指定，系统会自动打补丁：  
* [0001-fix-makefile.patch](customize/package/canopensocket/0001-fix-makefile.patch)


