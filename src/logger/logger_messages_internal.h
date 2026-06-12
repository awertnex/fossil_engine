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
 *  @file logger_messages_internal.h
 *
 *  @brief engine's standardized logger messages and log message conventions.
 */

/*!
 *  LOG MESSAGE CONVENTIONS:
 *      - action:   string, infinitive verb (e.g., "Open File", not "Opened File").
 *      - subject:  string, subject name (e.g., "door.png", not "Texture").
 *      - reason:   string, can be anything (e.g., "Size Too Small" or "`function()` Failed").
 *
 *      encapsulation:
 *          - immutable data (e.g., names, paths): single quotes `''`.
 *            (e.g., "File 'door.png' Not Found").
 *          - mutable/runtime-generated data (e.g., memory addresses, asset IDs):
 *            square brackets `[]` (e.g., Texture[39] Generated).
 *          - symbol names: back-ticks "``" (e.g., "`function().param` Empty").
 *
 *      general rules:
 *      - when passing function names, include the parentheses `()` (e.g., "fsl_shader_free()").
 *      - when passing symbol names, pass them as you'd write them in code, starting
 *        with function name (e.g., "fsl_shader_free().shader->source").
 *      - use square brackets `[]` with no space when the value belongs to the
 *        identifier (e.g., "Memory Cleared fsl_shader_free().shader->source[0x557a01dd1f50][3181B]").
 *      - when using square brackets and the unit of data is unknown, include an
 *        identifier within the brackets, examples:
 *          - "Memory Cleared [3181B]": it is clear that we've cleared 3181 Bytes.
 *          - "Memory Arena Unloaded main()._arena[0x7f12b73e6000][4521611264B][entry_total: 10][entry_cap: 45056B]":
 *            it is not immediately clear what the last two fields represent, hence
 *            the identifiers inside the brackets.
 */

#ifndef FSL_LOGGER_MESSAGE_H
#define FSL_LOGGER_MESSAGE_H

#include <stdio.h>
#include <inttypes.h>

/* ---- section: base ------------------------------------------------------- */

#define MSG_ACTION_FATAL(action)                            fsl_logger_stringf("Failed to %s, Process Aborted\n", action)
#define MSG_ACTION_REASON_FATAL(action, reason)             fsl_logger_stringf("Failed to %s, %s, Process Aborted\n", action, reason)
#define MSG_ACTION_ERROR(action)                            fsl_logger_stringf("Failed to %s\n", action)
#define MSG_ACTION_REASON_ERROR(action, reason)             fsl_logger_stringf("Failed to %s, %s\n", action, reason)
#define MSG_ACTION_SUBJECT_ERROR(action, subject)           fsl_logger_stringf("Failed to %s '%s'\n", action, subject)
#define MSG_ACTION_SUBJECT_REASON_ERROR(action, subject, reason) fsl_logger_stringf("Failed to %s '%s', %s\n", action, subject, reason)

/* ---- section: internal --------------------------------------------------- */

#define MSG_POINTER_NULL_ACTION(action)                     MSG_ACTION_REASON_ERROR(action, "Pointer `NULL`")

/* ---- section: file_io ---------------------------------------------------- */

