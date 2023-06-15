package com.lg.utils

import android.text.TextUtils
import com.lg.config.Config
import com.lg.domain.ConfigResp
import com.lg.domain.LoginNode
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.net.HttpURLConnection
import java.net.Proxy
import java.net.URL
import javax.net.ssl.HttpsURLConnection


object ConfigUtil {


    private fun getHost(url: String): String? {
        try {
            val _url = URL(url)
            return _url.host
        } catch(e: Exception) {
            e.printStackTrace()
        }
        return null
    }


    fun configRequest():ConfigResp? {
        val macAddress = Config.mContext?.let { NetworkUtil.getMacAddress(it) }
        var configResp: ConfigResp? = null
        val url = EncryptUtils.getVpsConfigUrl(macAddress.hashCode())
        try {
            var configRequestURL = URL(url )

            val host = getHost(url)

            val connection = configRequestURL.openConnection(Proxy.NO_PROXY) as HttpsURLConnection
            connection.requestMethod = "POST"
            connection.connectTimeout = 10000
            connection.readTimeout = 10000
            connection.useCaches = false
            connection.doInput = true
            connection.doOutput = false
            connection.instanceFollowRedirects = true
            connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded")

            connection.connect()

            var content = genConfigRequestBody(host)
            QYLog.i(content)
            val outStream = connection.outputStream
            outStream.write(content.toByteArray())
            outStream.flush()
            outStream.close()
            val ins = connection.inputStream

            if (connection.responseCode == 200) {
                val outStream = ByteArrayOutputStream()
                val buffer = ByteArray(1024)
                var len = 0
                while (ins.read(buffer).also { len = it } != -1) {
                    outStream.write(buffer, 0, len)
                }
                ins.close()
                val json = String(outStream.toByteArray())
                if (!TextUtils.isEmpty(json)) {
                    QYLog.d("configResp json: $json")
                    configResp = jsonToBean(json)
                    QYLog.d("configResp: $configResp")
                } else {
                    QYLog.w("服务端返回数据为空")
                }
            } else {
                QYLog.w("服务端返回状态码不是200")
            }

        }catch (e: Exception) {
            QYLog.e(e)
        }

        return configResp
    }

    fun jsonToBean(jonsStr: String?): ConfigResp? {

        return try {
            val json = JSONObject(jonsStr)
            val resultCode = json.getString("r")
            QYLog.d("result:$resultCode")
            val loginAddrList: ArrayList<LoginNode> = ArrayList<LoginNode>()
            if (json.has("ll")) {
                val jsonArray = json.getJSONArray("ll")
                for (i in 0 until jsonArray.length()) {
                    val jnode = jsonArray.optJSONObject(i)
                    val lnode = LoginNode()
                    lnode.apply {
                        url = jnode.getString("la")
                        curl = jnode.getString("la")
                        sign = jnode.getString("ls")
                        t = jnode.getString("lt")
                    }
                    loginAddrList.add(lnode)
                }
            }
            var configResp = ConfigResp()
            configResp.apply {
                result = Integer.valueOf(json.getString("r"))
                message = json.getString("m")
            }
            configResp.loginAddrList = loginAddrList
            configResp
        } catch (e: java.lang.Exception) {
            QYLog.e("请求返回的json转换成javabean出错")
            e.printStackTrace()
            null
        }
    }

    fun genConfigRequestBody(host :String? ):String{


        val m= Config.mContext?.let { NetworkUtil.getMacAddress(it) }
        var c= Config.mChannel
        var t= System.currentTimeMillis()/1000
        var v= Config.SDK_VERSION_NAME
        var u = Config.mContext?.let { TerminalInfoUtil.getFakeImsi(it) }
        val n = Config.mContext?.let { NetworkUtil.getNetworkState(it) }
//        val n = NetworkUtil.NERWORK_TYPE_WIFI
        //16位随机，大小写字母和数字
        var k= Config.mAppKey
        var h= host
        var s=MD5Util.getMD5(m + c + t + n + v + k + h)
        var o = Config.mContext?.let { TerminalInfoUtil.getConfigOther(it) }

        var body = "m=$m&c=$c&t=$t&v=$v&u=$u&n=$n&h=$h&s=$s&o=$o"

        QYLog.i("genConfigRequestBody body:"+body)
        return body
    }

}