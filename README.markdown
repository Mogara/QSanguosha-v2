Open Source Sanguosha
==========

| Homepage:      | https://qsanguosha.org                        |
|----------------|-----------------------------------------------|
| API reference: | http://gaodayihao.github.com/QSanguosha/api   |
| Documentation: | https://github.com/gaodayihao/QSanguosha/wiki (Chinese) |

Introduction
----------

Sanguosha is both a popular board game and online game,
this project try to clone the Sanguosha online version.
The whole project is written in C++, 
using Qt's graphics view framework as the game engine.
I've tried many other open source game engines, 
such as SDL, HGE, Clanlib and others, 
but many of them lack some important features. 
Although Qt is an application framework instead of a game engine, 
its graphics view framework is suitable for my game developing.

Features
----------

1. Framework
    * Open source with Qt graphics view framework
    * Use FMOD as sound engine
    * Use plib as joystick backend 
    * Use Lua as AI script

2. Operation experience
    * Full package (include all yoka extension package)
    * Drag card to target to use card
    * Keyboard shortcut
    * Cards sorting (by card type and card suit)
    * Multilayer display when cards are more than an upperlimit

3. Extensible
    * Some MODs are available based on this game

How to build it to play
-------

You can see the official site:
"太阳神三国杀 Windows 及 Android 构建指南" http://mogara.org/build-tutorial/
or unofficial document like:
"如何获得最新的太阳神三国杀 自己Qt编译" http://jingyan.baidu.com/article/6f2f55a15d28c9b5b83e6c5c.html