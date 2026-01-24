#ifndef FSL_DIR_H
#define FSL_DIR_H

#include "common.h"
#include "types.h"

/*! @brief get file type of `name`.
 *
 *  @return 0 on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u64 fsl_get_file_type(const str *name);

/*! @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_file(const str *name);

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_file_exists(const str *name, b8 log);

/*! @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_dir(const str *name);

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_is_dir_exists(const str *name, b8 log);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief make directory `path` if it doesn't exist.
 *
 *  @remark failure includes "directory already exists".
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_make_dir(const str *path);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief change current working directory.
 */
FSLAPI int fsl_change_dir(const str *path);

/*! @param dst pointer to `NULL` buffer to store file contents.
 *  @remark `dst` is allocated file size, + 1 if `terminate` is `TRUE`.
 *
 *  @param format read file `name` using `format` (@ref fopen() parameter).
 *  @param terminate enable/disable null (`\0`) termination.
 *
 *  @return file size in bytes.
 *  @return 0 on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u64 fsl_get_file_contents(const str *name, void **dst, u64 size, b8 terminate);

/*! @brief get directory entries at `name`.
 *
 *  @return `(fsl_buf){0}` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI fsl_buf fsl_get_dir_contents(const str *name);

/*! @brief get directory entry count of 'name'.
 *
 *  @return entry count.
 *  @return 0 on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u64 fsl_get_dir_entry_count(const str *name);

/*! @brief copy `src` into `dst`.
 *
 *  @remark can overwrite files.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_copy_file(const str *src, const str *dst);

/*! @brief copy `src` into `dst`.
 *
 *  @param contents_only
 *      TRUE: copy directory contents of `src` and place inside `dst`.
 *      FALSE: copy directory `src` and place inside `dst`.
 *
 *  @remark can overwrite directories and files.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_copy_dir(const str *src, const str *dst, b8 contents_only);

/*! @brief overwrite contents of file at `name` with contents of `buf`, and create
 *  new file if it doesn't exist.
 *  
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief append contents of `buf` to file at `name`.
 *
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief like @ref fsl_append_file(), but without logging on success (used for logger file writes).
 *
 *  @param log = enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief get calloc'd string of resolved `name`.
 *
 *  @param name relative path.
 *  @param dst result/canonical path at `name`, ending with slash (`/`).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_path_absolute(const str *name, str **dst);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief get real path.
 * 
 *  @param name relative path.
 *  @param dst result/canonical `path`, ending with slash (`/`).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 _fsl_get_path_absolute(const str *name, str *dst);

/*! @brief append @ref FSL_SLASH_NATIVE onto `path` if `path` not ending in
 *  @ref FSL_SLASH_NATIVE, null (`\n`) terminated.
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_check_slash(str *path);

/*! @brief normalize all slashes to @ref FSL_SLASH_NATIVE depending on operating system.
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_normalize_slash(str *path);

/*! @brief change all backslashes (`\`) to slashes ('\').
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 */
FSLAPI void fsl_posix_slash(str *path);

/*! @brief get `path` retracted to its parent directory.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_retract_path(str *path);

/*! @brief get base name of `path`.
 *
 *  @param size max size of `dst` to use.
 *
 *  @remark @ref fsl_err is set accordingly on failure.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_base_name(const str *path, str *dst, u64 size);

#endif /* FSL_DIR_H */
