#include "engine/WaveEngine.hpp"

void updateSettleTimers(WaveEngine& engine, bool rmsHigh, bool vppHigh, bool freqHigh, bool globalHigh,
                        float& rmsMS, float& vppMS, float& freqMS, float& globalMS) {
    static bool prevRMS = false;     static int rmsFrameCount = 0;
    static bool prevVpp = false;     static int vppFrameCount = 0;
    static bool prevFreq = false;    static int freqFrameCount = 0;
    static bool prevGlobal = false;  static int globalFrameCount = 0;

    if (!engine.getPauseState()) {
        if (rmsHigh == prevRMS)   rmsFrameCount++;    else { rmsFrameCount = 0;    prevRMS = rmsHigh; }
        if (vppHigh == prevVpp)   vppFrameCount++;    else { vppFrameCount = 0;    prevVpp = vppHigh; }
        if (freqHigh == prevFreq)   freqFrameCount++;   else { freqFrameCount = 0;   prevFreq = freqHigh; }
        if (globalHigh == prevGlobal) globalFrameCount++; else { globalFrameCount = 0; prevGlobal = globalHigh; }
    }

    rmsMS = rmsFrameCount * 16.67f;
    vppMS = vppFrameCount * 16.67f;
    freqMS = freqFrameCount * 16.67f;
    globalMS = globalFrameCount * 16.67f;
}

