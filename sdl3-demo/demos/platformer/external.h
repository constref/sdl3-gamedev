#pragma once

#include <engineapi.h>
#include <stdint.h>

class EngineWorker;

int StartAppStandalone();
void StartAppTooling(int editorPID, int logW, int logH, int width, int height);
