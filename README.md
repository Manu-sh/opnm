### Output Pnm :framed_picture:
###### common pnm file formats

This mini-library is for dealing with common PNM format.

#### Future (?)
- :white_check_mark: ~~add `PNM<T>(filename)` constructors or a factory method to read files from disk into a `PNM<T>` object~~
- :white_check_mark: ~~add `write_content(ostream&)`~~


Examples: [see here](https://github.com/Manu-sh/example-opnm)

The library was originally written for [this project](https://github.com/Manu-sh/cuda-mandelbrot), i decided to create a separate repository to be able to reuse the code more easily. 

There is duplicate code and the reason is that originally it was supposed to be only extremely efficient in creating and writing a pbm rgb type file only later it was expanded to include pbm and pgm, the correct approach would have been to avoid any kind of static polymorphism (template) and to use dynamic polymorphism (interfaces, vtable etc) and class inheritance. 

The in-memory representation of PNM<T> files is always binary, and include a padding when required by the format, see pbm. The raster data (aka pixmap) of an image w=3 h=2 in pbm format will occupy 2 bytes in memory. Any conversion to ascii occurs later when write_content() is called.


Learning resources

- https://en.wikipedia.org/wiki/Netpbm
- https://netpbm.sourceforge.net/doc/pbm.html
- https://netpbm.sourceforge.net/doc/pgm.html
- https://netpbm.sourceforge.net/doc/ppm.html

```cpp
PNM<pnm::monochrome_t> chessboard{1920, 1080};
bool color = pnm::monochrome_t::BLACK;

for (int h = 0; h < chessboard.height(); ++h, color = !color)
    for (int w = 0; w < chessboard.width(); ++w, color = !color)
        chessboard(h, w, color);

chessboard.write_file_content("chessboard-bin.pbm");
chessboard.write_file_content("chessboard-ascii.pbm", 1);

PNM<pnm::monochrome_t> pbm{3, 2};

pbm(0,0, {255, 0,   0}); // since bits aren't addressable you will use a different syntax
pbm(0,1, {0,   255, 0});
pbm(0,2, {0,   255, 0});

pbm(1,0, {255, 255, 0});
pbm(1,1, {255, 255, 255});
pbm(1,2, {0,   0,   0});

pbm.write_file_content("bin.pbm");
pbm.write_file_content("ascii.pbm", 1);

PNM<pnm::rgb<pnm::BIT_8>> ppm{3, 2};

ppm(0,0) = {255, 0,   0};
ppm(0,1) = {0,   255, 0};
ppm(0,2) = {0,   0,   255};

ppm(1,0) = {255, 255, 0};
ppm(1,1) = {255, 255, 255};
ppm(1,2) = {0,   0,   0};


ppm.write_file_content("bin.ppm");
ppm.write_file_content("ascii.ppm", 1);

PNM<pnm::grayscale<pnm::BIT_8>> pgm{3, 2};

pgm(0,0) = {255, 0,   0};
pgm(0,1) = {0,   255, 0};
pgm(0,2) = {0,   0,   255};

pgm(1,0) = {255, 255, 0};
pgm(1,1) = {255, 255, 255};
pgm(1,2) = {0,   0,   0};

pgm.write_file_content("bin.pgm");
pgm.write_file_content("ascii.pgm", 1);

```

###### Copyright © 2025, [Manu-sh](https://github.com/Manu-sh), s3gmentationfault@gmail.com. Released under the [MIT license](LICENSE).

