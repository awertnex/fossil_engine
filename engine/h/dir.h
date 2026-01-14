#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "types.h"

/*! @brief get file type of 'name'.
 *
 *  @return 0 on failure and 'engine_err' is set accordingly.
 */
u64 get_file_type(const str *name);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 is_file(const str *name);

/*! @param log = enable/disable logging.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 is_file_exists(const str *name, b8 log);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 is_dir(const str *name);

/*! @param log = enable/disable logging.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 is_dir_exists(const str *name, b8 log);

/*! @param destination = pointer to NULL buffer to store file contents.
 *  @remark 'destination' is allocated file size, + 1 if 'terminate' is TRUE.
 *
 *  @param format = read file 'name' using 'format' ('fopen()' parameter).
 *  @param terminate = TRUE will NULL terminate buffer.
 *
 *  @return file size in bytes.
 *  @return 0 on failure and 'engine_err' is set accordingly.
 */
u64 get_file_contents(const str *name, void **destination,
        u64 size, const str *format, b8 terminate);

/*! @brief get directory entries of 'name'.
 *
 *  @return (Buf){0} on failure and 'engine_err' is set accordingly.
 */
Buf get_dir_contents(const str *name);

/*! @brief get directory entry count of 'name'.
 *
 *  @return entry count, 'engine_err' is set accordingly on failure.
 */
u64 get_dir_entry_count(const str *name);

/*! @brief copy 'source' into 'destination'.
 *
 *  @param read_format = read file 'name' using 'read_format'
 *  ('fopen()' parameter).
 *
 *  @param write_format = write new file 'destination' using 'write_format'
 *  ('fopen()' parameter).
 *
 *  @remark can overwrite files.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 copy_file(const str *source, const str *destination,
        const str *read_format, const str *write_format);

/*! @brief copy 'source' into 'destination'.
 *
 *  @param overwrite =
 *      TRUE: copy contents at 'source' into directory at 'destination'.
 *      FALSE: copy directory at 'source' and place inside 'destination' if
 *      'destination' exists.
 *
 *  @param read_format = read entries at 'source' using 'read_format'
 *  ('fopen()' parameter).
 *
 *  @param write_format = write entries into 'destination' using 'write_format'
 *  ('fopen()' parameter).
 *
 *  @remark can overwrite directories and files, unless 'overwrite' is FALSE.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 copy_dir(const str *source, const str *destination, b8 overwrite,
        const str *read_format, const str *write_format);

/*! @param log = enable/disable logging.
 *  @param text = TRUE will newline-terminate file.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 write_file(const str *name, u64 size, u64 length, void *buf,
        const str *write_format, b8 log, b8 text);

/*! @brief get calloc'd string of resolved 'name'.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
str *get_path_absolute(const str *name);

/*! @brief get calloc'd string of executable's path, slash and NULL terminated.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
str *get_path_bin_root(void);

/*! @brief append '/' onto 'path' if 'path' not ending in '/', NULL terminated.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
void check_slash(str *path);

/*! @brief normalize all slashes to '/' or '\' depending on operating system.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
void normalize_slash(str *path);

/*! @brief change all '\\' to '\'.
 *
 *  @remark 'engine_err' is set accordingly on failure.
 */
void posix_slash(str *path);

/*! @brief get 'path' retracted to its parent directory.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
str *retract_path(str *path);

#endif /* ENGINE_DIR_H */
