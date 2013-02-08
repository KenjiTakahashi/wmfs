next WMFS dev branch.

my (KenjiTakahashi) patches info:

* Xft support (by @draftcode) with fixed bars colors and UTF8.
* Multiple fonts support (see below).

### Multiple fonts
A [fonts][font] directive which offers declaring any number (see **note2**) of fonts to use with statuses.

The `wmfsrc` syntax is as follows:
```sh
[themes]
    [theme]
        [fonts]
            [font]
                name = "liberation mono:pixelsize=11"
            [/font]
            [font]
                name = "liberation mono:pixelsize=11:weight=bold"
            [/font]
            ...
        [/fonts]
    ...
    [/theme]
    ...
[/themes]
...
```

After that you can use `{<fontnumber><text>}` inside `^s` text part, to use different fonts.

So, in your status script, do:
```sh
^s[<position>;<color>;{1textusingfont1}textusingdefaultfont(0)]
```
**note0**: If you have a space just before curly block, you might want to place it *inside*, because XftTextExtents seems to ignore trailing whitespaces.

**note1**: It works only with Xft enabled, because I'm lazy. (It probably won't get fixed)

**note2**: 'Any number' means <=10, because of an ugly parser hack ;-). (It probably will get fixed)
