#!/bin/sh
TOOLDIR="`dirname \"$0\"`/../src"
EDIT="$TOOLDIR/edit"

die()
{
	echo "$1"
	exit $2
}

[ $# -eq "1" ] || die "Usage $0 [EEPROM-FILE]." 1
[ -x "$EDIT" ] || die "EEPROM edit utility not found." 2
[ -e "$1" ] || die "File $1 does not exist." 3

"$EDIT" "$1" -l DEAD &> /dev/null
[ "$?" -eq "0" ] || die "EEPROM image has already been converted, or is invalid." 4

"$EDIT" "$1" -r DEAD -e CAFF,h:0x01,0x00,0x0a,0x00,0x00,0x00,0x0a,0x00,0x85,0x09,0x0a,0x01,0x72,0xfe,0x1a,0x00,0x00,0x00 &> /dev/null
[ "$?" -eq "0" ] || die "conversion failed." 5

echo "conversion done."
