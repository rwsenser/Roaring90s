echo "see License.txt"
dump ebs-1256.txt
translate ebs-1256.txt cp1256 ebs-test.txt cp420 -v
dump ebs-test.txt
translate ebs-420.txt cp420 ebs-test2.txt cp1256 -v
dump ebs-test2.txt
fc ebs-420.txt ebs-test.txt
fc ebs-1256.txt ebs-test2.txt
echo done