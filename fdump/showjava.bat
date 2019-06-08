echo "see license.txt"
dump ebs-1256.txt
java Translate ebs-1256.txt Cp1256 ebs-test.txt Cp420 -v
dump ebs-test.txt
java Translate ebs-420.txt Cp420 ebs-test2.txt Cp1256 -v
dump ebs-test2.txt
fc ebs-420.txt ebs-test.txt
fc ebs-1256.txt ebs-test2.txt
echo done