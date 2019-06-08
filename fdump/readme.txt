The MIT License (MIT)

Copyright (c) 2013 RW Senser

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

readme.txt:

Provided by Robert Senser in November 2013, based on work done in 2002.

Provided are the dump.c and makehex.c programs in source and Windows compiled form.

This simple programs can be used to dump and create small text files, in hexadecimal form.

They are very useful for looking at data stored with various character encodings.

Translate.java is also provided to translate files between encodings.

The demo.bat file shows these programs in use.

Contents:
Program		Comment
Translate	Java program that does conversions (source and class)
translate	C program using ICU that does conversions (source only)
makehex		C program to easily make small binary files (source and Windows exe)
dump		C program to quickly dump small files in hex/binary (source and Windows exe)

Bat File	Comment
demo.bat	Shows what these can do
showicu.bat     More translate demo
showjava.bat	More Translate demo

Data File	Comment
ebs-420.txt	Some cp-420 data
ebs-1256.txt	Some cp-1256 data

Other File      Comment
license.txt	MIT Software License
