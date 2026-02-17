#pragma once

#include <tooling/exportedresources.h>
#include <string>

typedef void(__stdcall InitCallback)(RenderInfo);
typedef void(__stdcall EventReceived)(const char *);
