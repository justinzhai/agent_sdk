package com.lg.domain


class ConfigResp() {
    /**
     * 返回执行状态 0为成功，其它为失败
     */
    var result = 0

    /**
     * result对应的描述
     */
    var message: String? = null

    /**
     * 配置信息
     */
    var loginAddrList: ArrayList<LoginNode>? =  ArrayList()

//
//    fun getLoginAddrList(): ArrayList<LoginNode>? {
//        return loginAddrList
//    }
//
//    fun setLoginAddrList(loginAddrList: ArrayList<LoginNode>?) {
//        this.loginAddrList = loginAddrList
//    }


}