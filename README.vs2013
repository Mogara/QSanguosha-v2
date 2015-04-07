To build QSanguosha with VS2013
Tips: "~" stands for the folder where the repo is in.

1. Download and then install following packages: 
(1) QT libraries for Windows (Visual Studio 2013, 5.x) http://download.qt.io/official_releases/qt/
(2) QT Visual Studio Add-in (for Qt 5, the lastest edition) http://download.qt.io/official_releases/vsaddin/

2. Download the swigwin (swig for Windows, 2.0.9 or higher to ensure the support for Lua 5.2) http://sourceforge.net/projects/swig/files/swigwin/
Create a ~/tools/swig folder under your source directory. Unzip swigwin and copy all unzipped files to tools/swig. To make sure you copied them to the right directory hierarchy, check if this file exists: ~/tools/swig/swig.exe

3. Open Qsanguosha.sln right under ~/builds/vs2013, change the configuration to Release Qt5|Win32.

4. Right click project "QSanguosha" in your Solution Explorer, select "Properties", go to "Debugging" tab, set "Working Directory" to "$(ProjectDir)..\..\" (do not enter the quote marks). Then select "OK".

4.1. [optional] Right click "sanguosha.ts" in the folder "Translaton Files" in project "QSanguosha", select "lrelease".

4.2. [optional] Check version of qt and specify the installed qt at QT Opitions from QT menu in VS2013.
You will get sanguosha.qm at ~/builds.

5. You are now able to build the solution.
