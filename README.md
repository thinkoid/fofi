# Font file identification

A program which identifies the type and/or content of a font file.

## Licensing, etc.

The program is a rewrite of the font file identification code in version 3 of xpdf. Because it keeps the original enum names it also preserves the copyright notices as well as the original licensing -- GPL2, GPL3, or both.

## Usage

```
$ fc-list | sed s,:.*,, | while read f; do [ ${f##*.} == "gz" ] && continue; echo "$f : $( ./fofi $f )"; done

...
/usr/share/fonts/noto/NotoSansKhmerUI-Thin.ttf : TrueType font
/usr/share/fonts/noto/NotoSerifDisplay-MediumItalic.ttf : TrueType font
/usr/share/fonts/noto/NotoSansDuployan-Regular.ttf : TrueType font
/usr/share/fonts/adobe-source-han-serif/SourceHanSerifTW-SemiBold.otf : OpenType container of CID-keyed CFF font
/usr/share/fonts/TTF/VeraMoBI.ttf : TrueType font
...
```
