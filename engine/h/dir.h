#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "common.h"
#include "types.h"

/*! @brief get file type of 'name'.
 *
 *  @return 0 on failure and 'engine_err' is set accordingly.
 */
FSLAPI u64 get_file_type(const str *name);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 is_file(const str *name);

/*! @param log = enable/disable logging.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 is_file_exists(const str *name, b8 log);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 is_dir(const str *name);

/*! @param log = enable/disable logging.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 is_dir_exists(const str *name, b8 log);

/*! -- IMPLEMENTATION: engine/platform_<x> --;
 *
 *  @brief make directory 'path' if it doesn't exist.
 *
 *  @remark failure includes "directory already exists".
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 make_dir(const str *path);

/*! -- IMPLEMENTATION: engine/platform_<x> --;
 *
 *  @brief change current working directory.
 */
FSLAPI int change_dir(const str *path);

/*! @param dst = pointer to NULL buffer to store file contents.
 *  @remark 'dst' is allocated file size, + 1 if 'terminate' is TRUE.
 *
 *  @param format = read file 'name' using 'format' ('fopen()' parameter).
 *  @param terminate = TRUE will NULL terminate buffer.
 *
 *  @return file size in bytes.
 *  @return 0 on failure and 'engine_err' is set accordingly.
 */
FSLAPI u64 get_file_contents(const str *name, void **dst, u64 size, b8 terminate);

/*! @brief get directory entries of 'name'.
 *
 *  @return (Buf){0} on failure and 'engine_err' is set accordingly.
 */
FSLAPI Buf get_dir_contents(const str *name);

/*! @brief get directory entry count of 'name'.
 *
 *  @return entry count, 'engine_err' is set accordingly on failure.
 */
FSLAPI u64 get_dir_entry_count(const str *name);

/*! @brief copy 'src' into 'dst'.
 *
 *  @remark can overwrite files.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 copy_file(const str *src, const str *dst);

/*! @brief copy 'src' into 'dst'.
 *
 *  @param contents_only =
 *      TRUE: copy directory contents of 'src' and place inside 'dst'.
 *      FALSE: copy directory 'src' and place inside 'dst'.
 *
 *  @remark can overwrite directories and files, unless 'overwrite' is FALSE.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 copy_dir(const str *src, const str *dst, b8 contents_only);

/*! @param log = enable/disable logging.
 *  @param text = TRUE will newline-terminate file.
 *
 *  @remark write and append functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief like 'write_file()', but without logging on success.
 *
 *  @param log = enable/disable logging.
 *  @param text = TRUE will newline-terminate file.
 *
 *  @remark write and append functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 _write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @param log = enable/disable logging.
 *  @param text = TRUE will newline-terminate file.
 *
 *  @remark write and append functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief like 'append_file()', but without logging on success.
 *
 *  @param log = enable/disable logging.
 *  @param text = TRUE will newline-terminate file.
 *
 *  @remark write and append functions are deliberately separate, to better avoid human error.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 _append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text);

/*! @brief get calloc'd string of resolved 'name'.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
FSLAPI str *get_path_absolute(const str *name);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: engine/platform_<x> --;
 *
 *  @brief get real path.
 * 
 *  @param path = relative path,
 *  @param path_real = result/canonical path, ending with slash.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _get_path_absolute(const str *path, str *path_real);

/*! @brief append '/' onto 'path' if 'path' not ending in '/', NULL terminated.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
FSLAPI void check_slash(str *path);

/*! @brief normalize all slashes to '/' or '\' depending on operating system.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
FSLAPI void normalize_slash(str *path);

/*! @brief change all '\\' to '\'.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
FSLAPI void posix_slash(str *path);

/*! @brief get 'path' retracted to its parent directory.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
FSLAPI str *retract_path(str *path);

/*! @brief get base name of 'path'.
 *
 *  @param size = max size of 'dst' to use.
 *
 *  'engine_err' is set accordingly on failure.
 */
FSLAPI void get_base_name(const str *path, str *dst, u64 size);

#endif /* ENGINE_DIR_H */
