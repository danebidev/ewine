# ewine

A small and simple cli program to manage wine prefix. Can also automatically install/remove dxvk in the prefixes.

I made this mainly because on Gentoo I would need to build webkit-gtk for lutris, and I really wanted to avoid that as much as possible. My shell scripts to manage prefixes were also starting to become too complex. This isn't really meant to have all the features lutris, just what I need on my system.

You have to manually download wine and dxvk, and then add it to the program.I made it this way because automatically downloading stuff would make the program to complex and I prefer being able to use my own  builds anyway. 

## Build

```
meson setup build
meson compile -C build
```
