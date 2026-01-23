#ifndef BUILD_DIAGNOSTICS_H
#define BUILD_DIAGNOSTICS_H

#define ERR_SUCCESS                     0
#define ERR_POINTER_NULL                4096
#define ERR_POINTER_NOT_NULL            4097
#define ERR_BUFFER_EMPTY                4098
#define ERR_BUFFER_FULL                 4099
#define ERR_BUFFER_OVERFLOW             4100
#define ERR_FILE_NOT_FOUND              4101
#define ERR_DIR_NOT_FOUND               4102
#define ERR_IS_NOT_FILE                 4103
#define ERR_IS_NOT_DIR                  4104
#define ERR_FILE_OPEN_FAIL              4105
#define ERR_DIR_OPEN_FAIL               4106
#define ERR_FILE_EXISTS                 4107
#define ERR_DIR_EXISTS                  4108
#define ERR_SOURCE_NOT_CHANGE           4109
#define ERR_PATH_TOO_LONG               4110
#define ERR_STRING_TOO_LONG             4111
#define ERR_PROCESS_FORK_FAIL           4112
#define ERR_EXEC_FAIL                   4113
#define ERR_EXEC_PROCESS_NON_ZERO       4114
#define ERR_EXEC_TERMINATE_BY_SIGNAL    4115
#define ERR_EXEC_ABNORMAL_EXIT          4116
#define ERR_EXECVP_FAIL                 4117
#define ERR_WAITPID_FAIL                4118
#define ERR_MEM_ALLOC_FAIL              4119
#define ERR_MEM_REALLOC_FAIL            4120
#define ERR_MEM_MAP_FAIL                4121
#define ERR_GET_PATH_ABSOLUTE_FAIL      4122
#define ERR_GET_PATH_BIN_ROOT_FAIL      4123
#define ERR_MEM_COMMIT_FAIL             4124
#define ERR_LOGGER_INIT_FAIL            4125
#define ERR_GLFW                        4126
#define ERR_GLFW_INIT_FAIL              4127
#define ERR_WINDOW_INIT_FAIL            4128
#define ERR_GLAD_INIT_FAIL              4129
#define ERR_GL_VERSION_NOT_SUPPORT      4130
#define ERR_SHADER_COMPILE_FAIL         4131
#define ERR_SHADER_PROGRAM_LINK_FAIL    4132
#define ERR_FBO_INIT_FAIL               4133
#define ERR_FBO_REALLOC_FAIL            4134
#define ERR_IMAGE_LOAD_FAIL             4135
#define ERR_IMAGE_SIZE_TOO_SMALL        4136
#define ERR_FONT_INIT_FAIL              4137
#define ERR_TEXT_INIT_FAIL              4138
#define ERR_MESH_GENERATION_FAIL        4139
#define ERR_INCLUDE_RECURSION_LIMIT     4140
#define ERR_SELF_INCLUDE                4141
#define ERR_DLL_LOAD_FAIL               4142
#define ERR_SIZE_TOO_SMALL              4143
#define ERR_DIR_CREATE_FAIL             4144
#define ERR_WINDOW_NOT_FOUND            4145
#define ERR_UI_INIT_FAIL                4146
#define ERR_SHADER_TYPE_NULL            4147
#define ERR_SCREENSHOT_FAIL             4148
#define ERR_BUILD_FUNCTION_NOT_FOUND    4149
#define ERR_MEM_ARENA_MAP_FAIL          4150
#define ERR_SIZE_LIMIT                  4151
#define ERR_MEM_REMAP_FAIL              4152

/*! @brief global variable for build-specific error codes.
 *
 *  @remark must be declared by the user if not including @ref build.h.
 */
extern u32 build_err;

#endif /* BUILD_DIAGNOSTICS_H */
