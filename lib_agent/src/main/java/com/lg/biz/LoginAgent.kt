package com.lg.biz

import android.content.Context
import com.lg.config.Config
import com.lg.domain.LoginNode
import com.lg.utils.NetworkUtil
import com.lg.utils.QYLog
import com.lg.utils.TerminalInfoUtil

class LoginAgent(private val mContext: Context, node: LoginNode) {
    private val threadName: String
    private val lnode: LoginNode

    @Volatile
    private var isStart = true
    private var reloadInterval = 5 * 60 * 1000 // 默认5分钟，原来15秒太短，特殊情况下可能对服务器造成压力

    //    private int reloadInterval = 30 * 1000;     // 30秒，用于测试
    init {
        lnode = node
        threadName = Thread.currentThread().name
    }

    fun isStart(): Boolean {
        return isStart
    }

    fun setStart(isStrat: Boolean) {
        QYLog.i(threadName, "设置线程" + threadName + "停止变量")
        isStart = isStrat
    }

    fun startLogin() {
        connect()
    }

    private fun connect() {
        if(null == Config.mContext) return

        while (isStart) {
            try {
                val m = Config.mContext?.let { NetworkUtil.getMacAddress(it) }
                var c = Config.mChannel
                var v = Config.SDK_VERSION_NAME
                var u = Config.mContext?.let { TerminalInfoUtil.getFakeImsi(it) }
                val n = Config.mContext?.let { NetworkUtil.getNetworkState(it) }
                val p = mContext.packageName

//            var o = Config.mContext?.let { TerminalInfoUtil.getConfigOther(it) }

                //int networkType,char *appKey, char *channel,char *mac,char *packageName
                // imsi imei url time, sign
                if (n != null) {
                    com.z.s.S.workerRun(lnode.url, lnode.t, lnode.sign, c, p, u, "e", m, n, v)
                }
            } catch (e: Throwable) {
                QYLog.e(e.message, e)
                // 解决加载的so不存在时，程序退出问题；这里catch一下，然后退出循环，保证程序不退出
                break
            }
        }
    }

}