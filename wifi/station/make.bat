@echo off

IF "%1%" == "mc" (
    idf.py menuconfig
    idf.py -p COM17 -b 921600 flash monitor

) ELSE IF "%1%" == "f" (
    idf.py -p COM17 -b 921600 flash

) ELSE IF "%1%" == "m" (
    idf.py -p COM17 -b 921600 monitor

) ELSE IF "%1%" == "fm" (
    idf.py -p COM17 -b 921600 flash monitor

) ELSE IF "%1%" == "mu" (
    idf.py menuconfig

) ELSE IF "%1%" == "a" (
    if exist build ( rmdir /S /Q build )
    idf.py build

) ELSE IF "%1%" == "clr" (
    if exist sdkconfig ( del sdkconfig )
    if exist build ( rmdir /S /Q build )

) ELSE IF "%1%" == "b" (
    idf.py build

) ELSE IF "%1%" == "app" (
    idf.py app

) ELSE IF "%1%" == "e" (
    idf.py -p COM17 -b 921600 erase_flash

) ELSE (
    echo help
    echo "  : mc    # make.bat mc   ----> make menuconfig and make flash monitor"
    echo "  : f     # make.bat f    ----> make flash"
    echo "  : m     # make.bat m    ----> make monitor"
    echo "  : fm    # make.bat fm   ----> make flash monitor"
    echo "  : a     # make.bat a    ----> rm -rf build;make all"
    echo "  : b     # make.bat b    ----> only build"
    echo "  : mu    # make.bat mu   ----> make menuconfig"
    echo "  : e     # make.bat e    ----> erase flash"
    echo "  : clr   # make.bat clr  ----> clear build
    echo "  :       # make.bat      ----> help"
)



