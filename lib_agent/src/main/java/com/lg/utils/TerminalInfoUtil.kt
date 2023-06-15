package com.lg.utils

import android.content.Context
import android.os.Environment
import android.text.TextUtils
import java.io.BufferedReader
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.FileReader
import java.util.Properties
import java.util.UUID

object TerminalInfoUtil {
    private var uuid: String? = null




    /**
     * 自定义的uuid
     *
     * @param context
     * @return
     */
    @Synchronized
    fun getFakeImsi(context: Context): String? {
        if (!TextUtils.isEmpty(uuid)) {
            return uuid
        }
        val key = "uuid"
        uuid = SharedPreferencesUtils.getString(context, key)
        //TODO 这里涉及到外部存储权限，不同进程的IMSI是否是同一个
        if (TextUtils.isEmpty(uuid)) {
            uuid = "uuid_" + UUID.randomUUID().toString().replace("-", "").substring(0, 16)
            SharedPreferencesUtils.putString(context, key, uuid)
        }
        return uuid
    }

    fun getConfigOther(mContext: Context): String {
        val hsman= android.os.Build.MANUFACTURER
        val hstype = android.os.Build.MODEL
        val osSdk = "android_" + android.os.Build.VERSION.RELEASE
        val displayMetrics =   mContext.resources.displayMetrics
        val width = displayMetrics.widthPixels
        val height = displayMetrics.heightPixels
        val packageName = mContext.packageName





        var json ="""
            {
                "hm":"$hsman",
                "ht":"$hstype",
                "o":"$osSdk",
                "sw":$width,
                "sh":$height,
                "p":"$packageName"
            }
        """.trimIndent()
        return json

    }

    /**
     * 获取数据统计文件根目录
     *
     * @return
     */
    val configFilePath: File?
        get() {
            var configDir: File? = null
            if (Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED) {
                configDir = File(Environment.getExternalStorageDirectory().absolutePath, ".Systemp")
                if (!configDir.exists()) {
                    configDir.mkdirs()
                }
            }
            return configDir
        }


}