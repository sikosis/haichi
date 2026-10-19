#!/bin/sh

set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/.." && pwd)
screenshot_path="$repository_root/docs/images/haichi.png"

if [ "$(uname -s)" != "Haiku" ]; then
	printf '%s\n' "prepare-release.sh must run on Haiku." >&2
	exit 1
fi

printf '%s\n' "Running build and tests..."
set +e
make -C "$repository_root" check
check_status=$?
set -e
if [ "$check_status" -ne 0 ]; then
	printf '%s\n' "Build or tests failed with status $check_status." >&2
	exit "$check_status"
fi

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
printf '%s\n' "Launching $application_path..."
"$application_path" &

printf '%s\n' "Capturing the active haichi window..."
set +e
screenshot -w -b -d 2 -s -f png "$screenshot_path"
screenshot_status=$?
set -e
if [ "$screenshot_status" -ne 0 ]; then
	printf '%s\n' "Screenshot failed with status $screenshot_status." >&2
	if command -v error >/dev/null 2>&1; then
		error "$screenshot_status" >&2 || true
	fi
	exit "$screenshot_status"
fi

if [ ! -s "$screenshot_path" ]; then
	printf '%s\n' "Screenshot was not created: $screenshot_path" >&2
	exit 1
fi

printf '%s\n' "Release checks passed."
printf '%s\n' "Updated screenshot: $screenshot_path"
printf '%s\n' "Review and commit the generated image with the release changes."
