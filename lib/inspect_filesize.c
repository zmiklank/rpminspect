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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

static bool filesize_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    off_t change;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Files without a peer have to be ignored */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debug and build paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Only run this check on regular files */
    if (!S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
        return true;
    }

    /* Nothing to do if the sizes are the same */
    if (file->st.st_size == file->peer_file->st.st_size) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_FILESIZE;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_OK;

    /* Size checks and messaging */
    if (file->st.st_size > 0 && file->peer_file->st.st_size == 0) {
        /* became non-empty */
        xasprintf(&params.msg, _("%s became a non-empty file on %s"), file->localpath, arch);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("non-empty ${FILE} on ${ARCH}");
        params.remedy = REMEDY_FILESIZE_BECAME_NOT_EMPTY;
        result = false;
    } else if (file->st.st_size == 0 && file->peer_file->st.st_size > 0) {
        /* became empty */
        xasprintf(&params.msg, _("%s became an empty file on %s"), file->localpath, arch);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("empty ${FILE} on ${ARCH}");
        params.remedy = REMEDY_FILESIZE_BECAME_EMPTY;
        result = false;
    } else {
        change = ((file->st.st_size - file->peer_file->st.st_size) * 100 / file->peer_file->st.st_size);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;

        if (ri->size_threshold > 0 && (llabs(change) >= ri->size_threshold)) {
            /* change is at or above our reporting change threshold for VERIFY */
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_CHANGED;
            result = false;
        }

        if (change > 0) {
            /* file grew */
            xasprintf(&params.msg, _("%s grew by %lld%% on %s"), file->localpath, llabs(change), arch);
            params.noun = _("${FILE} size grew on ${ARCH}");
            params.remedy = REMEDY_FILESIZE_GREW;
        } else if (change < 0) {
            /* file shrank */
            xasprintf(&params.msg, _("%s shrank by %lld%% on %s"), file->localpath, llabs(change), arch);
            params.noun = _("${FILE} size shrank on ${ARCH}");
            params.remedy = REMEDY_FILESIZE_SHRANK;
        }
    }

    /* info reporting if user configured that */
    if (ri->size_threshold == -1) {
        params.severity = RESULT_INFO;
        params.verb = VERB_OK;
        params.remedy = NULL;
        result = true;
    }

    /* Reporting */
    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
        reported = true;
    }

    return result;
}

/*
 * Main driver for the 'filesize' inspection.
 */
bool inspect_filesize(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* run the size inspection across all RPM files */
    result = foreach_peer_file(ri, NAME_FILESIZE, filesize_driver);

    /* if everything was fine, just say so */
    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_FILESIZE;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
