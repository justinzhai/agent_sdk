package com.lg.utils;
import java.lang.Thread.UncaughtExceptionHandler;


import android.content.Context;
import android.os.AsyncTask;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;

/**
 * UncaughtException处理类,当程序发生Uncaught异常的时候,由该类来接管程序,并记录发送错误报告.
 *
 * @author GW
 */
public class CrashHandler implements UncaughtExceptionHandler {
    private UncaughtExceptionHandler mDefaultHandler;// 系统默认的UncaughtException处理类
    private static CrashHandler instance = null;// CrashHandler实例
    private Context mContext;
    private static boolean isInit = true;

    private CrashHandler() {
    }

    /**
     * 获取CrashHandler实例 ,单例模式
     */
    public static CrashHandler getInstance() {
        if (instance == null) {
            instance = new CrashHandler();
        }
        return instance;
    }

    /**
     * 初始化
     *
     * @param context
     */
    public void init(Context context) {
        if (isInit) {
            mContext = context;
            mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();// 获取系统默认的UncaughtException处理器
            Thread.setDefaultUncaughtExceptionHandler(this);// 设置该CrashHandler为程序的默认处理器
            isInit = false;
        }
    }

    /**
     * 当UncaughtException发生时会转入该重写的方法来处理
     */
    public void uncaughtException(Thread thread, Throwable ex) {
        try {
            Writer result = new StringWriter();
            PrintWriter printWriter = new PrintWriter(result);
            ex.printStackTrace(printWriter);
            // 把上面获取的堆栈信息转为字符串
            String stacktrace = result.toString();
            printWriter.close();
//            QYLog.e("CrashHandler uncaughtException: " + stacktrace);
            SendCrashLog sendLog = new SendCrashLog();
            sendLog.execute(stacktrace);
            mDefaultHandler.uncaughtException(thread, ex);
        } catch (Exception e) {
//            QYLog.e(e.toString());
        }

    }

    private class SendCrashLog extends AsyncTask<String, String, Boolean> {
        public SendCrashLog() {
        }

        @Override
        protected Boolean doInBackground(String... params) {
            if (params[0].length() == 0) {
                return false;
            }
//            QYLog.e("CrashHandler uncaughtException:" + params[0]);
            // ProxyManager.getInstance().sendExceptionLog(params[0], false,
            // ExceptionType.UNCATCH_EXCEPTON);
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
        }
    }
}
