package com.lg.utils

class RandomUtil {
    companion object {
        const val str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

        fun getRandomString(i: Int): String? {
            val sb = StringBuilder(i)
            for (j in 0 until i) {
                val index = (Math.random() * 3).toInt()
                sb.append(str[index])
            }
            return sb.toString()
        }
    }

}
