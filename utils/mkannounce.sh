#!/bin/sh
#
# Create a text file suitable for a GitHub release announcement or
# blog post.  Groups git log summaries by category and only includes
# those log entries that have a category marking.
#
# Copyright © 2021 David Cantrell <david.l.cantrell@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

PATH=/bin:/usr/bin
TMPDIR="$(mktemp -d)"
CWD="$(pwd)"

trap 'rm -rf "${TMPDIR}"' EXIT

# need the correct tags
if [ "$1" = "--stable" ]; then
    PREV_TAG="$(git tag -l | tail -n 2 | head -n 1)"
    LATEST_TAG="$(git tag -l | tail -n 1)"
else
    PREV_TAG="$(git tag -l | tail -n 1)"
    LATEST_TAG="HEAD"
fi

if [ -z "${PREV_TAG}" ]; then
    echo "*** unable to find the previous git tag" >&2
    exit 1
fi

if [ -z "${LATEST_TAG}" ]; then
    echo "*** unable to find the latest git tag" >&2
    exit 1
fi

# gather log entries since the latest tag in reverse order and only
# those with category markers
git log --reverse --pretty=format:%s "${PREV_TAG}".."${LATEST_TAG}" 2>/dev/null | grep -E "^\[" | while read -r logline ; do
    category="$(echo "${logline}" | cut -d ']' -f 1 | cut -d '[' -f 2)"
    [ -z "${category}" ] && continue
    echo "*$(echo "${logline}" | cut -d ']' -f 2 | xargs -0)" >> "${TMPDIR}"/"${category}"
done

# now create a draft announcement grouped by category
for category in "${TMPDIR}"/* ; do
    if [ -f "${CWD}"/doc/git.md ]; then
        desc="$(grep "\[$(basename "${category}")\]" "${CWD}"/doc/git.md | cut -d '|' -f 3 | awk '{$1=$1};1')"
    fi

    if [ -z "${desc}" ]; then
        echo "$(basename "${category}"):"
    else
        echo "${desc}:"
    fi

    cat "${category}"
    echo
done
