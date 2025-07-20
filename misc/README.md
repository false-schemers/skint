# SKINT Miscellania
                         
This directory contains miscellaneous SKINT files. 

The `nmakefile` is a make file for Microsoft NMAKE. To use it, first open Visual Studio IDE
and select the `Visual Stidio Command Prompt` tool from the `Tools` menu (for 64-bit tools,
press Start and type `x64 Native Tools Command Prompt for VS 2022` or whichever version you have). 
A terminal window will open. Switch to it and enter the following commands (it is assumed that your SKINT 
distributive is located at `C:\Users\Gwendolyn\Documents\skint_dist` – change it before proceeding):

```
pushd C:\Users\Gwendolyn\Documents\skint_dist
nmake /f misc\nmakefile
nmake /f misc\nmakefile test
nmake /f misc\nmakefile install
```

The executable will be installed into the `skint\bin` subdirectory of your Home directory.
If this is not what you want, just copy it manually wherever you wish.

