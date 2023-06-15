package com.lg.utils

import android.util.Log
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import java.io.PrintWriter
import java.io.StringWriter
import java.io.Writer
import java.util.Properties


/**
 * 文件名称: Log.java<br></br>
 * 作者: hbx <br></br>
 * 创建时间：2014-5-13 15:08:14<br></br>
 * 模块名称: <br></br>
 * 功能说明: <br></br>
 */
object QYLog {
    const val OpenSns = "QYLOG"


    private var p: Properties? = null

    // 是否打印日志
    var isOpenLog = false

    // 内外网判断
    var isDebugMode = false


    init {
        try {
            if (p == null) {
                val myFile: File? = FileUtils.getDebugFile()
                if (myFile != null && myFile.exists()) {
                    p = Properties()
                    var fis: FileInputStream? = null
                    try {
                        fis = FileInputStream(myFile)
                        p!!.load(fis)
                    } catch (e: Exception) {
                        e(e)
                    } finally {
                        if (fis != null) {
                            try {
                                fis.close()
                            } catch (e: IOException) {
                                e(e)
                            }
                        }
                    }
                }
            }
            initDebugMode()
            initLog()
        } catch (e: Throwable) {
            e(e)
        }
    }

    // 如果有该属性且为1，设为内网，否则为现网
    private fun initDebugMode() {
        if (p != null) {
            val b: String = p!!.getProperty("a")
            if ("1" == b) {
                isDebugMode = true
            }
        }
    }

    /**
     * b=1，则打开log信息
     */
    private fun initLog() {
        if (p != null) {
            val b: String = p!!.getProperty("b")
            if ("1" == b) {
                isOpenLog = true
            }
        }
    }
    /**
     * 输出错误信息
     */
    fun e(TAG: String?, msg: String?, e: Throwable?) {
        // 根据配置来判断是否打印日志
        if (!isOpenLog) return
        Log.w(TAG, msg, e)
    }

    /**
     * 调试信息
     */
    private fun e(TAG: String?, msg: String?) {
        if (!isOpenLog) return
        Log.w(TAG, msg!!)
    }

    /**
     * 调试信息
     */
    fun e(msg: String?) {
        if (!isOpenLog) return
        Log.e(OpenSns, msg!!)
    }

    /**
     * 调试信息
     */
    fun w(msg: String?) {
        if (!isOpenLog) return
        Log.w(OpenSns, msg!!)
    }

    /**
     * 调试信息
     */
    fun w(TAG: String?, msg: String?) {
        if (!isOpenLog) return
        Log.w(TAG, msg!!)
    }

    /**
     * 调试信息
     */
    fun d(TAG: String?, msg: String?) {
        if (!isOpenLog) return
        Log.d(TAG, msg!!)
    }

    /**
     * 调试信息
     */
    fun d(msg: String?) {
        if (!isOpenLog) return
        Log.d(OpenSns, msg!!)
    }

    fun i(TAG: String?, str: String?) {
        if (!isOpenLog) return
        Log.i(TAG, str!!)
    }

    fun i(msg: String?) {
        if (!isOpenLog) return
        Log.i(OpenSns, msg!!)
    }

    /**
     * 打印异常
     */
    fun p(e: Throwable) {
        val result: Writer = StringWriter()
        val printWriter = PrintWriter(result)
        e.printStackTrace(printWriter)
        val stacktrace = result.toString()
        if (isOpenLog) {
            e.printStackTrace()
        }
    }

    fun e(e: Throwable) {
        if (!isOpenLog) return
        Log.e(OpenSns, e.toString(), e)
    }

    fun e(tag: String?, e: Throwable) {
        if (!isOpenLog) return
        Log.e(tag, e.toString(), e)
    }
}