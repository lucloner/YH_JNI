package com;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-26 23:13
 **/
public class BlankPageDetectDLL {

    static {
        System.loadLibrary("BlankPage_Detect");
    }

    public native int BlankPageDetect(String SrcPath);

}
