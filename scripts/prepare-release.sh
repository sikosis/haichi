#!/bin/sh

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
screenshot_path="$repository_root/docs/images/haichi.png"

if [ "$(uname -s)" != "Haiku" ]; then
	printf '%s\n' "prepare-release.sh must run on Haiku." >&2
	exit 1
fi

make -C "$repository_root" check

application_path=""
for candidate in "$repository_root"/objects.*-release/haichi; do
	if [ -x "$candidate" ]; then
		application_path=$candidate
		break
	fi
done

if [ -z "$application_path" ]; then
	printf '%s\n' "Could not find the built haichi application." >&2
	exit 1
fi

mkdir -p "$(dirname -- "$screenshot_path")"
"$application_path" &

screenshot --window --border --delay 2 --silent --format=png \
	"$screenshot_path"

if [ ! -s "$screenshot_path" ]; then
	printf '%s\n' "Screenshot was not created: $screenshot_path" >&2
	exit 1
fi

printf '%s\n' "Release checks passed."
printf '%s\n' "Updated screenshot: $screenshot_path"
printf '%s\n' "Review and commit the generated image with the release changes."
