package com.z.s

import android.content.Context
import com.lg.biz.AgentManager



object Agent {

    @JvmOverloads
    fun init(context: Context, callBack: ISLoadCallBack?) {
        callBack.let {
            init(context, "kWMTThfUmdk1kPU1", "LG_001", callBack)
        }
    }

    fun init(context: Context, appKey: String, channel: String, callBack: ISLoadCallBack?) {
        AgentManager.init(context, appKey, channel, callBack)
    }


}