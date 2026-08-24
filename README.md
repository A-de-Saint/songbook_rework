# Songbook rework

> Disclaimer:  
> You will most likely find no use for a czech campfire songbooks manager written in C for Linux.

### About this project

This is a happiness project to make a working **songbook manager** for **czech campfire songs with chords**. It allows you to download songs that get parsed from html and then organize them to songbooks, which are either `.tex` or `.html` - both can be printed into `.pdf`.

I'm still working on this project, it's stability and some proper releases.

### Dependencies and environment

Songbook rework is coded in C and is made to run on both Linux and Windows. On Linux, simply use **Makefile**. **MinGW** with GCC is needed for compilation on Windows.  
For it to work properly, you need **Google Chrome** or **Chromium Browser** and **libcurl**.

### How it works

Upon starting the executable, terminal UI will show. Use your keyboard to operate it. You can manage songbooks and songs. If you choose to add a new song, the program will check if it's available on [Akordy Kytary](https://akordy.kytary.cz/), then gets the HTML source, parses it and saves the song in a ChordPro-like format. Transposition is also possible.

### What purpose does this project serve?

Why make such a small utility for czech songbooks in C and English? Because I love low-level programming languages and this idea sounded like a little bit of everything I needed to practice C. And who knows - maybe someone finds some use for it someday.
