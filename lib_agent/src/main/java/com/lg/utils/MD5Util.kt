package com.lg.utils

import java.math.BigInteger
import java.nio.charset.Charset
import java.security.MessageDigest
import java.security.NoSuchAlgorithmException

object MD5Util {


    @Throws(NoSuchAlgorithmException::class)
    fun getMD5(str: String): String? {
        val digest = MessageDigest.getInstance("MD5")
        var output = ""
        val buffer = str.toByteArray(Charset.defaultCharset())
        digest.update(buffer, 0, buffer.size)
        val md5sum = digest.digest()
        val bigInt = BigInteger(1, md5sum)
        output = bigInt.toString(16)
        while (output.length < 32) {
            output = "0$output"
        }
        return output
    }
}
