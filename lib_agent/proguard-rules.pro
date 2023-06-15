# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile


-forceprocessing
#-dontoptimize
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*
-optimizationpasses 5
-printmapping printmapping.txt
-applymapping applymapping.txt
-dontusemixedcaseclassnames
#-keepattributes Exceptions,Signature,InnerClasses,*Annotation*
-dontpreverify
-ignorewarnings
-dontwarn


## Kotlin wrapper
-keep class kotlin.jvm.internal.** {  }
-keep class kotlin.jvm.internal.FunctionReference {  }
-keep class kotlin.jvm.internal.Lambda {  }

-keep class kotlin.jvm.functions.Function0 {  }
-keep class kotlin.jvm.functions.Function1 {  }
-keep class kotlin.jvm.functions.Function2 {  }
-keep class kotlin.jvm.functions.* {  }

## don't waring Kotlin
-dontwarn kotlin.jvm.internal.**
-dontwarn kotlin.jvm.functions.**

-keep class com.z.s.* { *; }

-keepclasseswithmembers,allowshrinking class * {
    native <methods>;
}