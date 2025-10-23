# supplement 2.30  --  Useful Ruby enhancements

Some simple Ruby extensions.


## Motivation

The original motivation was my conviction that a language that has a
`Numeric#nonzero?` method induces a programming style that as well demands
for a `String#notempty?` method.


## Description

These are methods on standard classes that didn't manage to become part of
the Ruby interpreter. (Although, some did in the meantime.)

They are all very small, most of them have less than 15 lines of code.
So they would not be a weight in the Ruby interpreter but it is very
difficult to convince a majority that they belong there.

The methods won't be useful for everybody but some programmers may like
to make them part of their programming style.

If you like to get taunted, then go to the Ruby mailing list and propose
one of them to be included into the Ruby standard.


## Contents (uncomplete)

  * `String#notempty?`
  * `String#head`
  * `String#tail`
  * `String#rest`
  * `String#starts_with?`/`ends_with?`
  * `Array#notempty?`
  * `Array#first=`/`last=`
  * `Hash#notempty?`
  * `Struct.[]`
  * `Integer.roman`
  * `Date.easter`
  * `TCPServer/UNIXServer.accept` with a code block
  * `File system stats`
  * `Process.renice`
  * Interval timer
  * `LockedFile`


## Copyright

  * (C) 2009-2025 Bertram Scharpf <software@bertram-scharpf.de>
  * License: [BSD-2-Clause+](./LICENSE)
  * Repository: [ruby-supplement](https://github.com/BertramScharpf/ruby-supplement)

