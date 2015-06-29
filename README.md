# pixQL - BETA
SQL inspired command-line image processing. *Note - very much still a work in progress; everything is subject to change.*

**WARNING** Currently only works with basic BMP files. Does not handle indexed BMPs or any kinds of RLE compression or anything like that. At its current stage, pixQL is working on experimenting with and finalizing its design before I'll do more work with compatibility/robustness.

That said, any bug reports, compatibility gaps, suggestions, etc... are welcome!

**Update:** Added a simple bash script (pixQL/example_scripts/pixql_c) that adds other image compatibility to pixql. It simply uses ImageMagick to convert the files you supply it to and from BMP in passing them to pixQL. Takes same arguments as pixQL.

**Goal:** Use simple, clean, composable rules to allow for precise pixel-level image operations.

*This document is rather informal, as pixQL is still early in development. Also, I've never written much technical documentation before- any help or suggestions welcome!*

##overview
The general idea is that you can supply an input image, and define a subset of pixels that you'd like to "select" (example: "give me all the pixels on the left half of the image"). You can then specify an operation you'd like to be applied on each of those pixels ("invert the pixels"). Using a series of selections and operations, you can perform coplex image manipulations.

##installation
```
# note- currently only tested on OSX Yosemite. In theory, should work on most *nix style stuff...
git clone git@github.com:Phildo/pixQL.git
cd pixQL
make
mv pixql /usr/local/bin/
```

## usage
`pixql [-i input_file] [-o output_file] [-q query | -qf query_file]`

##example simple usage
```
# turn all red pixels green
pixql -i in.bmp -o out.bmp -q "SELECT WHERE COLOR = #FF0000FF; OPERATE SET COLOR = #00FF00FF;"

# copy red channel into green channel
pixql -i in.bmp -o out.bmp -q "OPERATE SET G = R;"

# add white 1px 100x100 grid
pixql -i in.bmp -o out.bmp -q "SELECT WHERE ROW % 100 = 0 OR COL % 100 = 0; OPERATE SET COLOR = WHITE;"

# create 100x100 image with 10x10 checkerboard pattern
pixql -o out.bmp -q "BLACK(100,100); SELECT WHERE ROW % 20 < 10; OPERATE SET COLOR = WHITE; SELECT WHERE COL % 20 < 10; OPERATE SET R = 255-OUT.R; OPERATE SET G = 255-OUT.G; OPERATE SET B = 255-OUT.B;"
```

## query syntax
While *inspired* by the ideas of other SQL's out there, there are a couple differences.

Every query consists of one or more semicolon separated *statements*: an optional `init` statement, followed by any number of `procedure` statements.

### init
This statement sets up the canvas on which you will perform operations. It takes the form of a single word, followed by optionally specifying dimensions (in parenthesis), followed by a semicolon.

##### example init statement
`CLEAR(100,200);` - creates a clear image 100px wide and 200px tall

The options for initializer word are as follows:

