/*
 * Copyright 2019 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_OUTPUT_H
#define _LIBRPMINSPECT_OUTPUT_H

/**
 * @defgroup OUTPUT_FORMATS
 *
 * Output formats are reference by a constant.
 *
 * @{
 */

/**
 * @def FORMAT_TEXT
 *
 * Constant for the text output format (the default).
 */
#define FORMAT_TEXT    0

/**
 * @def FORMAT_JSON
 *
 * Constant for the JSON output format.
 */
#define FORMAT_JSON    1

/**
 * @def FORMAT_XUNIT
 *
 * Constant for the XUnit output format.
 */
#define FORMAT_XUNIT   2

/**
 * @def FORMAT_SUMMARY
 *
 * Constant for the summary output format.
 */
#define FORMAT_SUMMARY 3

/** @} */

#endif
