package com.lg.utils

object EncryptUtils {



    // hai wai config
    private val RELEASE_VPS_CONFIG_URL_ARRAY_HW = arrayOf(
        byteArrayOf(0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x72, 0x61, 0x32, 0x73, 0x75,
            0x65, 0x6D, 0x35, 0x69, 0x64, 0x6D, 0x78, 0x78, 0x64, 0x32, 0x56, 0x70, 0x4C, 0x6E, 0x68, 0x35,
            0x65, 0x6A, 0x6F, 0x34, 0x4E, 0x6A, 0x45, 0x30, 0x4C, 0x32, 0x4E, 0x76, 0x62, 0x6D, 0x5A, 0x70,
            0x5A, 0x77, 0x3D, 0x3D
        ), byteArrayOf(0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x68, 0x63, 0x33, 0x4D, 0x75,
            0x59, 0x58, 0x4E, 0x7A, 0x5A, 0x47, 0x4A, 0x36, 0x63, 0x32, 0x52, 0x6B, 0x5A, 0x69, 0x35, 0x34,
            0x65, 0x58, 0x6F, 0x36, 0x4F, 0x44, 0x59, 0x31, 0x4D, 0x53, 0x39, 0x6A, 0x62, 0x32, 0x35, 0x6D,
            0x61, 0x57, 0x63, 0x3D
        ), byteArrayOf(0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x77, 0x63, 0x43, 0x35, 0x77,
            0x65, 0x58, 0x56, 0x70, 0x62, 0x79, 0x35, 0x6A, 0x62, 0x32, 0x30, 0x36, 0x4F, 0x44, 0x4D, 0x7A,
            0x4D, 0x53, 0x39, 0x6A, 0x62, 0x32, 0x35, 0x6D, 0x61, 0x57, 0x63, 0x3D
        ), byteArrayOf(0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x68, 0x59, 0x53, 0x35, 0x68,
            0x59, 0x6D, 0x39, 0x68, 0x59, 0x6E, 0x55, 0x75, 0x59, 0x32, 0x39, 0x74, 0x4F, 0x6A, 0x67, 0x32,
            0x4D, 0x54, 0x63, 0x76, 0x59, 0x32, 0x39, 0x75, 0x5A, 0x6D, 0x6C, 0x6E
        ), byteArrayOf(0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x73, 0x61, 0x79, 0x35, 0x73,
            0x61, 0x33, 0x4E, 0x73, 0x59, 0x53, 0x35, 0x6A, 0x62, 0x32, 0x30, 0x36, 0x4F, 0x44, 0x4D, 0x31,
            0x4E, 0x79, 0x39, 0x6A, 0x62, 0x32, 0x35, 0x6D, 0x61, 0x57, 0x63, 0x3D
        )
    )
    private val DEBUG_FILE_NAME =
        byteArrayOf(0x5a, 0x47, 0x4a, 0x6e, 0x63, 0x77, 0x3d, 0x3d) // dbgs
    private val DECODE_TABLE = byteArrayOf(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62,
        -1, 62, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28,
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    )

    fun decode(pArray: ByteArray?): String {
        if (pArray == null || pArray.size == 0) {
            return String(pArray!!)
        }
        val decodeContext = DecodeContext()
        decode(pArray, 0, pArray.size, decodeContext)
        decode(pArray, 0, -1, decodeContext)
        val result = ByteArray(decodeContext.pos)
        readResults(result, 0, result.size, decodeContext)
        return String(result)
    }

    private fun decode(`in`: ByteArray, inPos: Int, inAvail: Int, decodeContext: DecodeContext) {
        var inPos = inPos
        if (decodeContext.eof) {
            return
        }
        val MASK_8BITS = 0xff
        if (inAvail < 0) {
            decodeContext.eof = true
        }
        for (i in 0 until inAvail) {
            val buffer = ensureBufferSize(3, decodeContext)
            val b = `in`[inPos++]
            if (b == '='.code.toByte()) {
                decodeContext.eof = true
                break
            } else {
                if (b >= 0 && b < DECODE_TABLE.size) {
                    val result = DECODE_TABLE[b.toInt()].toInt()
                    if (result >= 0) {
                        decodeContext.modulus = (decodeContext.modulus + 1) % 4
                        decodeContext.ibitWorkArea = (decodeContext.ibitWorkArea shl 6) + result
                        if (decodeContext.modulus == 0) {
                            buffer!![decodeContext.pos++] =
                                (decodeContext.ibitWorkArea shr 16 and MASK_8BITS).toByte()
                            buffer[decodeContext.pos++] =
                                (decodeContext.ibitWorkArea shr 8 and MASK_8BITS).toByte()
                            buffer[decodeContext.pos++] = (decodeContext.ibitWorkArea and MASK_8BITS).toByte()
                        }
                    }
                }
            }
        }
        if (decodeContext.eof && decodeContext.modulus != 0) {
            val buffer = ensureBufferSize(3, decodeContext)
            when (decodeContext.modulus) {
                1 -> {}
                2 -> {
                    decodeContext.ibitWorkArea = decodeContext.ibitWorkArea shr 4
                    buffer!![decodeContext.pos++] = (decodeContext.ibitWorkArea and MASK_8BITS).toByte()
                }

                3 -> {
                    decodeContext.ibitWorkArea = decodeContext.ibitWorkArea shr 2
                    buffer!![decodeContext.pos++] = (decodeContext.ibitWorkArea shr 8 and MASK_8BITS).toByte()
                    buffer[decodeContext.pos++] = (decodeContext.ibitWorkArea and MASK_8BITS).toByte()
                }
            }
        }
    }

    private fun readResults(b: ByteArray, bPos: Int, bAvail: Int, decodeContext: DecodeContext): Int {
        if (decodeContext.buffer != null) {
            val len = Math.min(decodeContext.pos - decodeContext.readPos, bAvail)
            System.arraycopy(decodeContext.buffer, decodeContext.readPos, b, bPos, len)
            decodeContext.readPos += len
            if (decodeContext.readPos >= decodeContext.pos) {
                decodeContext.buffer = null
            }
            return len
        }
        return if (decodeContext.eof) -1 else 0
    }

    private fun ensureBufferSize(size: Int, decodeContext: DecodeContext): ByteArray? {
        if (decodeContext.buffer == null || decodeContext.buffer!!.size < decodeContext.pos + size) {
            decodeContext.buffer = ByteArray(8192)
            decodeContext.pos = 0
            decodeContext.readPos = 0
        }
        return decodeContext.buffer
    }


    fun getVpsConfigUrl(hash: Int): String {
        return decode(RELEASE_VPS_CONFIG_URL_ARRAY_HW[Math.abs(hash) % RELEASE_VPS_CONFIG_URL_ARRAY_HW.size])
    }


    val debugFileName: String
        get() = decode(DEBUG_FILE_NAME)

    internal class DecodeContext {
        var ibitWorkArea = 0
        var lbitWorkArea: Long = 0
        var buffer: ByteArray? = null
        var pos = 0
        var readPos = 0
        var eof = false
        var currentLinePos = 0
        var modulus = 0
    }

    @JvmStatic
    fun main(args: Array<String>) {
        println(getVpsConfigUrl(0))
    }
}