#define MSG_FILE_NOT_FOUND(name)                            fsl_logger_stringf("File '%s' Not Found\n", name)
#define MSG_DIR_NOT_FOUND(name)                             fsl_logger_stringf("Directory '%s' Not Found\n", name)
#define MSG_DIR_NOT_FOUND_ACTION(action, name)              fsl_logger_stringf("Failed to %s, Directory '%s' Not Found\n", action, name)
#define MSG_IS_NOT_FILE(name)                               fsl_logger_stringf("'%s' is Not a File\n", name)
#define MSG_IS_NOT_DIR(name)                                fsl_logger_stringf("'%s' is Not a Directory\n", name)
#define MSG_FILE_OPEN_FAIL(name)                            MSG_ACTION_SUBJECT_ERROR("Open File", name)
#define MSG_FILE_COPY_FAIL(name_in, name_out)               fsl_logger_stringf("Failed to Copy File '%s' -> '%s', `fopen()` Failed\n", name_in, name_out)
#define MSG_FILE_COPY(name_in, name_out)                    fsl_logger_stringf("File Copied '%s' -> '%s'\n", name_in, name_out)
#define MSG_FILE_WRITE_FAIL(name)                           MSG_ACTION_SUBJECT_ERROR("Write File", name)
#define MSG_FILE_WRITE(name)                                fsl_logger_stringf("File Written '%s'\n", name)
#define MSG_FILE_APPEND_FAIL(name)                          MSG_ACTION_SUBJECT_ERROR("Append File", name)
#define MSG_FILE_APPEND(name)                               fsl_logger_stringf("File Appended '%s'\n", name)
#define MSG_FILE_PERMISSION_COPY_FAIL(name_in, name_out)    fsl_logger_stringf("Failed to Copy File Permissions '%s' -> '%s', `fsl_stat()` Failed\n", name_in, name_out)
#define MSG_FILE_SYMLINK_COPY_FAIL(name_in, name_out)       fsl_logger_stringf("Failed to Copy Symlink '%s' -> '%s'\n", name_in, name_out)
#define MSG_FILE_SYMLINK_COPY(name_in, name_out)            fsl_logger_stringf("Symlink Copied '%s' -> '%s'\n", name_in, name_out)
#define MSG_DIR_CREATE(name)                                fsl_logger_stringf("Directory Created '%s'\n", name)
#define MSG_DIR_CREATE_FAIL(name)                           MSG_ACTION_SUBJECT_ERROR("Create Directory", name)
#define MSG_DIR_CHANGE(name)                                fsl_logger_stringf("Working Directory Changed to '%s'\n", name)
#define MSG_DIR_COPY(name_in, name_out)                     fsl_logger_stringf("Directory Copied '%s' -> '%s'\n", name_in, name_out)
#define MSG_DIR_PERMISSION_COPY_FAIL(name_in, name_out)     fsl_logger_stringf("Failed to Copy Directory Permissions '%s' -> '%s', `fsl_stat()` Failed\n", name_in, name_out)
#define MSG_PATH_TOO_LONG_FATAL(action, name)               fsl_logger_stringf("Failed to %s, Path Too Long '%s', Process Aborted\n", action, name)
#define MSG_GET_BASE_NAME_FAIL(name, reason)                fsl_logger_stringf("Failed to Get Base Name '%s', %s\n", name, reason)

/* ---- section: memory ----------------------------------------------------- */

