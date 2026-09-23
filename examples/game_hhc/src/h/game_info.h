#ifndef HHC_GAME_INFO_H
#define HHC_GAME_INFO_H

#define GAME_AUTHOR         "Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"

#define GAME_VERSION_STABLE "-stable"
#define GAME_VERSION_BETA   "-beta"
#define GAME_VERSION_ALPHA  "-alpha"
#define GAME_VERSION_DEV    "-dev"

#define GAME_VERSION        "0.5.0"GAME_VERSION_DEV
#define GAME_TITLE          GAME_NAME": "GAME_VERSION

#ifdef HHC_RELEASE_BUILD
#   define GAME_RELEASE_BUILD 1
#else
#   define GAME_RELEASE_BUILD 0
#endif /* HHC_RELEASE_BUILD */

#endif /* HHC_GAME_INFO_H */
