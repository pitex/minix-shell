rm -f reader.out.*
./rw 3 3 --test | sort > test.A
./rw 3 3
cat /dev/null > test.agg
for l in $(ls reader.out.*); do cat $l >> test.agg; done
sort test.agg > test.B
diff test.A test.B >/dev/null
if [ $? != 0 ] ; then
	echo "INCORRECT"
#	exit 1
else 
	echo "CORRECT"
fi 
echo "Sizes"
for l in $(ls reader.out.*); do 
	cat $l >> test.agg; 
	echo $l $(stat -c%s "$l")
done
rm -f reader.out.* test.A test.B test.agg
