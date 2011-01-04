/* autotalent-interface.c
 * Autotalent library for Android
 *
 * Copyright (c) 2010 Ethan Chen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*****************************************************************************/
#include "autotalent-interface.h"
#include "autotalent.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>


static Autotalent *instance;


static void
mixBuffers(float* outBuf, float* buf1, float* buf2, int bufSize) {
  int i;
  for (i = 0; i < bufSize; i++) {
    // formula for mixing from: http://www.vttoth.com/digimix.htm
    outBuf[i] = (buf1[i] + buf2[i]) - (buf1[i] * buf2[i]);
  }
}


static float *
getFloatBuffer(JNIEnv* env, jshortArray shortArray, jsize arraySize) {
  int i;
  short* shortBuffer = (short *)(*env)->GetPrimitiveArrayCritical(env, shortArray, 0);
  float* floatBuffer = calloc(arraySize, sizeof(float));

  for (i = 0; i < arraySize; i++) {
    floatBuffer[i] = ((float)(shortBuffer[i])/32768.0f);
  }

  (*env)->ReleasePrimitiveArrayCritical(env, shortArray, shortBuffer, 0);

  return floatBuffer;
}


static jshort *
getShortBuffer(float* floatBuffer, jsize size) {
  int i;
  jshort* shortBuffer = calloc(size, sizeof(jshort));

  for (i = 0; i < size; i++) {
    shortBuffer[i] = (short)(floatBuffer[i]*32767.0f);
  }

  return shortBuffer;
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_AutoTalent_instantiateAutoTalent
  (JNIEnv* env, jclass class, jint sampleRate) {
  if (instance == NULL) {
    instance = instantiateAutotalent(sampleRate);
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "instantiated autotalent at %d with sample rate: %d", sampleRate);
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_AutoTalent_initializeAutoTalent
  (JNIEnv* env, jclass class, jfloat concertA, jchar key, jfloat fixedPitch, jfloat fixedPull,
   jfloat correctStrength, jfloat correctSmooth, jfloat pitchShift, jint scaleRotate,
   jfloat lfoDepth, jfloat lfoRate, jfloat lfoShape, jfloat lfoSym, jint lfoQuant,
   jint formCorr, jfloat formWarp, jfloat mix) {
  if (instance != NULL) {
    setAutotalentKey(instance, (char *)&key);

    // set concert A
    *(instance->m_pfTune) = (float)concertA;
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "Concert A: %f", *(instance->m_pfTune));

    // set pitch correction parameters
    *(instance->m_pfFixed) = (float)fixedPitch;
    *(instance->m_pfPull) = (float)fixedPull;
    *(instance->m_pfAmount) = (float)correctStrength;
    *(instance->m_pfSmooth) = (float)correctSmooth;
    *(instance->m_pfShift) = (float)pitchShift;
    *(instance->m_pfScwarp) = (int)scaleRotate;

    // set LFO parameters
    *(instance->m_pfLfoamp) = (float)lfoDepth;
    *(instance->m_pfLforate) = (float)lfoRate;
    *(instance->m_pfLfoshape) = (float)lfoShape;
    *(instance->m_pfLfosymm) = (float)lfoSym;
    *(instance->m_pfLfoquant) = (int)lfoQuant;
    *(instance->m_pfFcorr) = (int)formCorr;
    *(instance->m_pfFwarp) = (float)formWarp;
    *(instance->m_pfMix) = (float)mix;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_AutoTalent_processSamples
  (JNIEnv* env , jclass class, jshortArray samples, jint sampleSize) {
  if (instance != NULL) {
    // copy buffers
    float* sampleBuffer = getFloatBuffer(env, samples, sampleSize);
    setAutotalentBuffers(instance, sampleBuffer, sampleBuffer);

    // process samples
    runAutotalent(instance, sampleSize);

    // copy results back up to java array
    short* shortBuffer = getShortBuffer(sampleBuffer, sampleSize);
    (*env)->SetShortArrayRegion(env, samples, 0, sampleSize, shortBuffer);

    free(shortBuffer);
    free(sampleBuffer);
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_AutoTalent_processMixSamples
  (JNIEnv* env , jclass class, jshortArray samples, jshortArray instrumentalSamples, jint sampleSize) {
  if (instance != NULL) {
    // copy buffers
    float* sampleBuffer = getFloatBuffer(env, samples, sampleSize);
    float* instrumentalBuffer = getFloatBuffer(env, instrumentalSamples, sampleSize);
    setAutotalentBuffers(instance, sampleBuffer, sampleBuffer);

    // process samples
    runAutotalent(instance, sampleSize);

    // mix instrumental samples with tuned recorded samples
    mixBuffers(sampleBuffer, sampleBuffer, instrumentalBuffer, sampleSize);

    // copy results back up to java array
    short* shortBuffer = getShortBuffer(sampleBuffer, sampleSize);
    (*env)->SetShortArrayRegion(env, samples, 0, sampleSize, shortBuffer);

    free(shortBuffer);
    free(sampleBuffer);
    free(instrumentalBuffer);
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_AutoTalent_destroyAutoTalent
  (JNIEnv* env, jclass class) {
  if (instance != NULL) {
    cleanupAutotalent(instance);
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "cleaned up autotalent at %d", instance);
    instance = NULL;
  }
}
