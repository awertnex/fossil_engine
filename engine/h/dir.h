#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "common.h"
#include "types.h"

/*! @brief get file type of `name`.
 *
 *  @return 0 on failure and @ref engine_err is set accordingly.
 */
FSLAPI u64 get_file_type(const str *name);

/*! @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 is_file(const str *name);

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 is_file_exists(const str *name, b8 log);

/*! @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 is_dir(const str *name);

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 is_dir_exists(const str *name, b8 log);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief make directory `path` if it doesn't exist.
 *
 *  @remark failure includes "directory already exists".
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 make_dir(const str *path);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief change current working directory.
 */
FSLAPI int change_dir(const str *path);

/*! @param dst pointer to `NULL` buffer to store file contents.
 *  @remark `dst` is allocated file size, + 1 if `terminate` is `TRUE`.
 *
 *  @param format read file `name` using `format` (@ref fopen() parameter).
 *  @param terminate enable/disable null (`\0`) termination.
 *
 *  @return file size in bytes.
 *  @return 0 on failure and @ref engine_err is set accordingly.
 */
FSLAPI u64 get_file_contents(const str *name, void **dst, u64 size, b8 terminate);

/*! @brief get directory entries at `name`.
 *
 *  @return `(Buf){0}` on failure and @ref engine_err is set accordingly.
 */
FSLAPI Buf get_dir_contents(const str *name);

/*! @brief get directory entry count of 'name'.
 *
 *  @return entry count.
 *  @return 0 on failure and @ref engine_err is set accordingly.
 */
FSLAPI u64 get_dir_entry_count(const str *name);

/*! @brief copy `src` into `dst`.
 *
 *  @remark can overwrite files.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 copy_file(const str *src, const str *dst);

/*! @brief copy `src` into `dst`.
 *
 *  @param contents_only
 *      TRUE: copy directory contents of `src` and place inside `dst`.
 *      FALSE: copy directory `src` and place inside `dst`.
 *
 *  @remark can overwrite directories and files.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 copy_dir(const str *src, const str *dst, b8 contents_only);

/*! @brief overwrite contents of file at `name` with contents of `buf`, and create
 *  new file if it doesn't exist.
 *  
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief append contents of `buf` to file at `name`.
 *
 *  @param log enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief like @ref append_file(), but without logging on success (used for logger file writes).
 *
 *  @param log = enable/disable logging.
 *  @param text enable/disable newline (`\n`) termination of file.
 *
 *  @remark 'write' and 'append' functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 _append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief get calloc'd string of resolved `name`.
 *
 *  @return `NULL` on failure and @ref engine_err is set accordingly.
 */
FSLAPI str *get_path_absolute(const str *name);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief get real path.
 * 
 *  @param path relative path.
 *  @param path_real result/canonical `path`, ending with slash (`/`).
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
u32 _get_path_absolute(const str *path, str *path_real);

/*! @brief append @ref SLASH_NATIVE onto `path` if `path` not ending in @ref SLASH_NATIVE, null (`\n`) terminated.
 *
 *  @remark @ref engine_err is set accordingly on failure.
 */
FSLAPI void check_slash(str *path);

/*! @brief normalize all slashes to @ref SLASH_NATIVE depending on operating system.
 *
 *  @remark @ref engine_err is set accordingly on failure.
 */
FSLAPI void normalize_slash(str *path);

/*! @brief change all backslashes (`\`) to slashes ('\').
 *
 *  @remark @ref engine_err is set accordingly on failure.
 */
FSLAPI void posix_slash(str *path);

/*! @brief get `path` retracted to its parent directory.
 *
 *  @return `NULL` on failure and @ref engine_err is set accordingly.
 */
FSLAPI str *retract_path(str *path);

/*! @brief get base name of `path`.
 *
 *  @param size max size of `dst` to use.
 *
 *  @remark @ref engine_err is set accordingly on failure.
 */
FSLAPI void get_base_name(const str *path, str *dst, u64 size);

#endif /* ENGINE_DIR_H */
