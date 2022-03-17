#!/bin/sh -e

# Enable this to save spaces
# DUMP_OPTS="--unformat"

# Generate template JSON
./dump --template $DUMP_OPTS > .j2s_template.json

# Generate template JSON C header
xxd -iu .j2s_template.json .j2s_template_json.h
{
	echo "#if 0 // Template JSON:"
	cat .j2s_template.json
	echo "#endif"
	echo

	cat .j2s_template_json.h
} > $1
