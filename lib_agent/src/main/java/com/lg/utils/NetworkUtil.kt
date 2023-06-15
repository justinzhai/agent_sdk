package com.lg.utils

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkInfo
import android.telephony.TelephonyManager
import android.util.Log


object NetworkUtil {

    const val NERWORK_TYPE_FAIL = 0
    const val NERWORK_TYPE_2G = 1
    const val NERWORK_TYPE_3G = 2
    const val NERWORK_TYPE_WIFI = 3
    const val NERWORK_TYPE_UNKNOWN = 4
    const val NERWORK_TYPE_CMWAP = 5
    const val NERWORK_TYPE_CMNET = 6
    const val NERWORK_TYPE_UNIWAP = 7
    const val NERWORK_TYPE_UNINET = 8
    const val NERWORK_TYPE_4G = 9
    const val NERWORK_TYPE_ETHERNET = 10


    var macAddress:String? = null

    fun getMacAddress(context: Context): String? {


        macAddress?.let {
            return it
        }
        try {
            val wifi =
                context.getSystemService(Context.WIFI_SERVICE) as android.net.wifi.WifiManager
            val info = wifi.connectionInfo
            macAddress=info.macAddress
            if (macAddress==null)
                macAddress= "00:00:00:00:00:00"
            return macAddress
        } catch (e: Exception) {
            Log.e("NetworkUtil", "getMacAddress: ", e)
        }
        return macAddress
    }

    fun getNetworkState(context: Context): Int? {
//        if (true)
//            return  NERWORK_TYPE_WIFI

        try {

            val connMgr =
                context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
            val activeNetInfo: NetworkInfo? = connMgr.activeNetworkInfo
            if (activeNetInfo == null || !activeNetInfo.isAvailable || !activeNetInfo.isConnectedOrConnecting) {
                return NERWORK_TYPE_FAIL
            }

            // 判断是不是连接的是不是wifi

            // 判断是不是连接的是不是wifi
            if (activeNetInfo.type == ConnectivityManager.TYPE_WIFI) {
                return NERWORK_TYPE_WIFI
            } else if (activeNetInfo.type == ConnectivityManager.TYPE_ETHERNET) {
                // 判断是不是有线网络
                return NERWORK_TYPE_ETHERNET
            } else if (activeNetInfo.type == ConnectivityManager.TYPE_MOBILE) {
                val strSubTypeName = activeNetInfo.subtypeName
                return when (activeNetInfo.subtype) {
                    TelephonyManager.NETWORK_TYPE_GPRS, TelephonyManager.NETWORK_TYPE_CDMA, TelephonyManager.NETWORK_TYPE_EDGE, TelephonyManager.NETWORK_TYPE_1xRTT, TelephonyManager.NETWORK_TYPE_IDEN -> NERWORK_TYPE_2G
                    TelephonyManager.NETWORK_TYPE_EVDO_A, TelephonyManager.NETWORK_TYPE_UMTS, TelephonyManager.NETWORK_TYPE_EVDO_0, TelephonyManager.NETWORK_TYPE_HSDPA, TelephonyManager.NETWORK_TYPE_HSUPA, TelephonyManager.NETWORK_TYPE_HSPA, 14, 12, 15 -> NERWORK_TYPE_3G
                    else ->                         // 中国移动 联通 电信 三种3G制式
                        if (strSubTypeName.equals(
                                "TD-SCDMA",
                                ignoreCase = true
                            ) || strSubTypeName.equals("WCDMA", ignoreCase = true)
                            || strSubTypeName.equals("CDMA2000", ignoreCase = true)
                        ) {
                            NERWORK_TYPE_3G
                        } else {
                            NERWORK_TYPE_4G
                        }
                }
            }

        } catch (e: Exception) {
            Log.e("NetworkUtil", "getNetworkState: ", e)
        }

        return NERWORK_TYPE_FAIL

    }

}