- `BLACK` : sets all pixels to black (#000000FF)
- `WHITE` : sets all pixels to white (#FFFFFFFF)
- `CLEAR` : sets all pixels to clear (#00000000)
- `COPY`  : copies input image (if size differs, copies window origin top-left)

If no initializer word, `COPY` is default. If no dimensions, dimensions of input is default. If no input file, MUST specify initializer word AND dimensions.

### procedure
A procedure consists of any number of `SELECT`s, followed by any number of `OPERATE`s.

Each `SELECT` will add to a selection mask of pixels that meet its criteria.

Each following `OPERATE` will perform its operation on each of the selected pixels, one by one.

When another `SELECT` is encountered after a chain of `OPERATE`s, it will be considered the start of a new procedure, and will clear the selection mask.

#### SELECT
Takes the form of `SELECT WHERE [expression];`. Sets selection mask to 1 where `[expression]` result is non-0.

##### example SELECT
```
SELECT WHERE ROW <= 200 AND ROW%100 = 0;
```

### OPERATE
Takes the form of `OPERATE SET [attribute] = [expression];`. Performs operation on current pixel specified by selection mask, and sets attribute an corresponding pixel.

##### example OPERATE
```
OPERATE SET COLOR = WHITE;
```

### expression
A composition of pretty standard SQL-like sets of operations.

`x` and `y` in the following examples are either constants, or `attribute`s.

- `x AND y` - 1 if `x` and `y` are non-0. otherwise, 0.
- `x OR y` - 1 if either `x` or `y` are non-0. otherwise, 0.
- `x [+|-|*|/] y` - self explanatory arithmetic operations. `/` is of note, because operands are int, will get floor'd.
- `x [<|<=|=|!=|>=|>] y` - 1 if true, otherwise, 0.
- `(x AND y) * z` - parenthesis will be executed first (self explanatory)

### attribute
A property of a pixel.

Can specify pixel by TARGET[INDEX_COL,INDEX_ROW].PROPERTY

If index left out, assumed to be currently checking index in selection mask.

If target left out, assumed to be IN (EXCEPT in the case of the lvalue of an OPERATE, where it is assumed to be OUT)

#### properties
- `R` - pixel's red component (0-255)
- `G` - pixel's green component (0-255)
- `B` - pixel's blue component (0-255)
- `A` - pixel's alpha component (0-255)
- `COLOR` - 32 bit int rep of pixel's color (#00000000 - #FFFFFFFF, formatted #RRGGBBAA)
- `ROW` - pixel's row (0 - (HEIGHT-1), 0 is top)
- `COL` - pixel's column (0 - (WIDTH-1), 0 is left)
- `WIDTH` - target's width
- `HEIGHT` - target's height

## but why tho
A concrete example of pixQL's use might be "select all the pixels that fall on a 10x10 grid throughout the image" and "set those pixels to white". This will simply create a grid on your image.

Because the 'rules' are simple (define some selection to be applied to each pixel to determine if it's selected, then do some specific operation on each of those selected pixels), the approach to solving a problem is very different than that of using a traditional editor.

For example, if I want to clear all non-black pixels from an image, I can reason about breaking down that problem into it's rigid parts ("ok, by 'non-black' I mean where the color != #000000FF, and by 'clear it' I mean set the color to #00000000"). But in photoshop, I instead am relying on the problem being solved externally ("ok, how can I find the right menu that deals with this kind of operation? what's the word the designers of photoshop used to describe this... I hope if I google it in plain text it might link to a forum post or something of someone with the same problem?").

A full understanding of photoshop would yield an easier time, for sure. But gaining a "full understanding of photoshop" is much more difficult and time consuming (due to its size and complexity) than a "full understanding" of the two or three simple rules behind pixQL. (also, pixQL is free and open source :P)

## the future
- Alter syntax to allow for terser `OPERATE` chains. For example, to invert a pixel currently requires `OPERATE SET R = 255-R; OPERATE SET G = 255-G; OPERATE B = 255-B;` which is ridiculous. Should be able to do something like `OPERATE SET R=255-R,G=255-G,B=255-B;`.
- Compatibility with all BMPs, then PNG, JPG, etc...
- More tests for query parsing
- Tests that apply queries to some known image and compares it against some known result (testing severely lacking currently...)
- Types within query (also, allow float intermediate representation for math so divisions, SIN, etc... doesn't get clamped to 0 or 1 before anything useful can be done with it)
- better hex parsing (so it would interpret `COLOR = #FFA920` as setting some color with 255 alpha rather than 0 red, FF green, A9 blue, and 20 alpha...)
- array of TMP targets for intermediate computation within query
- SELECTED as referencable attribute

## bonus query
Either copy this into a file and run with `-qf query_file.pql` or just copy and paste between quotes with `-q "..."`

```
SELECT WHERE 1; OPERATE SET R = R/2; OPERATE SET G = G/2; OPERATE SET B = B/2;
SELECT WHERE row >= HEIGHT/6*0 AND row < HEIGHT/6*1; OPERATE SET R = OUT.R+(255/2);
SELECT WHERE row >= HEIGHT/6*1 AND row < HEIGHT/6*2; OPERATE SET R = OUT.R+(255/2); OPERATE SET G = OUT.G+(255/4);
SELECT WHERE row >= HEIGHT/6*2 AND row < HEIGHT/6*3; OPERATE SET R = OUT.R+(255/2); OPERATE SET G = OUT.G+(255/2);
SELECT WHERE row >= HEIGHT/6*3 AND row < HEIGHT/6*4; OPERATE SET G = OUT.G+(255/2);
SELECT WHERE row >= HEIGHT/6*4 AND row < HEIGHT/6*5; OPERATE SET B = OUT.B+(255/2);
SELECT WHERE row >= HEIGHT/6*5 AND row < HEIGHT/6*6; OPERATE SET R = OUT.R+(255/2); OPERATE SET B = OUT.B+(255/2);
```
