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
mixBuffers(float *out, float *buf1, float *buf2, int len) {
  int i;
  for (i = 0; i < len; i++) {
    // formula for mixing from: http://www.vttoth.com/digimix.htm
    out[i] = (buf1[i] + buf2[i]) - (buf1[i] * buf2[i]);
  }
}


static void
downMix(float *out, float *pcm_left, float *pcm_right, int len) {
  int i;
  for (i = 0; i < len; i++) {
    out[i] = (pcm_left[i] / 2 + pcm_right[i] / 2);
  }
}


static float *
getFloatBuffer(JNIEnv *env, jshortArray buf, jsize len) {
  int i;
  short *shortbuf = (short *)(*env)->GetPrimitiveArrayCritical(env, buf, 0);
  float *floatbuf = calloc(len, sizeof(float));

  for (i = 0; i < len; i++) {
    floatbuf[i] = ((float)(shortbuf[i])/32768.0f);
  }

  (*env)->ReleasePrimitiveArrayCritical(env, buf, shortbuf, 0);

  return floatbuf;
}


static jshort *
getShortBuffer(float *buf, jsize size) {
  int i;
  jshort *out = calloc(size, sizeof(jshort));

  for (i = 0; i < size; i++) {
    out[i] = (short)(buf[i]*32767.0f);
  }

  return out;
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_instantiateAutotalent
  (JNIEnv *env, jclass class, jint sampleRate) {
  if (instance == NULL) {
    instance = instantiateAutotalent(sampleRate);
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "instantiated autotalent at %d with sample rate: %d", sampleRate);
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setConcertA
  (JNIEnv *env, jclass class, jfloat concertA) {
  if (instance != NULL) {
    *(instance->m_pfTune) = (float) concertA;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setKey
  (JNIEnv *env, jclass class, jchar key) {
  if (instance != NULL) {
    setAutotalentKey(instance, (char *) &key);
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setFixedPitch
  (JNIEnv *env, jclass class, jfloat fixed) {
  if (instance != NULL) {
    *(instance->m_pfFixed) = (float) fixed;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setFixedPull
  (JNIEnv *env, jclass class, jfloat pull) {
  if (instance != NULL) {
    *(instance->m_pfPull) = (float) pull;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setCorrectionStrength
  (JNIEnv *env, jclass class, jfloat strength) {
  if (instance != NULL) {
    *(instance->m_pfAmount) = (float) strength;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setCorrectionSmoothness
  (JNIEnv *env, jclass class, jfloat smooth) {
  if (instance != NULL) {
    *(instance->m_pfSmooth) = (float) smooth;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setPitchShift
  (JNIEnv *env, jclass class, jfloat shift) {
  if (instance != NULL) {
    *(instance->m_pfShift) = (float) shift;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setScaleRotate
  (JNIEnv *env, jclass class, jint rotate) {
  if (instance != NULL) {
    *(instance->m_pfScwarp) = (int) rotate;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setLfoDepth
  (JNIEnv *env, jclass class, jfloat depth) {
  if (instance != NULL) {
    *(instance->m_pfLfoamp) = (float) depth;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setLfoRate
  (JNIEnv *env, jclass class, jfloat rate) {
  if (instance != NULL) {
    *(instance->m_pfLforate) = (float) rate;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setLfoShape
  (JNIEnv *env, jclass class, jfloat shape) {
  if (instance != NULL) {
    *(instance->m_pfLfoshape) = (float) shape;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setLfoSymmetric
  (JNIEnv *env, jclass class, jfloat symmetric) {
  if (instance != NULL) {
    *(instance->m_pfLfosymm) = (float) symmetric;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setLfoQuantization
  (JNIEnv *env, jclass class, jint quantization) {
  if (instance != NULL) {
    *(instance->m_pfLfoquant) = (int) quantization;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setFormantCorrection
  (JNIEnv *env, jclass class, jint correction) {
  if (instance != NULL) {
    *(instance->m_pfFcorr) = (int) correction;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setFormantWarp
  (JNIEnv *env, jclass class, jfloat warp) {
  if (instance != NULL) {
    *(instance->m_pfFwarp) = (float) warp;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_setMix
  (JNIEnv *env, jclass class, jfloat mix) {
  if (instance != NULL) {
    *(instance->m_pfMix) = (float) mix;
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_processSamples___3SI
  (JNIEnv *env , jclass class, jshortArray samples, jint numSamples) {
  if (instance != NULL) {
    // copy buffers
    float *sampleBuffer = getFloatBuffer(env, samples, numSamples);
    setAutotalentBuffers(instance, sampleBuffer, sampleBuffer);

    // process samples
    runAutotalent(instance, numSamples);

    // copy results back up to java array
    short *shortBuffer = getShortBuffer(sampleBuffer, numSamples);
    (*env)->SetShortArrayRegion(env, samples, 0, numSamples, shortBuffer);

    free(shortBuffer);
    free(sampleBuffer);
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_processSamples___3S_3S_3SI
  (JNIEnv *env , jclass class, jshortArray samples, jshortArray mixLeft, jshortArray mixRight, jint numSamples) {
  if (instance != NULL) {
    // copy buffers
    float *outbuf = getFloatBuffer(env, samples, numSamples);
    float *instrumental = getFloatBuffer(env, mixLeft, numSamples);
    setAutotalentBuffers(instance, outbuf, outbuf);

    // process samples
    runAutotalent(instance, numSamples);

    // downmix if necessary
    if (mixRight != NULL) {
      float *instrumental_right = getFloatBuffer(env, mixRight, numSamples);
      downMix(instrumental, instrumental, instrumental_right, numSamples);
      free(instrumental_right);
    }

    // mix instrumental samples with tuned recorded samples
    mixBuffers(outbuf, outbuf, instrumental, numSamples);

    // copy results back up to java array
    short *shortbuf = getShortBuffer(outbuf, numSamples);
    (*env)->SetShortArrayRegion(env, samples, 0, numSamples, shortbuf);

    free(shortbuf);
    free(outbuf);
    free(instrumental);
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "No suitable autotalent instance found!");
  }
}


JNIEXPORT void JNICALL Java_net_sourceforge_autotalent_Autotalent_destroyAutotalent
  (JNIEnv *env, jclass class) {
  if (instance != NULL) {
    cleanupAutotalent(instance);
    __android_log_print(ANDROID_LOG_DEBUG, "libautotalent.so", "cleaned up autotalent at %d", instance);
    instance = NULL;
  }
}
