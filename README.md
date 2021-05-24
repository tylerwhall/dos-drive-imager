# dos-drive-imager

Creates a raw disk image by reading using BIOS calls and writing the output to
a file. Useful for backing up a drive from a retro DOS machine if the drive
can't be physically removed.

I could not find a tool that did exactly this and actually worked on a 286.
This is essentially dd, and there is a dd for DOS, but I could not get it to
work. Hence this very simple program.

Requires openwatcom compiler.
