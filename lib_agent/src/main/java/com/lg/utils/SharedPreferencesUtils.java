package com.lg.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class SharedPreferencesUtils {

    private static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(SharedPreferencesConfig.NAME, Context.MODE_PRIVATE);
    }

    public static void putInt(Context context, String key, int value) {
        Editor editor = getSharedPreferences(context).edit();
        editor.putInt(key, value).commit();
    }

    public static void putString(Context context, String key, String value) {
        Editor editor = getSharedPreferences(context).edit();
        editor.putString(key, value).commit();
    }

    public static int getInt(Context context, String key) {
        return getSharedPreferences(context).getInt(key, 0);
    }

    public static String getString(Context context, String key) {
        return getSharedPreferences(context).getString(key, "");
    }

}