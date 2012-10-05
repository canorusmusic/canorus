#!/bin/bash

# Creates qhp/qhcp files from template

if [ "$1" = "" ]; then
	echo "Usage: $0 <lang-prefix>"
	echo "For example '$0 en'"
	exit 0
fi

if [ "$1" = "template" ]; then
	echo "Won't overwrite template"
	exit 1
fi

toclevels=( partToc sectionToc subsectionToc subsubsectionToc )
tmpl="template"
lang="$1"
target="$lang"
file=`mktemp`

# Replace line templates
# $1 - replace %$1% in template
# $2 - content to replace with
# $3 - file name
replace() {
	content="`printf '%s' "$2" | sed 's,[/\&],\\\\&,g;$!s/$/\\\\/'`" 
	sed -i "s/%$1%/$content/g" "$3"
}

indent() {
	/bin/echo -ne "\t\t\t"
	if [ ! -z $1 ]; then
		for i in `seq 1 $1`; do
			/bin/echo -ne "\t"
		done
	fi
}

# Recursively create <section> tags.
# $1 - current array id
# $2 - current toc level
doToc() {
	if [ $2 -eq ${#toclevels[@]} ]; then
		return
	fi
	local idx=$1
	while [ $idx -lt ${#parts[@]} ]; do
		local line="$(printf '%s\n' "${parts[$idx]}" | tail -n 1)"
		# check if toclevel is $2
		local toclevel="$(printf '%s\n' "${parts[$idx]}" | head -n 1 | egrep -o 'class="[^"]*"' | egrep -o '".*"' | tr -d '"')"
		if [ "$toclevel" != "${toclevels[$2]}" ]; then
			return
		fi
		idx=$(($idx+1))
		local inner="`doToc $idx $(($2+1))`"
		local title=$(echo $line | egrep -o ">[^<]*</a>" | sed -r 's+(^>|</a>$)++g')
		local anchor=$(echo $line | egrep -o 'href="[^"]*' | sed -r 's|^href="#||')
		if [ -z "$inner" ]; then
			indent $(expr $1)
			echo "<section title=\"$title\" ref=\"$lang.html#$anchor\"/>"
		else
			indent $(expr $1)
			echo "<section title=\"$title\" ref=\"$lang.html#$anchor\">"
			printf '%s\n' "$inner"
			indent $(expr $1)
			echo "</section>"
		fi
	done
}

toc() {
	local first=`grep -n "tableofcontents" $lang.html | cut -d":" -f1`
	if [ -z $first ]; then
		exit 1
	fi
	local last=
	grep -n '</div>' $lang.html > $file  2>/dev/null
	while read ln; do 
		line="`echo $ln | cut -d':' -f1`"
		if [ $line -gt $first ]; then
			last=$line
			break
		fi
	done < $file
	local lines="$(sed -n "$(($first+1))h;$(($first + 2)),$(($last - 1))H;"'${;g;s+<br />+&\n+g;p;}' $lang.html)"

	# split at <br /> lines
	parts=()
	local j=0
	local lastLine=1
	for line in $(printf '%s' "$lines" | grep -n "<br />" | cut -d":" -f1) $((`printf '%s\n' "$lines" | wc -l`+1)); do
		parts[$((j++))]="$(printf '%s' "$lines" | sed -n "$lastLine,$(($line-1))p")"
		lastLine=$(($line + 1))
	done
	printf '%s' "`doToc 0 0`"
}

images() {
	local i
	for i in $lang*x.png; do
		indent
		echo "<file>$i</file>"
	done
}


qhp() {
	cp "$tmpl.qhp" "$target.qhp"

	replace "LANG" "$lang" "$target.qhp"
	replace "TOC" "`toc`" "$target.qhp"
	replace "IMAGES" "`images`" "$target.qhp"
}

qhcp() {
	# Copy title over from existing qhcp, if possible
	local title
	title=
	if [ -e $target.qhcp ]; then
		title=$(grep "<title>" $target.qhcp | head -n 1 | sed 's/.*<title>\(.*\)<\/title>/\1/')
	fi
	if [ -z "$title" -o "$title" = "%TITLE%" ]; then
		title="Canorus Users's guide"
	fi
	
	cp "$tmpl.qhcp" "$target.qhcp"
	replace "LANG" "$lang" "$target.qhcp"
	replace "TITLE" "$title" "$target.qhcp"
}

qhp
qhcp
rm $file

