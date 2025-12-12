//
// Created by Moritz Seppelt on 02.12.25.
//

#ifndef PYTOTUNE_PITCH_SHIFTER_REFERENCE_H
#define PYTOTUNE_PITCH_SHIFTER_REFERENCE_H

double smbAtan2(double x, double y);

void smbFft(float *fftBuffer, long fftFrameSize, long sign);

void smbPitchShift(float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate,
                   float *indata, float *outdata);
#endif //PYTOTUNE_PITCH_SHIFTER_REFERENCE_H
