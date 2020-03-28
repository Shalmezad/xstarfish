# xstarfish

xstarfish: X wallpaper generator

Written by:

* Mars Saxman

With help from:
* Sebastien Loisel's 'zut'
* Adrian Bridgett
* Philip Derrin
* "other peoples"

Maintained by:

* Richard Wardin

## Nomenclature

"Starfish", with capitals, is a wallpaper generator program. "xstarfish"
is the unix port of this program. Neither of these entities has any
connection to "starfish", the xscreensaver hack. I regret the confusion.

## Description

Starfish generates colourful, tiled images using random numbers fed through
mathematical functions. It does not use source image files, so it can generate
its images nearly forever without running out of material.

Once it has created an image, Starfish applies it to the root window of
your display as the background pixmap. Since Starfish images are
seamlessly tiled, the pixmap will wrap around forming a "wallpaper" effect
behind your windows. This allows you to customize your desktop, as often
as you wish, with a new look and without much work.

## Usage

There is no GUI control panel, but Starfish is quite simple to control on a
command line. There are not many options. In fact you can run Starfish without
any command line options at all:

```
xstarfish
```

This will generate a 256 pixel by 256 pixel wallpaper pattern. If you don't
like it, simply run Starfish again and it will replace the old pattern.

Perhaps you have an older machine that runs slowly, or you don't like big
bold patterns. You can instruct Starfish to create a smaller-than-normal
pattern:

```
xstarfish --size small
```

Valid sizes are small, medium, large, and full. These sizes are all
relative to the size of your screen, and they are all slightly randomized.
While two "medium" patterns will be roughly similar, they are unlikely to
come out exactly the same. "full" size, however, is always exactly the
same size as your default screen.

If you really don't care how big the patterns are and you just want
variety, use

```
xstarfish --size random
```

While Starfish will happily create a new desktop pattern any time you ask
for one, the ultimate lazy person's solution is to run Starfish in "daemon
mode". In this mode, Starfish will create patterns automatically at a time
interval you determine. Perhaps you want a new pattern every day:

```
xstarfish --daemon 1 day
```

Or you are a real graphics junkie and you want Starfish to spit out
patterns more rapidly:

```
xstarfish --daemon 30 seconds
```

Recognized time units are days, weeks, seconds, and minutes. When you run
Starfish in daemon mode, it will appear to quit immediately, taking you
back to the command line. This is an illusion; Starfish has merely put
itself in the background and will continue to create new patterns
indefinitely. You can shut it down with the standard "killall" command,
like this:

```
killall xstarfish
```

Finally, you can direct Starfish to save its output as a PNG file instead
of applying it to the X root window:

```
xstarfish --outfile wallpaper.png
```

These are the basics. For a complete listing of Starfish command line
options, type

```
xstarfish --usage
```

This will print out a list of Starfish's options and what they do, but
will not create a new pattern.

## The MacOS Version

Starfish was originally written for the MacOS. The Mac source code is not
included in this package.

Copyright (c) 1999,2000 by Mars Saxman.
Copyright (c) 1998 of the bits taken from zut by Sebastien Loisel.
Copyright (c) 2000 (makepng.c) by Philip Derrin.
