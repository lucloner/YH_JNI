package com;

/**
 * @program: YH_JNI
 * @description: 根据城建要求一次性强化JPG，偏斜校正，去黑边
 * @author: Biggee
 * @create: 2020-02-17 15:47
 **/
public class AllInOneEnhance {
    public static native double enhance(String srcPath, String destPath);
}
