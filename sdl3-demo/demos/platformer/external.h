#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int ENGINE_API StartAppStandalone();
int ENGINE_API StartAppTooling();

#ifdef __cplusplus
}
#endif
