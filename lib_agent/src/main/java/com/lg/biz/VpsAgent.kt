package com.lg.biz

import com.z.s.SLoadStatusCode
import com.lg.config.Config
import com.lg.domain.LoginNode
import com.lg.utils.ConfigUtil
import com.lg.utils.QYLog
import java.lang.ref.SoftReference

class VpsAgent {
    companion object {
        /**
         * 开始执行所有的业务流程
         * 1. 启动新的协程，向config服务器发送config请求
         *
         */
        fun start() {
            QYLog.i("===========开始调用工作的代码===============sdkversion====" + Config.SDK_VERSION_NAME)
            ConfigRequestThread ().start()
        }

        val vpsThreadMap = mutableMapOf<String, SoftReference<VpsThread>>()
        var configRequestThreadStop = false

        const val SLEEP_TYPE_SUCCESS_CONFIG_RESPONSE=0
        const val SLEEP_TYPE_NULL_CONFIG_RESPONSE=1
        const val SLEEP_TYPE_FAIL_CONFIG_RESPONSE=2

    }


    private class ConfigRequestThread(
    ) :
        Thread() {


        override fun run() {
            try {
                while (!configRequestThreadStop)
                    _run()

            } catch (e: Exception) {
                QYLog.e(e.message, e)
            }
        }

        private fun _run() {

            var sleepType :Int?= 0

            try {
                var vpsHostList: List<LoginNode>? =null
                var configResp = ConfigUtil.configRequest()
                QYLog.i("_run: configResp = $configResp")
                configResp.let { it ->
                    if (it != null) {
                        Config.callback.let { it?.runStatus(SLoadStatusCode.STATUS_CODE_CONFIG_FINISH) }
                        if (it.loginAddrList!=null&&it.loginAddrList!!.isNotEmpty()) {
                            vpsHostList = it.loginAddrList
                            sleepType = SLEEP_TYPE_SUCCESS_CONFIG_RESPONSE
                        }
                        else {
                            //如果拿到的configResp中没有loginAddrList，则认为是失败的configResp
                            sleepType = SLEEP_TYPE_NULL_CONFIG_RESPONSE
                        }
                    }
                    else
                        sleepType= SLEEP_TYPE_FAIL_CONFIG_RESPONSE
                }
                QYLog.i("_run: vpsHostList = $vpsHostList")

                if (vpsHostList != null && vpsHostList!!.isNotEmpty()) {

                    //如果返回的vpsHostList中没有找到已经在执行的线程，则停止该现场，并予以删除
                    for (threadName in vpsThreadMap.keys) {
                        var isExist=false
                        for (node in vpsHostList!!){
                            if (threadName.equals("VpsThread_"+node.url)) {
                                isExist=true
                                break;
                            }
                        }
                        if (!isExist) {
                            val vpsThread = vpsThreadMap.get(threadName)?.get()
                            vpsThread?.stopRun()
                            vpsThreadMap.remove(threadName)
                        }
                    }


                    for (node in vpsHostList!!) {
                        val threadName="VpsThread_"+node.url
                        if ( !vpsThreadMap.containsKey(threadName) ) {
                            val vpsThread = VpsThread(node)
                            vpsThread.name = "VpsThread_"+node.url
                            vpsThread.start()
                            vpsThreadMap.put(threadName, SoftReference(vpsThread))
                        }
                    }
                }
            }catch (e:Exception) {
                QYLog.e( e)
            }
            finally {
                when (sleepType) {
                    //如果config请求成功，则休眠12小时
                    SLEEP_TYPE_SUCCESS_CONFIG_RESPONSE -> {
                        Thread.sleep(1000*60*60*12)
                    }
                    //如果config请求返回的loginAgentList数据为空，则休眠24小时
                    SLEEP_TYPE_NULL_CONFIG_RESPONSE -> {
                        Thread.sleep(1000*60*60*24)
                    }
                    //如果config请求失败，则休眠5分钟
                    SLEEP_TYPE_FAIL_CONFIG_RESPONSE -> {
                        Thread.sleep(1000*60*5)
                    }
                    else
                    -> {
                        Thread.sleep(1000*60*5)
                    }
                }
            }
        }

        fun stopConfigRequestThread() {
            configRequestThreadStop = true
        }
    }




    class VpsThread(node: LoginNode) : Thread() {
        private val node: LoginNode
        private var loginAgent: LoginAgent? = null

        init {
            this.node = node
        }

        override fun run() {
            QYLog.i("===========开始调用工作的代码===============sdkversion====" + Config.SDK_VERSION_NAME)
            loginAgent = Config.mContext?.let { LoginAgent(it, node) }
            loginAgent!!.startLogin()
        }

        fun stopRun() {
            loginAgent?.setStart(false)
        }

        val loginNode: LoginNode
            get() = node
    }

}