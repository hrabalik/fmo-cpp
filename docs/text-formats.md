# Text formats used by the desktop executable

## Ground truth text format

The file begins with the integers `W`, `H`, `F`, `L` on a separate line. These denote: width, height, number of frames, and the number of non-empty frames.

Following are `L` lines with the following format: integer `I`, `1 <= I <= F`, specifies the frame number that is being described on this line. Integer `N`, `1 <= N <= W*H`, specifies the number of pixels that will be listed. The following `N` integers are pixel indices in range `1` to `W*H` inclusive. Empty frames are not listed.

Both frame numbers and pixel indices use MATLAB's one-based indexing. The following conversions apply:
```
x = (i - 1) div H
y = (i - 1) mod H
```
where `i` is the one-based index, and `{x,y}` are zero-based coordinates.
