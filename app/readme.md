# lib_agent接入说明
1. 请将 lib_agent.aar 文件放到 libs目录下
2. 在app的 build.gradle文件中加入以下内容
```gradle
plugins {
    ...
    id 'org.jetbrains.kotlin.android'
}

dependencies {
    implementation fileTree(dir: 'libs', include: ['*.aar', '*.jar'], exclude: [])
    ...
}
```

3. 在某个Activity中调用以下方法
```java
import com.z.s.*;

......


        Agent agent=new Agent();
        agent.init(getContext(), new ISLoadCallBack() {
            @Override
            public void runStatus(int i) {
                System.out.println("runStatus:"+i); 
            }
        });

```