#define MSG_MEM_ALLOC_POINTER_NULL_FAIL(name)               fsl_logger_stringf("Failed to Allocate Memory %s[%p], Pointer `NULL`\n", name, NULL)
#define MSG_MEM_ALLOC(name, address, size)                  fsl_logger_stringf("Memory Allocated %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_REALLOC_FAIL(name, address)                 fsl_logger_stringf("Failed to Reallocate Memory %s[%p]\n", name, address)
#define MSG_MEM_REALLOC_POINTER_NULL_FAIL(name)             fsl_logger_stringf("Failed to Reallocate Memory %s[%p], Pointer `NULL`\n", name, NULL)
#define MSG_MEM_REALLOC(name, address_old, address_new, size) fsl_logger_stringf("Memory Reallocated %s[%p -> %p][%"PRIu64"B]\n", name, address_old, address_new, size)
#define MSG_MEM_MAP_REASON_FAIL(name, address, size, reason) fsl_logger_stringf("Failed to Map Memory %s[%p][%"PRIu64"B], %s\n", name, address, size, reason)
#define MSG_MEM_MAP(name, address, size)                    fsl_logger_stringf("Memory Mapped %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_COMMIT_REASON_FAIL(name, address_base, address_committed, size, reason) fsl_logger_stringf("Failed to Commit Memory %s[base: %p][commit: %p][%"PRIu64"B], %s\n", name, address_base, address_committed, size, reason)
#define MSG_MEM_COMMIT(name, address_base, address_committed, size) fsl_logger_stringf("Memory Committed %s[base: %p][commit: %p][%"PRIu64"B]\n", name, address_base, address_committed, size)
#define MSG_MEM_REMAP_REASON_FAIL(name, address, size, reason) fsl_logger_stringf("Failed to Remap Memory %s[%p][%"PRIu64"B], %s\n", name, address, size, reason)
#define MSG_MEM_REMAP(name, address_old, address_new, size_old, size_new) fsl_logger_stringf("Memory Remapped %s[%p -> %p][%"PRIu64"B -> %"PRIu64"B]\n", name, address_old, address_new, size_old, size_new)
#define MSG_MEM_UNMAP(name, address, size)                  fsl_logger_stringf("Memory Unmapped %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_FREE(name, address, size)                   fsl_logger_stringf("Memory Unloaded %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_CLEAR(name, address, size)                  fsl_logger_stringf("Memory Cleared %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_ARENA_INIT_POINTER_NULL_FAIL(name, size)    fsl_logger_stringf("Failed to Initialize Memory Arena %s[%p][%"PRIu64"], Pointer `NULL`\n", name, NULL, size)
#define MSG_MEM_ARENA_INIT_REASON_FAIL(name, address, size, reason) fsl_logger_stringf("Failed to Initialize Memory Arena %s[%p][%"PRIu64"], %s\n", name, address, size, reason)
#define MSG_MEM_ARENA_INIT_FAIL(name, address, size)        fsl_logger_stringf("Failed to Initialize Memory Arena %s[%p][%"PRIu64"], `fsl_mem_map_internal()` Failed\n", name, address, size)
#define MSG_MEM_ARENA_INIT(name, address, size)             fsl_logger_stringf("Memory Arena Initialized %s[%p][%"PRIu64"B]\n", name, address, size)
#define MSG_MEM_ARENA_PUSH_REASON_FAIL(name, address, size, reason) fsl_logger_stringf("Failed to Push to Memory Arena %s[%p][%"PRIu64"B], %s\n", name, address, size, reason)
#define MSG_MEM_ARENA_PUSH(name, address, offset_pushed, size_pushed, entry, size_entry) fsl_logger_stringf("Memory Arena Pushed %s[%p][offset_pushed: %"PRIu64"][size_pushed: %"PRIu64"B][entry_total: %"PRIu64"][entry_cap: %"PRIu64"B]\n", name, address, offset_pushed, size_pushed, entry, size_entry)
#define MSG_MEM_ARENA_POP_REASON_FAIL(name, address, reason) fsl_logger_stringf("Failed to Pop from Memory Arena %s[%p], %s\n", name, address, reason)
#define MSG_MEM_ARENA_POP(name, address, offset_popped, size_popped, entry, size_entry) fsl_logger_stringf("Memory Arena Popped %s[%p][offset_popped: %"PRIu64"][size_popped: %"PRIu64"B][entry_total: %"PRIu64"][entry_cap: %"PRIu64"B]\n", name, address, offset_popped, size_popped, entry, size_entry)
#define MSG_MEM_ARENA_FREE(name, address, size_arena, entry, size_entry) fsl_logger_stringf("Memory Arena Unmapped %s[%p][%"PRIu64"B], Entry Total [%"PRIu64"][%"PRIu64"B]\n", name, address, size_arena, entry, size_entry)

/* ---- section: process ---------------------------------------------------- */

#define MSG_EXEC_SIGTERM(name_cmd, signal)                  fsl_logger_stringf("CMD '%s' Terminated by Signal[%d]\n", name_cmd, signal)
#define MSG_EXEC_ABNORMAL_EXIT(name_cmd)                    fsl_logger_stringf("CMD '%s' Exited Abnormally\n", name_cmd)
#define MSG_EXEC(name_cmd, exit_code)                       fsl_logger_stringf("'%s' Exit[%d]\n", name_cmd, exit_code)

/* ---- section: setup_and_limits ------------------------------------------- */

#define MSG_RATE_LIMIT_EXCEED_ACTION(action)                MSG_ACTION_REASON_ERROR(action, "Rate Limit Exceeded")
#define MSG_INCLUDE_RECURSION_LIMIT_EXCEED_ACTION_SUBJECT(action, subject) MSG_ACTION_SUBJECT_REASON_ERROR(action, subject, "Include Recursion Limit Exceeded")
#define MSG_SELF_INCLUDE_DETECT_ACTION_SUBJECT(action, subject) MSG_ACTION_SUBJECT_REASON_ERROR(action, subject, "Self-Include Detected")
#define MSG_WINDOW_NOT_FOUND_WARNING                        "no Window Found for the Currently Bound Render\n"
#define MSG_GL_VERSION_NOT_SUPPORT(major, minor)            fsl_logger_stringf("Failed to Initialize OpenGL Context, OpenGL 4.3+ Required, Current Version '%"PRId32".%"PRId32"', Process Aborted\n", major, minor)

