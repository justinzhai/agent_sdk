package com.lg.utils

import android.content.Context
import android.os.Environment
import android.text.TextUtils
import com.lg.config.Config
import java.io.File
import java.nio.charset.Charset

object FileUtils {

    fun readPidFile(context: Context): String? {
        val filePath =
            context.filesDir.path + File.separator + "myPid"
        if (TextUtils.isEmpty(filePath)) return null
        val myPid: String? = readFile(filePath)
        QYLog.e("myPid: $myPid ")
        return myPid
    }

    fun writePidFile(context: Context, myPid: String) {
        val file_path =
            context.filesDir.path + File.separator + "myPid"

        writeToFile(file_path, myPid)
    }


    //读取文件中的字符串
    fun readFile(filePath: String): String? {
        val file = File(filePath)
        if (!file.exists())
            return null
        var text = file.readText(Charset.forName("UTF-8"))
        return text
    }

    fun writeToFile(filePath: String, content: String) {
        val file = File(filePath)
        if (!file.exists()) file.createNewFile()
        file.writeText(content, Charset.forName("UTF-8"))
    }

    fun getDebugFile(): File? {

        var debugFilePath: String = ""
        if (isSDExists()) {
            debugFilePath =
                (Environment.getExternalStorageDirectory().absolutePath + "/"
                        + EncryptUtils.debugFileName)
        } else {
            try {
                debugFilePath= Config.mContext?.let { context ->
                    context.filesDir.path + File.separator + EncryptUtils.debugFileName  }.toString()

            } catch (e: Exception) {
                e.printStackTrace()
            }
        }

        return File(debugFilePath)
    }

    /**
     * 判断sd是否装载
     *
     * @return
     */
    fun isSDExists(): Boolean {
        return false
//        return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED
    }

}