#!/bin/bash

show() {
	pushd "$1" >> /dev/null
	HASH=$(git rev-parse --short=8 HEAD)
	NAME=$(basename $PWD)
	REPO=$(git config --get remote.origin.url)
	REPO=$(sed -E "s,(^git@github.com:)|(^https?://github.com/)|(.git$)|(/$),,g" <<<"$REPO")
	popd >> /dev/null

	printf '%-24s%-10s%s\n' $NAME $HASH $REPO
}
list() {
	pushd "$1" >> /dev/null
	for D in ./*; do
		show "$D"
	done
	popd >> /dev/null
}
tell() {
	printf '%s\n--------\n' $1
}
bump() {
	printf '\n'
}

{
	tell MINUI
	show ./
	bump
	
	tell TOOLCHAINS
	list ./toolchains
	bump

	tell tg3040
	show ./workspace/tg3040/other/unzip60
	echo CORES
	list ./workspace/tg3040/cores/src
	bump

} | sed 's/\n/ /g'