/* ---- section: data_and_assets -------------------------------------------- */

#define MSG_VBO_INIT(id, size)                              fsl_logger_stringf("VBO[%u][%"PRIu64"B] Initialized\n", id, size)
#define MSG_VBO_FREE(id, size)                              fsl_logger_stringf("VBO[%u][%"PRIu64"B] Unloaded\n", id, size)
#define MSG_FBO_INIT_FAIL(id, status)                       fsl_logger_stringf("Failed to Initialize FBO[%u], Status[%d]\n", id, status)
#define MSG_FBO_INIT(id, size_x, size_y)                    fsl_logger_stringf("FBO[%u][size_x: %"PRIu64"][size_y: %"PRIu64"] Initialized\n", id, size_x, size_y)
#define MSG_FBO_REALLOC(id, size_x, size_y)                 fsl_logger_stringf("FBO[%u][size_x: %"PRIu64"][size_y: %"PRIu64"] Reallocated\n", id, size_x, size_y)
#define MSG_FBO_FREE(id)                                    fsl_logger_stringf("FBO[%u] Unloaded\n", id)
#define MSG_TEXTURE_LOAD_REASON_FAIL(name, reason)          MSG_ACTION_SUBJECT_REASON_ERROR("Load Texture", name, reason)
#define MSG_TEXTURE_LOAD(name, id)                          fsl_logger_stringf("Texture %s[%u] Loaded\n", name, id)
#define MSG_TEXTURE_UNLOAD(name, id)                        fsl_logger_stringf("Texture %s[%u] Unloaded\n", name, id)
#define MSG_TEXTURE_HANDLE_CREATE(handle_id, texture_name, texture_id) fsl_logger_stringf("Handle[%"PRIx64"] for Texture %s[%u] Created\n", handle_id, texture_name, texture_id)
#define MSG_TEXTURE_HANDLE_DESTROY(handle_id, texture_name, texture_id) fsl_logger_stringf("Handle[%"PRIx64"] for Texture %s[%u] Destroyed\n", handle_id, texture_name, texture_id)
#define MSG_MESH_INIT(name)                                 fsl_logger_stringf("Mesh '%s' Loaded\n", name)
#define MSG_MESH_UNLOAD(name)                               fsl_logger_stringf("Mesh '%s' Unloaded\n", name)
#define MSG_MESH_EXPORT_FMESH(path, size)                   fsl_logger_stringf("Mesh File '%s' [%"PRIu64"B] Expoted\n", path, size)
#define MSG_SHADER_INIT(name, id)                           fsl_logger_stringf("Shader %s[%u] Loaded\n", name, id)
#define MSG_SHADER_UNLOAD(name, id)                         fsl_logger_stringf("Shader %s[%u] Unloaded\n", name, id)
#define MSG_SHADER_PROGRAM_LOAD(name, id)                   fsl_logger_stringf("Shader Program %s[%u] Loaded\n", name, id)
#define MSG_SHADER_PROGRAM_UNLOAD(name, id)                 fsl_logger_stringf("Shader Program %s[%u] Unloaded\n", name, id)
#define MSG_FONT_LOAD(name)                                 fsl_logger_stringf("Font '%s' Loaded\n", name)
#define MSG_FONT_UNLOAD(name)                               fsl_logger_stringf("Font '%s' Unloaded\n", name)
#define MSG_UPDATE_RENDER_SETTINGS_FAIL                     "Something Went Wrong While Updating Render Settings\n"

/* ---- section: input ------------------------------------------------------ */

#define MSG_INPUT_KEY_BIND_INIT(key, shift, ctrl, alt, super) fsl_logger_stringf("Key Bind [%s%s%s%s%s] Initialized\n", key, shift, ctrl, alt, super)

/* ---- section: ui --------------------------------------------------------- */

#define MSG_UI_ELEMENT_BAKE(pos_x, pos_y, size_x, size_y)   fsl_logger_stringf("Element Baked [pos.x: %.1f, pos.y: %.1f][size.x: %.1f, size.y: %.1f]\n", pos_x, pos_y, size_x, size_y)

#endif /* FSL_LOGGER_MESSAGE_H */
