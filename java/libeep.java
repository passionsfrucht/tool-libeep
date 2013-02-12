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

  public static native long   get_zero_offset(int handle);     // averages only
  public static native String get_condition_label(int handle); // averages only
  public static native String get_condition_color(int handle); // averages only
  public static native long   get_trials_total(int handle);    // averages only
  public static native long   get_trials_averaged(int handle); // averages only
}
