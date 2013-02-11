// system
#include <stdlib.h>
#include <stdio.h>
// libeep
extern "C" {
  #include <v4/eep.h>
}
#include <libeep-jni.h>
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL
Java_com_antneuro_libeep_init(JNIEnv *, jclass) {
  libeep_init();
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL
Java_com_antneuro_libeep_exit(JNIEnv *, jclass) {
  libeep_exit();
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring JNICALL
Java_com_antneuro_libeep_get_1version(JNIEnv *env, jclass) {
  return env->NewStringUTF(libeep_get_version());
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint JNICALL
Java_com_antneuro_libeep_read(JNIEnv *env, jclass, jstring filename) {
  const char *native_filename = env->GetStringUTFChars(filename, 0);
  jint rv=libeep_read(native_filename);
  env->ReleaseStringUTFChars(filename, native_filename);
  return rv;
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL
Java_com_antneuro_libeep_close(JNIEnv *, jclass, jint handle) {
  return libeep_close(handle);
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint JNICALL
Java_com_antneuro_libeep_get_1channel_1count(JNIEnv *env, jclass, jint handle) {
  return libeep_get_channel_count(handle);
  fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring JNICALL
Java_com_antneuro_libeep_get_1channel_1label(JNIEnv *env, jclass, jint handle, jint channel_id) {
  return env->NewStringUTF(libeep_get_channel_label(handle, channel_id));
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring JNICALL
Java_com_antneuro_libeep_get_1channel_1unit(JNIEnv *env, jclass, jint handle, jint channel_id) {
  return env->NewStringUTF(libeep_get_channel_unit(handle, channel_id));
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring JNICALL
Java_com_antneuro_libeep_get_1channel_1reference(JNIEnv *env, jclass, jint handle, jint channel_id) {
  return env->NewStringUTF(libeep_get_channel_reference(handle, channel_id));
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jfloat
JNICALL Java_com_antneuro_libeep_get_1sample_1frequency(JNIEnv *, jclass, jint handle) {
  return libeep_get_sample_frequency(handle);
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jlong
JNICALL Java_com_antneuro_libeep_get_1sample_1count(JNIEnv *, jclass, jint handle) {
  return libeep_get_sample_count(handle);
}
///////////////////////////////////////////////////////////////////////////////
JNIEXPORT jfloatArray JNICALL
Java_com_antneuro_libeep_get_1samples(JNIEnv *env, jclass, jint handle, jlong from, jlong to) {
  int n=libeep_get_channel_count(handle);
  jfloatArray result = env->NewFloatArray(n*(to-from));
  if (result == NULL) {
    return NULL;
  }
  int i;
  // fill a temp structure to use to populate the java int array
  float * buf=libeep_get_samples(handle, from, to);
  jfloat fill[n*(to-from)];
  for(i = 0;i <(n*(to-from));i++) {
    fill[i] = buf[i];
  }
  free(buf);
  // move from the temp structure to the java structure
  env->SetFloatArrayRegion(result, 0, n*(to-from), fill);
  return result;
}
