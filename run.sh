# Before launch
ipcs -a >tracker/ipcs.old
ps -a >tracker/ps.old

# Launch
./parking

# After launch
ipcs -a >tracker/ipcs.new
ps -a >tracker/ps.new

# Did we let something here ?
diff tracker/ipcs.new tracker/ipcs.old >tracker/ipcs.diff
if [ $? -eq 0 ]
then
	echo "IPCS match"
else
	echo "IPCS don't match :"
	cat ipcs.diff
fi

diff tracker/ps.new tracker/ps.old >tracker/ps.diff
if [ $? -eq 0 ]
then
	echo "PS match"
else
	echo "PS don't match"
	cat ps.diff
fi
