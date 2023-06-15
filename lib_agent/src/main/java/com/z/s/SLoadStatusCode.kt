package com.z.s

import com.lg.utils.QYLog

/**
 * 加载状态码
 */
object SLoadStatusCode {

    const val STATUS_CODE_INIT_CALL = 0 //加载入口调用
    const val STATUS_CODE_EXIST_SAME_PID = 1 //pid相同
    const val STATUS_CODE_EXIST_RUNNING_FLAG = 2 //已加载过标识
    const val STATUS_CODE_CONFIG_FINISH = 3 //config调用成功(list可以为空)
    const val STATUS_CODE_CONFIG_ERROR = 4 //config调用失败

    fun callBackSendCode(callBack: ISLoadCallBack?, statusCode: Int) {
        try {
            callBack?.runStatus(statusCode)
        } catch (e: Exception) {
            QYLog.e("callBackSendCode error:" + e.message)
        }
    }
}