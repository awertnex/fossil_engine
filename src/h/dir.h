/*!
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*!
 *  @file dir.h
 *
 *  @brief directory and file parsing, writing, copying and path resolution.
 */

#ifndef FSL_DIR_H
#define FSL_DIR_H

#include "common.h"
#include "types.h"

enum fsl_file_type_index
{
    /* zero is reserved for "error" */
    FSL_FILE_TYPE_REG = 1,
    FSL_FILE_TYPE_LNK,
    FSL_FILE_TYPE_DIR,
    FSL_FILE_TYPE_CHR,
    FSL_FILE_TYPE_BLK,
    FSL_FILE_TYPE_FIFO
}; /* fsl_file_type_index */

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_file(const fsl_fs_path *path);

/*!
 *  @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_file_exists(const fsl_fs_path *path, b8 log);

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_dir(const fsl_fs_path *path);

/*!
 *  @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_dir_exists(const fsl_fs_path *path, b8 log);

/*!
 *  @brief make directory `path` if it doesn't exist.
 *
 *  @remark "directory already exists" counts as failure.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_make_dir(const fsl_fs_path *path);

/*!
 *  @brief change current working directory.
 */
FSLAPI int fsl_change_dir(const fsl_fs_path *path);

/*!
 *  @brief get file type at `path` and store in `type`.
 *
 *  @param type pointer to `u32` to store file type,
 *  can be one of the enum values at @ref fsl_file_type_index.
 *
 *  @remark does not follow symlinks, reports symlinks themselves.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_file_type(const fsl_fs_path *path, u32 *type);

/*!
 *  @brief read file at `path` and write its contents into `*dst`.
 *
 *  @param dst pointer to `NULL` buffer to allocate and store file contents.
 *  @param terminate enable/disable null (`\0`) termination.
 *
 *  @remark `dst` is allocated file size, + 1 if `terminate` is `TRUE`.
 *
 *  @return file size, in bytes.
 *  @return 0 on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u64 fsl_get_file_contents(const fsl_fs_path *path, void **dst, b8 terminate);

/*!
 *  @brief get directory entries at `path`.
 *
 *  @return `(fsl_buf){0}` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI fsl_buf fsl_get_dir_contents(const fsl_fs_path *path);

/*!
 *  @brief get directory entry count at 'path'.
 *
 *  @return entry count.
 *  @return 0 on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u64 fsl_get_dir_entry_count(const fsl_fs_path *path);

/*!
 *  @brief copy `src` into `dst`, preserve permissions and modification time.
 *
 *  @remark can overwrite files and symlinks.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_copy_file(const fsl_fs_path *src, const fsl_fs_path *dst);

/*!
 *  @brief copy `src` into `dst`, preserve all permissions and modification times.
 *
 *  @param contents_only
 *      TRUE: copy directory contents of `src` and place inside `dst`.
 *      FALSE: copy directory `src` and place inside `dst`.
 *
 *  @remark can overwrite directories, files and symlinks.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_copy_dir(const fsl_fs_path *src, const fsl_fs_path *dst, b8 contents_only);

/*!
 *  @brief write contents of `buf` into `path` and create new file if it doesn't exist.
 *
 *  @param size size of data, in bytes.
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark can overwrite files and symlinks.
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_write_file(const fsl_fs_path *path, u64 size, void *buf, b8 log, b8 text);

/*!
 *  @brief append contents of `buf` into `path` and create new file if it doesn't exist.
 *
 *  @param size size of data, in bytes.
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark can overwrite files and symlinks.
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_append_file(const fsl_fs_path *path, u64 size, void *buf, b8 log, b8 text);

/*!
 *  @brief get calloc'd string of resolved `path`.
 *
 *  @param path relative path.
 *  @param dst result/canonical `path`, slash (`/`) terminated.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_path_absolute(const fsl_fs_path *path, str **dst);

/*!
 *  @internal
 *
 *  @brief get real path from relative path at `path`.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param path relative path.
 *  @param dst result/canonical `path`, slash (`/`) teminated.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_get_path_absolute_internal(const fsl_fs_path *path, str *dst);

/*!
 *  @brief append @ref FSL_SLASH_NATIVE onto `path` if `path` not ending in
 *  @ref FSL_SLASH_NATIVE, null (`\n`) terminated.
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_check_slash(fsl_fs_path *path);

/*!
 *  @brief normalize all slashes to @ref FSL_SLASH_NATIVE depending on operating system.
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_normalize_slash(fsl_fs_path *path);

/*!
 *  @brief change all backslashes (`\`) in `path` to slashes ('/').
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_posix_slash(fsl_fs_path *path);

/*!
 *  @brief get `path` retracted to its parent directory.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_retract_path(fsl_fs_path *path);

/*!
 *  @brief get base name of `path`.
 *
 *  @param size max size of `dst` to use.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_base_name(const fsl_fs_path *path, str *dst, u64 size);

#endif /* FSL_DIR_H */
