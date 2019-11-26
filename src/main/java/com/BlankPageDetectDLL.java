package com;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-26 19:55
 **/
public class BlankPageDetectDLL {
    static {
        System.loadLibrary("IdentityCard");
    }

    public native static void BlankPageDetect(String SrcPath, String DstPath);
}
