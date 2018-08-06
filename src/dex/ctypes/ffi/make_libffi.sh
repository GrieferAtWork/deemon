#!/bin/sh

cd $(dirname $(readlink -f "$0"))

while [[ $# -gt 0 ]]; do
	case $1 in
		-*)
			echo "Unrecognized option: '$1'"
			exit 1
			;;
		*)
			echo "Option: $1"
			if [[ "$1" == *"="* ]]; then
				export $1
			else
				export $1=""
			fi
			;;
	esac
	shift
done

mkdir -p $NAME
cd $NAME || exit $?
bash ../libffi/configure --host=$TARGET --target=$TARGET --enable-static --prefix $(pwd)
make
