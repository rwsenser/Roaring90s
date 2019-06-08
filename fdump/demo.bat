echo off
echo ***
echo please read license.txt file
echo ***
echo pass a short character string (with an umlaut) from
echo "8859-1 (nearly ASCII) --> via Java --> UTF-8"
echo "UTF-8  --> via Java --> UTF-16" 
echo "UTF-16 --> via Java --> 8859-1"
echo and then verify equal!
echo  push enter to start (and may God help us...)
pause
echo on
rem *** create a little file with u umlaut in it
makehex base.txt 3132333435eb
dump base.txt
rem *** convert file to utf-8 with Jave
rem *** find encoding names in Java doc under "encodings"
rem *** Java encoding names are case sensitive....
java Translate base.txt ISO8859_1 utf-8.txt UTF8
dump utf-8.txt
rem *** convert utf-8 to UnicodeBig with Java
java Translate utf-8.txt UTF8 utf-16.txt UnicodeBig
dump utf-16.txt
rem *** convert utf-16 back to 8859-1 with Java
Java Translate utf-16.txt UnicodeBig test.txt ISO8859_1
rem *** test to see if matches base.txt
fc base.txt test.txt
pause clean up now (control-c to leave work files...)
erase base.txt
erase utf-8.txt
erase utf-16.txt
erase test.txt  
rem done