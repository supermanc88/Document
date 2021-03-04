# ubuntu18.04开机默认进入命令行模式/用户图形界面

一、开机默认进入命令行模式
 1、输入命令：`sudo systemctl set-default multi-user.target`
 2、重启：reboot
 要进入图形界面，只需要输入命令`sudo startx`
 从图形界面切换回命令行：ctrl+alt+F7

二、开机默认进入图形用户界面
 1、输入命令：`sudo systemctl set-default graphical.target`
 2、重启：reboot
 要进入命令行模式：ctrl+alt+F2
 从命令行切换到图形界面：ctrl+alt+F7