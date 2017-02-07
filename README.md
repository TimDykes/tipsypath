### Tipsy Path

Tool to make interpolated Splotch scenefile for a set of tipsy data  

Interpolation uses time from tipsy headers to interpolate linearly across time regardless of snapshot spacing, hence uneven snapshot spacing is acceptable.

```
make
./tipsypath /path/to/filenames.txt snapshot_name(no extension) n_frames
```

filenames.txt is a list of the tipsy files to use (full path)
snapshot_name is the full path and name of the tipsy files without .XXXXX extension
n_frames is number of frames to interpolate over