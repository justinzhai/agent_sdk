package com.z.s;

public class S {

    // Used to load the 'vps_sdk' library on application startup.
    static {
        System.loadLibrary("agent");
    }

    public static native int init();

    public static native int workerRun(String url, String t, String sign, String channel, String pkg, String u, String e, String mac, int net, String ver);

    public static native int release();
}