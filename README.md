# pixQL - BETA
SQL inspired command-line image processing. *Note - very much still a work in progress; everything is subject to change.*

**WARNING** Currently only works with basic BMP files. Does not handle indexed BMPs or any kinds of RLE compression or anything like that. At its current stage, pixQL is working on experimenting with and finalizing its design before I'll do more work with compatibility/robustness.

That said, any bug reports, compatibility gaps, suggestions, etc... are welcome!

**Goal:** Use simple, clean, composable rules to allow for precise pixel-level image operations.

*This document is rather informal, as pixQL is still early in development. Also, I've never written much technical documentation before- any help or suggestions welcome!*

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
pixql -i in.bmp -o out.bmp -q "SELECT WHERE COLOR = #FF000000; OPERATE SET COLOR = #00FF0000;"

# copy red channel into green channel
pixql -i in.bmp -o out.bmp -q "OPERATE SET G = R;"

# add white 1px 100x100 grid
pixql -i in.bmp -o out.bmp -q "SELECT WHERE ROW % 100 = 0 OR COL % 100 = 0; OPERATE SET COLOR = WHITE;"

# create 100x100 image with 10x10 checkerboard pattern
pixql -o out.bmp -q "BLACK(100,100); SELECT WHERE ROW % 20 < 10; OPERATE SET COLOR = WHITE; SELECT WHERE COL % 20 < 10; OPERATE SET R = 255-OUT.R; OPERATE SET G = 255-OUT.G; OPERATE SET B = 255-OUT.B;"
```

## query syntax
While *inspired* by the ideas of other SQL's out there, there are a couple differences.

Every query consists of an optional `init` statement, followed by any number of `procedures`.

### init
This statement sets up the canvas on which you will perform operations. It takes the form of a single word, followed by optionally specifying dimensions (in parenthesis), followed by a semicolon.

##### example init statement
`CLEAR(100,200);` - creates a clear image 100px wide and 200px tall

The options for initializer word are as follows:

- `BLACK` : sets all pixels to black (#000000FF)
- `WHITE` : sets all pixels to white (#FFFFFFFF)
- `CLEAR` : sets all pixels to clear (#00000000)
- `COPY`  : copies input image (if size differs, copies window origin top-left)

If no initializer word, `COPY` is default. If no dimensions, dimensions of input is default. If no input, MUST specify initializer word AND dimensions.

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

## the future
- Alter syntax to allow for terser `OPERATE` chains. For example, to invert a pixel currently requires `OPERATE SET R = 255-R; OPERATE SET G = 255-G; OPERATE B = 255-B;` which is ridiculous. Should be able to do something like `OPERATE SET R=255-R,G=255-G,B=255-B;`.
- Compatibility with all BMPs, then PNG, JPG, etc...
- More tests for query parsing
- Tests that apply queries to some known image and compares it against some known result (testing severely lacking currently...)
- Types within query (also, allow float intermediate representation for math so divisions, SIN, etc... doesn't get clamped to 0 or 1 before anything useful can be done with it)
- better hex parsing (so it would interpret `COLOR = #FFA920` as setting some color with 255 alpha rather than 0 red, FF green, A9 blue, and 20 alpha...)
- array of TMP targets for intermediate computation within query
- SELECTED as referencable attribute
