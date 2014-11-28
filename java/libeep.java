package com.antneuro;

public class libeep {
  static {
    System.loadLibrary("EepJNI");
  }

  public static native void init();
  public static native void exit();

  public static native String get_version();

  // channel related
  public static native int create_channel_info();
  public static native int add_channel(int handle, String label, String ref, String unit);

  // recording info related
  public static native int  create_recording_info();
  public static native void set_start_time(int handle, double epoch, double fraction);

  public static native int  writeCnt(String filename, int rate, int channel_info_handle, int recording_info_handle);
  public static native int  read(String filename);
  public static native void close(int handle);

  public static native int    get_channel_count(int handle);
  public static native String get_channel_label(int handle, int channel_id);
  public static native String get_channel_unit(int handle, int channel_id);
  public static native String get_channel_reference(int handle, int channel_id);

  public static native float get_sample_frequency(int handle);
  public static native long  get_sample_count(int handle);

  public static native void    add_samples(int handle, float[] data, int n);
  public static native float[] get_samples(int handle, long from, long to);

  // for averages only
  public static native long   get_zero_offset(int handle);
  public static native String get_condition_label(int handle);
  public static native String get_condition_color(int handle);
  public static native long   get_trials_total(int handle);
  public static native long   get_trials_averaged(int handle);
}
