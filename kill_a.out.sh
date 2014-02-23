kill -9 $(ps -a | grep a.out | awk '{ print $1 }')

