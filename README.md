# fitsThumbnailer
generate nautilus thumbnails for fit(s) files

needs:
```bash
sudo apt-get install libopencv-dev libcfitsio-dev
```

run:
```bash
./fit_thumbnailer <input_fits_file> <output_thumbnail_file> [thumbnail_size]
```

register with nautilus:
```bash
cp fits-thumbnailer.thumbnailer ~/.local/share/thumbnailers/
sudo cp fit_thumbnailer /usr/local/bin/
nautilus -q
```
