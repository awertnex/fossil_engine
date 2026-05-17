#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_CAP 2048

/*!
 *  @internal
 *
 *  @brief process shader before compilation.
 *
 *  - parse includes recursively.
 *
 *  @return `NULL` on failure.
 */
char *shader_pre_process(const char *path, size_t *file_len);

/*!
 *  @internal
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return `NULL` on failure.
 */
char *shader_pre_process_includes_internal(const char *path, size_t *file_len, size_t recursion_limit);

/*!
 *  @brief get `path` retracted to its parent directory.
 *
 *  @return non-zero on failure.
 */
int retract_path(char *path);

char *shader_pre_process(const char *path, size_t *file_len)
{
    return shader_pre_process_includes_internal(path, file_len, 512);
}

char *shader_pre_process_includes_internal(const char *path, size_t *file_len, size_t recursion_limit)
{
    static char token[][256] =
    {
        "#include \"",
        "\"\n"
    };

    FILE *file = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;
    size_t cursor = 0;
    char *buf = NULL;
    char *buf_include = NULL;
    char *buf_resolved = NULL;
    char *buf_resolved_temp = NULL;
    size_t buf_len = 0;
    size_t buf_include_len = 0;
    size_t buf_resolved_len = 0;
    char temp[PATH_CAP] = {0};
    size_t temp_len = 0;

    if (recursion_limit == 0)
    {
        fprintf(stderr, "Failed to Pre-Process Shader '%s', Include Recursion Limit Exceeded\n", path);
        return NULL;
    }

    /* ---- open, read and close file --------------------------------------- */

    if ((file = fopen(path, "rb")) == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    buf_len = ftell(file);
    fseek(file, 0, SEEK_SET);
    buf = calloc(1, buf_len + 1);
    if (buf == NULL)
        goto cleanup;
    fread(buf, 1, buf_len, file);
    fclose(file);
    buf[buf_len] = 0;

    /* ---- pre-process shader ---------------------------------------------- */

    buf_resolved = calloc(1, buf_len + 1);
    if (buf_resolved == NULL)
        goto cleanup;

    buf_resolved_len = buf_len + 1;
    snprintf(buf_resolved, buf_resolved_len, "%s", buf);
    for (; i < buf_len; ++i)
    {
        if ((i == 0 || (buf[i - 1] == '\n')) &&
                (buf[i] == '#') &&
                !strncmp(buf + i, token[0], strlen(token[0])))
        {
            temp_len = 0;
            for (j = strlen(token[0]); i + j < buf_len &&
                    strncmp(buf + i + j, token[1], strlen(token[1]));
                    ++temp_len, ++j)
            {}
            j += strlen(token[1]);

            snprintf(temp, PATH_CAP, "%s", path);
            retract_path(temp);
            snprintf(temp + strlen(temp), PATH_CAP - strlen(temp),
                    "%.*s", (int)temp_len, buf + i + strlen(token[0]));

            if (strncmp(temp, path, strlen(temp)) == 0)
            {
                fprintf(stderr, "Failed to Pre-Process Shader '%s', Self-Include Detected\n", path);
                goto cleanup;
            }

            buf_include = shader_pre_process_includes_internal(temp, &buf_include_len, recursion_limit - 1);
            if (buf_include == NULL)
                goto cleanup;

            buf_resolved_temp = realloc(buf_resolved, buf_resolved_len + buf_include_len + 1);
            if (buf_resolved_temp == NULL)
                goto cleanup;
            buf_resolved = buf_resolved_temp;
            buf_resolved_temp = NULL;
            buf_resolved_len += buf_include_len;

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%.*s", (int)(i - k), buf + k);
            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%s", buf_include);

            k = i + j;
            if (buf_include != NULL)
            {
                free(buf_include);
                buf_include = NULL;
            }
        }
    }

    if (k < buf_len)
        snprintf(buf_resolved + cursor,
                buf_resolved_len - cursor, "%s", buf + k);

    if (buf != NULL)
        free(buf);
    if (file_len != NULL)
        *file_len = buf_resolved_len;

    return buf_resolved;

cleanup:

    if (file != NULL)
        fclose(file);
    if (buf_include != NULL)
        free(buf_include);
    if (buf_resolved != NULL)
        free(buf_resolved);
    if (buf != NULL)
        free(buf);
    return NULL;
}

int retract_path(char *path)
{
    size_t i = 0;
    size_t len = 0;
    size_t stage = 0;

    if (path == NULL)
        return 1;

    len = strlen(path);
    if (len <= 1)
    {
        fprintf(stderr, "%s\n", "Failed to Retract Path, Size Too Small");
        return 1;
    }

    for (i = 0; i < len; ++i)
    {
        if (stage == 1 &&
                (path[len - i - 1] == '/' || path[len - i - 1] == '\\'))
            break;
        if (path[len - i - 1])
        {
            path[len - i - 1] = 0;
            stage = 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    char *buf = NULL;

    if (argc < 2)
        return 1;

    buf = shader_pre_process(argv[1], NULL);
    printf("%s", buf);
    return 0;
}
