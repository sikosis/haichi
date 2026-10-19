#!/bin/sh

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
source_icon="$repository_root/resources/haichi.svg"
output_icon="$repository_root/resources/haichi.hvif"

if ! command -v icon2icon >/dev/null 2>&1; then
	printf '%s\n' "icon2icon is required; on Haiku run: pkgman install hvif_tools" >&2
	exit 1
fi

icon2icon "$source_icon" "$output_icon"
header=$(od -An -N4 -tx1 "$output_icon" | tr -d ' \n')
if [ "$header" != "6e636966" ]; then
	printf '%s\n' "icon2icon did not produce a valid HVIF header." >&2
	exit 1
fi

printf '%s\n' "Updated $output_icon"
