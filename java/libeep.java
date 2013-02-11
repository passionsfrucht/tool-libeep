package com.antneuro;

public class libeep {
  static {
    System.loadLibrary("EepJNI");
  }

  public static native void init();
  public static native void exit();

  public static native String get_version();

  public static native int  read(String filename);
  public static native void close(int handle);

  public static native int    get_channel_count(int handle);
  public static native String get_channel_label(int handle, int channel_id);
  public static native String get_channel_unit(int handle, int channel_id);
  public static native String get_channel_reference(int handle, int channel_id);

  public static native float get_sample_frequency(int handle);
  public static native long  get_sample_count(int handle);

  public static native float[] get_samples(int handle, long from, long to);
}
