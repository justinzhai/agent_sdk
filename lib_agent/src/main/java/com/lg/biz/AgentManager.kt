package com.lg.biz

import android.content.Context
import android.os.Process
import android.text.TextUtils
import com.z.s.ISLoadCallBack
import com.z.s.SLoadStatusCode
import com.lg.config.Config
import com.lg.utils.CrashHandler
import com.lg.utils.FileUtils
import com.lg.utils.QYLog
import com.z.s.S

object AgentManager {
    private var inited:Boolean=false


    /**
     * 初始化
     */
    @Synchronized
    fun init(context: Context, appKey: String, channel: String, callBack: ISLoadCallBack?) {
        let { Config.mContext=context }



        QYLog.e("AgentManager init start, SDK version" + Config.SDK_VERSION_NAME)
        //注意，在初始化成功之前，不能调用QYLog.e
        callBack?.runStatus(SLoadStatusCode.STATUS_CODE_INIT_CALL)
        if (inited) {
            QYLog.e("AgentManager init end, SDK version" + Config.SDK_VERSION_NAME+" inited:"+inited)
            callBack?.runStatus(SLoadStatusCode.STATUS_CODE_EXIST_RUNNING_FLAG)

            return
        }
        inited=true



        //从本地文件中读取PID，判断是否与本进程的PID一致，如果不一致，说明是新进程，说明不能启动SDK


        val myPid: String? = FileUtils.readPidFile(context)
        if (!TextUtils.isEmpty(myPid) && myPid == Process.myPid().toString()) {
            SLoadStatusCode.callBackSendCode(callBack, SLoadStatusCode.STATUS_CODE_EXIST_SAME_PID)
            return
        }
        FileUtils.writePidFile(context, Process.myPid().toString())


        CrashHandler.getInstance().init(context)

        Config.callback= callBack
        Config.mAppKey=appKey
        Config.mChannel=channel

        // libagent 初始化
        try {
            S.init()
        } catch (e: Throwable) {
            QYLog.e(e.message, e)
            // 解决加载的so不存在时，程序退出问题；这里catch一下，保证程序不退出
        }

        VpsAgent.start()

        QYLog.i("AgentManager init end")
    }



}