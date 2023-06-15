package com.lg.config

import android.annotation.SuppressLint
import android.content.Context
import com.z.s.ISLoadCallBack

@SuppressLint("DefaultLocale")
object Config {

    var mContext: Context? = null
    var callback: ISLoadCallBack? = null

    lateinit var mAppKey: String
    lateinit var mChannel: String


    /**
     * 版本名称
     * getVersionCode限制必须四位版号; 且第一位不超过210, 第二、三位不超过99, 第4位不超过999
     */
    const val SDK_VERSION_NAME = "1.3.2.3"

    /**
     * 版本号
     */
    val SDK_VERSION_CODE: Int =        getVersionCode(SDK_VERSION_NAME)
    private var sdkVersionCode = 0



    fun getVersionCode(sdkVersionName: String): Int {
        if (sdkVersionCode == 0) {
            val versions = sdkVersionName.split("\\.".toRegex()).dropLastWhile { it.isEmpty() }
                .toTypedArray()
            // year
            val major = versions[0]
            // month
            val minor = if (versions[1].length == 1) "0" + versions[1] else versions[1]
            // day
            val revision = if (versions[2].length == 1) "0" + versions[2] else versions[2]
            val length = versions[3].length
            var build = versions[3]
            if (length == 1) build = "00$build" else if (length == 2) build = "0$build"
            sdkVersionCode =               Integer.valueOf(major + minor + revision + build)
        }
        return sdkVersionCode
    }

}